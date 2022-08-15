#include "checker.hxx"

#include <vector>

using namespace std;
using namespace device;
using namespace topo;

Checker::Checker(const Topology& topo,
                 const Device& device,
                 const vector<Operation>& ops,
                 const vector<size_t>& assign)
    : topo_(topo.clone()), device_(new Device(device)), ops_(ops) {
    device_->place(assign);
}

size_t Checker::get_cycle(Operator op_type) const {
    switch (op_type) {
        case Operator::Swap:
            return device_->SWAP_CYCLE;
        case Operator::CX:
            return device_->CX_CYCLE;
        case Operator::Single:
            return device_->SINGLE_CYCLE;
    }
    cerr << "Wrong Type" << endl;
    abort();
}

void Checker::apply_gate(const Operation& op, Qubit& q0) const {
    size_t start = get<0>(op.get_duration());
    size_t end = get<1>(op.get_duration());

    if (!(start >= q0.get_avail_time())) {
        cerr << op << "\n"
             << "Q" << q0.get_id() << " occu: " << q0.get_avail_time() << endl;
        abort();
    }

    if (!(end == start + get_cycle(op.get_operator()))) {
        cerr << op << endl;
        abort();
    }

    q0.set_occupied_time(end);
}

void Checker::apply_gate(const Operation& op, Qubit& q0, Qubit& q1) const {
    size_t start = get<0>(op.get_duration());
    size_t end = get<1>(op.get_duration());

    if (!(start >= q0.get_avail_time() && start >= q1.get_avail_time())) {
        cerr << op << "\n"
             << "Q" << q0.get_id() << " occu: " << q0.get_avail_time() << "\n"
             << "Q" << q1.get_id() << " occu: " << q1.get_avail_time() << endl;
        abort();
    }

    if (!(end == start + get_cycle(op.get_operator()))) {
        cerr << op << endl;
        abort();
    }

    q0.set_occupied_time(end);
    q1.set_occupied_time(end);
}

void Checker::apply_Swap(const Operation& op) const {
    if (op.get_operator() != Operator::Swap) {
        cerr << op.get_operator_name() << " in apply_Swap" << endl;
        abort();
    }
    size_t q0_idx = get<0>(op.get_qubits());
    size_t q1_idx = get<1>(op.get_qubits());
    auto& q0 = device_->get_qubit(q0_idx);
    auto& q1 = device_->get_qubit(q1_idx);
    apply_gate(op, q0, q1);

    // swap
    size_t temp = q0.get_topo_qubit();
    q0.set_topo_qubit(q1.get_topo_qubit());
    q1.set_topo_qubit(temp);
}

bool Checker::apply_CX(const Operation& op, const Gate& gate) const {
    if (!(op.get_operator() == Operator::CX)) {
        cerr << op.get_operator_name() << " in apply_CX" << endl;
        abort();
    }

    size_t q0_idx = get<0>(op.get_qubits());
    size_t q1_idx = get<1>(op.get_qubits());
    auto& q0 = device_->get_qubit(q0_idx);
    auto& q1 = device_->get_qubit(q1_idx);

    size_t topo_0 = q0.get_topo_qubit();
    if (topo_0 == ERROR_CODE) {
        cerr << "topo_0 is ERROR CODE" << endl;
        abort();
    }

    size_t topo_1 = q1.get_topo_qubit();
    if (topo_1 == ERROR_CODE) {
        cerr << "topo_1 is ERRORCODE" << endl;
        abort();
    }

    if (topo_0 > topo_1) {
        swap(topo_0, topo_1);
    } else if (topo_0 == topo_1) {
        cerr << "topo_0 == topo_1: " << topo_0 << endl;
        abort();
    }
    if (topo_0 != get<0>(gate.get_qubits()) ||
        topo_1 != get<1>(gate.get_qubits())) {
        return false;
    }

    apply_gate(op, q0, q1);
    return true;
}

bool Checker::apply_Single(const Operation& op, const Gate& gate) const {
    if (!(op.get_operator() == Operator::Single)) {
        cerr << op.get_operator_name() << " in apply_Single" << endl;
        abort();
    }

    size_t q0_idx = get<0>(op.get_qubits());
    if (get<1>(op.get_qubits()) != ERROR_CODE) {
        cerr << "Single gate " << gate.get_id() << " has no null second qubit"
             << endl;
        abort();
    }
    auto& q0 = device_->get_qubit(q0_idx);

    size_t topo_0 = q0.get_topo_qubit();
    if (topo_0 == ERROR_CODE) {
        cerr << "topo_0 is ERROR CODE" << endl;
        abort();
    }

    if (topo_0 != get<0>(gate.get_qubits())) {
        return false;
    }

    apply_gate(op, q0);
    return true;
}

void Checker::test_operations() const {
    vector<size_t> finished_gates;

    cout << "Checking..." << endl;
    TqdmWrapper bar{ops_.size()};
    for (const auto& op : ops_) {
        if (op.get_operator() == Operator::Swap) {
            apply_Swap(op);
        } else {
            auto& avail_gates = topo_->get_avail_gates();
            bool pass_condition = false;
            if (op.get_operator() == Operator::CX) {
                for (auto gate : avail_gates) {
                    if (apply_CX(op, topo_->get_gate(gate))) {
                        pass_condition = true;
                        topo_->update_avail_gates(gate);
                        finished_gates.push_back(gate);
                        break;
                    }
                }
            } else if (op.get_operator() == Operator::Single) {
                for (auto gate : avail_gates) {
                    if (apply_Single(op, topo_->get_gate(gate))) {
                        pass_condition = true;
                        topo_->update_avail_gates(gate);
                        finished_gates.push_back(gate);
                        break;
                    }
                }
            } else {
                cerr << "Error gate type " << op.get_operator_name() << endl;
                abort();
            }
            if (!pass_condition) {
                cerr << "Executed gates:\n";
                for (auto gate : finished_gates) {
                    cerr << gate << "\n";
                }
                cerr << "Available gates:\n";
                for (auto gate : avail_gates) {
                    cerr << gate << "\n";
                }
                cerr << "Device status:\n";
                device_->print_device_state(cout);
                cerr << "Failed Operation: " << op;
                abort();
            }
        }
        ++bar;
    }
    cout << "\nnum gates: " << finished_gates.size() << "\n"
         << "num operations:" << ops_.size() << "\n";
    if (finished_gates.size() != topo_->get_num_gates()) {
        cerr << "Number of finished gates " << finished_gates.size()
             << " different from number of gates " << topo_->get_num_gates()
             << endl;
        abort();
    }
}
