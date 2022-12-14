#include "qft_scheduler.hxx"

#include <algorithm>
#include <cassert>
#include "util.hxx"

using namespace scheduler;
using namespace device;

Base::Base(unique_ptr<Topology> topo) : topo_(move(topo)), ops_({}) {}

Base::Base(const Base& other) : topo_(other.topo_->clone()), ops_(other.ops_) {}

Base::Base(Base&& other) : topo_(move(other.topo_)), ops_(move(other.ops_)) {}

unique_ptr<Base> Base::clone() const {
    return make_unique<Base>(*this);
}

void Base::sort() {
    std::sort(ops_.begin(), ops_.end(), op_order);
    sorted_ = true;
}

void Base::write_assembly(ostream& out) const {
    assert(sorted_);

    for (size_t i = 0; i < ops_.size(); ++i) {
        const auto& op = ops_.at(i);
        string operator_name{operator_get_name(op.get_operator())};
        out << operator_name << " ";
        tuple<size_t, size_t> qubits = op.get_qubits();
        out << "Q[" << std::get<0>(qubits) << "] Q[" << std::get<1>(qubits)
            << "]; ";
        out << "(" << op.get_op_time() << "," << op.get_cost() << ")\n";
    }
}

void Base::to_json(json& j) const {
    assert(sorted_);

    json o;
    for (size_t i = 0; i < ops_.size(); ++i) {
        const auto& op = ops_.at(i);
        json buf = op;
        o.push_back(buf);
    }
    j["Operations"] = o;
}

void Base::assign_gates(unique_ptr<QFTRouter> router) {
    cout << "Default scheduler running..." << endl;

    for (TqdmWrapper bar{topo_->get_num_gates()}; !bar.done(); ++bar) {
        route_one_gate(*router, bar.idx());
    }
}

size_t Base::get_final_cost() const {
    assert(sorted_);
    return ops_.at(ops_.size() - 1).get_cost();
}

size_t Base::get_total_time() const {
    assert(sorted_);

    size_t ret = 0;
    for (size_t i = 0; i < ops_.size(); ++i) {
        tuple<size_t, size_t> dur = ops_[i].get_duration();
        ret += std::get<1>(dur) - std::get<0>(dur);
    }
    return ret;
}

size_t Base::get_swap_num() const {
    size_t ret = 0;
    for (size_t i = 0; i < ops_.size(); ++i) {
        if (ops_.at(i).get_operator() == Operator::Swap) {
            ++ret;
        }
    }
    return ret;
}

static size_t max_op_cost(const vector<Operation>& ops) {
    return std::max_element(ops.cbegin(), ops.cend(),
                            [](const Operation& a, const Operation& b) {
                                return a.get_cost() < b.get_cost();
                            })
        ->get_cost();
}

size_t Base::ops_cost() const {
    return max_op_cost(ops_);
}

size_t Base::get_executable(QFTRouter& router) const {
    const auto& avail_gates = topo_->get_avail_gates();
    return get_executable(router, avail_gates);
}

size_t Base::get_executable(QFTRouter& router,
                            const vector<size_t>& wait_list) const {
    [[maybe_unused]] const auto& avail_gates = topo_->get_avail_gates();
    for ([[maybe_unused]] size_t gate_idx : wait_list) {
        assert(find(avail_gates.cbegin(), avail_gates.cend(), gate_idx) !=
               avail_gates.end());
    }

    for (size_t gate_idx : wait_list) {
        if (router.is_executable(topo_->get_gate(gate_idx))) {
            return gate_idx;
        }
    }
    return ERROR_CODE;
}

size_t Base::route_one_gate(QFTRouter& router, size_t gate_idx, bool forget) {
    const auto& gate = topo_->get_gate(gate_idx);
    auto ops{router.assign_gate(gate)};
    size_t max_cost = max_op_cost(ops);

    if (!forget) {
        ops_.insert(ops_.end(), ops.begin(), ops.end());
    }

    topo_->update_avail_gates(gate_idx);
    return max_cost;
}
