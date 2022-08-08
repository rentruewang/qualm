#include "topo.hxx"
#include "qft_topo.hxx"
#include "util.hxx"

using namespace topo;
using namespace std;

// Explicitly instantiate true/false parameter so other files can call them.

template unordered_map<size_t, vector<size_t>> Topology::gate_by<true>() const;

template unordered_map<size_t, vector<size_t>> Topology::gate_by<false>() const;

void Topology::update_avail_gates(size_t executed) {
    assert(find(avail_gates_.cbegin(), avail_gates_.cend(), executed) !=
           avail_gates_.cend());
    const Gate& g_exec = get_gate(executed);

    auto erase_idx = remove(avail_gates_.begin(), avail_gates_.end(), executed);
    assert(avail_gates_.end() - erase_idx == 1);
    avail_gates_.erase(erase_idx, avail_gates_.end());

    assert(g_exec.get_id() == executed);

    executed_gates_[executed] = 0;
    for (size_t next : g_exec.get_nexts()) {
        if (get_gate(next).is_avail(executed_gates_)) {
            avail_gates_.push_back(next);
        }
    }

    vector<size_t> gates_to_trim;
    for (size_t prev_id : g_exec.get_prevs()) {
        const auto& prev_gate = get_gate(prev_id);

        size_t children_processed = ++executed_gates_[prev_id];
        size_t num_children = prev_gate.get_nexts().size();
        assert(children_processed <= num_children);

        if (children_processed == num_children) {
            gates_to_trim.push_back(prev_id);
        }
    }

    for (size_t gate_id : gates_to_trim) {
        executed_gates_.erase(gate_id);
    }
}

template <bool first>
vector<size_t> Topology::get_gates() const {
    using namespace std;
    vector<size_t> result;

    for (size_t i = 0; i < get_num_gates(); ++i) {
        const Gate& gate = get_gate(i);

        bool condition = first ? gate.is_first() : gate.is_last();
        if (condition) {
            result.push_back(i);
        }
    }

    return result;
}

template <bool first>
unordered_map<size_t, size_t> Topology::dist_to() const {
    using namespace std;

    unordered_map<size_t, size_t> dist;
    unordered_set<size_t> waiting;

    auto no_preceding = [&](size_t gate_idx) -> bool {
        const auto& gate = get_gate(gate_idx);

        const auto& indices = first ? gate.get_prevs() : gate.get_nexts();

        return all_of(indices.cbegin(), indices.cend(), [&](size_t idx) {
            return idx == ERROR_CODE || dist.find(idx) != dist.end();
        });
    };

    auto zero_nodes{get_gates<first>()};
    for (size_t node : zero_nodes) {
        waiting.insert(node);
    }

    size_t counter = 0;
    for (TqdmWrapper bar{get_num_gates()}; waiting.size() != 0; ++counter) {
        const auto cloned_waiting{waiting};

        vector<size_t> visited_this_cycle;
        for (size_t idx : cloned_waiting) {
            if (no_preceding(idx)) {
                const auto& gate = get_gate(idx);

                const auto& indices =
                    first ? gate.get_nexts() : gate.get_prevs();

                for (size_t idx : indices) {
                    waiting.insert(idx);
                }

                visited_this_cycle.push_back(idx);
                waiting.erase(idx);
            }
        }

        for (auto vtc : visited_this_cycle) {
            dist[vtc] = counter;
            ++bar;
        }
    }

    return dist;
}

template <bool first>
unordered_map<size_t, vector<size_t>> Topology::gate_by() const {
    auto dist{dist_to<first>()};

    if (first) {
        cout << "First done\n";
    } else {
        cout << "Last done\n";
    }

    return gate_by_generation(dist);
}

QFTTopology::QFTTopology(size_t num) {
    size_t num_qubits = num;
    vector<Gate> all_gates;
    assert(num > 0);

    for (size_t i = 0, count = 0; i < num; ++i) {
        for (size_t j = 0; j < i; ++j, ++count) {
            size_t prev_up = (j == i - 1) ? ERROR_CODE : count + 1 - i;
            size_t prev_left = (j == 0) ? ERROR_CODE : count - 1;
            size_t next_down = (i == num - 1) ? ERROR_CODE : count + i;
            size_t next_right = (j == i - 1) ? ERROR_CODE : count + 1;

            Gate gate{count, Operator::CX, make_tuple(j, i)};
            gate.add_prev(prev_up);
            gate.add_prev(prev_left);
            gate.add_next(next_down);
            gate.add_next(next_right);

            all_gates.push_back(move(gate));
        }
    }
    avail_gates_.push_back(0);

    dep_graph_ = make_shared<DependencyGraph>(num_qubits, move(all_gates));
}

QFTTopology::QFTTopology(const QFTTopology& other) : Topology(other) {}
QFTTopology::QFTTopology(QFTTopology&& other) : Topology(move(other)) {}
