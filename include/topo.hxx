#pragma once

#include <cassert>
#include <climits>
#include <cmath>
#include <iostream>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "gate.hxx"
#include "operator.hxx"
#include "tqdm_wrapper.hxx"
#include "util.hxx"

namespace topo {
using namespace std;

class DependencyGraph {
   public:
    DependencyGraph(size_t n, vector<Gate>&& gates)
        : num_qubits_(n), gates_(move(gates)) {}
    DependencyGraph(const DependencyGraph& other) = delete;
    DependencyGraph(DependencyGraph&& other)
        : num_qubits_(other.num_qubits_), gates_(move(other.gates_)) {}

    const vector<Gate>& gates() const { return gates_; }
    const Gate& get_gate(size_t idx) const { return gates_[idx]; }
    size_t get_num_qubits() const { return num_qubits_; }

   private:
    size_t num_qubits_;
    vector<Gate> gates_;
};

template <typename T>
void unstable_erase(vector<T>& vec, const T& value);

class Topology {
   public:
    Topology()
        : dep_graph_(shared_ptr<DependencyGraph>(nullptr)),
          avail_gates_({}),
          executed_gates_({}) {}

    Topology(const Topology& other)
        : dep_graph_(other.dep_graph_),
          avail_gates_(other.avail_gates_),
          executed_gates_(other.executed_gates_) {}

    Topology(Topology&& other)
        : dep_graph_(move(other.dep_graph_)),
          avail_gates_(move(other.avail_gates_)),
          executed_gates_(move(other.executed_gates_)) {}

    virtual ~Topology() {}

    virtual unique_ptr<Topology> clone() const = 0;

    void update_avail_gates(size_t executed);
    size_t get_num_qubits() const { return dep_graph_->get_num_qubits(); }
    size_t get_num_gates() const { return dep_graph_->gates().size(); }
    const Gate& get_gate(size_t i) const { return dep_graph_->get_gate(i); }
    const vector<size_t>& get_avail_gates() const { return avail_gates_; }

    template <bool first>
    unordered_map<size_t, vector<size_t>> gate_by() const;

   protected:
    // data member
    shared_ptr<const DependencyGraph> dep_graph_;
    vector<size_t> avail_gates_;

    // Executed gates is a countable set.
    unordered_map<size_t, size_t> executed_gates_;

    // private function
    static unordered_map<size_t, vector<size_t>> gate_by_generation(
        const unordered_map<size_t, size_t>& map);

    template <bool first>
    vector<size_t> get_gates() const;

    template <bool first>
    unordered_map<size_t, size_t> dist_to() const;
};
}  // namespace topo
