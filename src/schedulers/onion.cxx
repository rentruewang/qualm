#include "qft_scheduler.hxx"

using namespace scheduler;
using namespace std;

Onion::Onion(unique_ptr<Topology> topo, const json& conf)
    : Greedy(move(topo), conf),
      first_mode_(json_get<bool>(conf, "layer_from_first")) {}

Onion::Onion(const Onion& other)
    : Greedy(other), first_mode_(other.first_mode_) {}

Onion::Onion(Onion&& other)
    : Greedy(move(other)), first_mode_(other.first_mode_) {}

void Onion::assign_gates(unique_ptr<QFTRouter> router) {
    using namespace std;
    cout << "Onion scheduler running..." << endl;

    auto gen_to_gates =
        first_mode_ ? topo_->gate_by<true>() : topo_->gate_by<false>();

    size_t num_gates = topo_->get_num_gates();

    TqdmWrapper bar{num_gates};

    while (gen_to_gates.size()) {
        size_t amount = assign_generation(*router, gen_to_gates);
        for (size_t i = 0; i < amount; ++i, ++bar)
            ;
    }

    auto& avail_gates = topo_->get_avail_gates();
    assert(avail_gates.empty());
    assert(bar.done());

    cout << avail_gates.size() << "\n";
}

size_t Onion::executable_with_fallback(QFTRouter& router,
                                       const vector<size_t>& wait_list) const {
    size_t gate_idx = get_executable(router, wait_list);
    return greedy_fallback(router, wait_list, gate_idx);
}

size_t Onion::assign_generation(
    QFTRouter& router,
    unordered_map<size_t, vector<size_t>>& gen_to_gates) {
    using gen_pair = const pair<size_t, vector<size_t>>&;
    auto select =
        first_mode_ ? [](gen_pair a, gen_pair b) { return a.first < b.first; }
                    : [](gen_pair a, gen_pair b) { return a.first > b.first; };

    auto youngest =
        min_element(gen_to_gates.begin(), gen_to_gates.end(), select);

    assert(youngest != gen_to_gates.end());

    auto& [distance, wait_list] = *youngest;

    const size_t iter_count = wait_list.size();
    for (size_t i = 0; i < iter_count; ++i) {
        assign_from_wait_list(router, wait_list);
    }

    assert(wait_list.empty());
    gen_to_gates.erase(distance);
    return iter_count;
}

void Onion::assign_from_wait_list(QFTRouter& router,
                                  vector<size_t>& wait_list) {
    size_t gate_idx = executable_with_fallback(router, wait_list);
    unstable_erase(wait_list, gate_idx);
    route_one_gate(router, gate_idx);
}

unique_ptr<Base> Onion::clone() const {
    return make_unique<Onion>(*this);
}
