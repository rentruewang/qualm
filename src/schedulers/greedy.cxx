#include "qft_scheduler.hxx"

#include <vector>
#include "util.hxx"

using namespace scheduler;
using namespace std;

class TopologyCandidate {
   public:
    TopologyCandidate(const topo::Topology& topo, size_t candidate)
        : topo_(topo), cands_(candidate) {}

    vector<size_t> get_avail_gates() const {
        auto& gates = topo_.get_avail_gates();

        if (gates.size() < cands_) {
            return gates;
        }

        return {gates.cbegin(), gates.cbegin() + cands_};
    }

   private:
    const topo::Topology& topo_;
    size_t cands_;
};

ShortestPathConf::ShortestPathConf(const json& conf) : ShortestPathConf() {
    int cands = json_get<int>(conf, "candidates");
    if (cands > 0) {
        this->candidates = cands;
    }

    this->apsp_coef = json_get<size_t>(conf, "apsp_coef");

    string avail_typ = json_get<string>(conf, "avail");
    if (avail_typ == "min") {
        this->avail_typ = false;
    } else if (avail_typ == "max") {
        this->avail_typ = true;
    } else {
        cerr << "\"min_max\" can only be \"min\" or \"max\"." << endl;
        abort();
    }

    string cost_typ = json_get<string>(conf, "cost");
    if (cost_typ == "min") {
        this->cost_typ = false;
    } else if (cost_typ == "max") {
        this->cost_typ = true;
    } else {
        cerr << "\"min_max\" can only be \"min\" or \"max\"." << endl;
        abort();
    }
}

ShortestPath::ShortestPath(unique_ptr<topo::Topology> topo, const json& conf)
    : Base(move(topo)), conf_(conf) {}

ShortestPath::ShortestPath(const ShortestPath& other)
    : Base(other), conf_(other.conf_) {}

ShortestPath::ShortestPath(ShortestPath&& other)
    : Base(move(other)), conf_(other.conf_) {}

size_t ShortestPath::executable_with_fallback(
    QFTRouter& router,
    const vector<size_t>& wait_list) const {
    size_t gate_idx = get_executable(router);
    return greedy_fallback(router, wait_list, gate_idx);
}

void ShortestPath::assign_gates(unique_ptr<QFTRouter> router) {
    cout << "ShortestPath scheduler running..." << endl;

    [[maybe_unused]] size_t count = 0;
    auto topo_wrap = TopologyCandidate(*topo_, conf_.candidates);

    for (TqdmWrapper bar{topo_->get_num_gates()};
         !topo_wrap.get_avail_gates().empty(); ++bar) {
        auto wait_list = topo_wrap.get_avail_gates();
        assert(wait_list.size() > 0);

        size_t gate_idx = executable_with_fallback(*router, wait_list);
        route_one_gate(*router, gate_idx);
        ++count;
    }
    assert(count == topo_->get_num_gates());
}

size_t ShortestPath::greedy_fallback(const QFTRouter& router,
                                     const vector<size_t>& wait_list,
                                     size_t gate_idx) const {
    if (gate_idx != ERROR_CODE) {
        return gate_idx;
    }
    vector<size_t> cost_list(wait_list.size(), 0);

    for (size_t i = 0; i < wait_list.size(); ++i) {
        const auto& gate = topo_->get_gate(wait_list[i]);
        cost_list[i] =
            router.get_gate_cost(gate, conf_.avail_typ, conf_.apsp_coef);
    }

    auto list_idx = conf_.cost_typ
                        ? max_element(cost_list.cbegin(), cost_list.cend()) -
                              cost_list.cbegin()
                        : min_element(cost_list.cbegin(), cost_list.cend()) -
                              cost_list.cbegin();
    return wait_list[list_idx];
}

unique_ptr<Base> ShortestPath::clone() const {
    return make_unique<ShortestPath>(*this);
}
