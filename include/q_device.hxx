#pragma once

#include <assert.h>
#include <limits.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <string>
#include <tuple>
#include <vector>
#include "apsp.hxx"
#include "nlohmann/json.hpp"
#include "operator.hxx"
#include "util.hxx"

using nlohmann::json;

namespace device {
using namespace std;
class Operation {
   public:
    friend ostream& operator<<(ostream&, const Operation&);
    friend void to_json(json& j, const Operation& op);

    Operation(Operator oper, tuple<size_t, size_t> qs, tuple<size_t, size_t> du)
        : oper_(oper), qubits_(qs), duration_(du) {
        // sort qs
        size_t a = get<0>(qs);
        size_t b = get<1>(qs);
        assert(a != b);
        if (a > b) {
            qubits_ = make_tuple(b, a);
        }
    }
    Operation(const Operation& other)
        : oper_(other.oper_),
          qubits_(other.qubits_),
          duration_(other.duration_) {}

    Operation& operator=(const Operation& other) {
        oper_ = other.oper_;
        qubits_ = other.qubits_;
        duration_ = other.duration_;
        return *this;
    }

    size_t get_cost() const { return get<1>(duration_); }
    size_t get_op_time() const { return get<0>(duration_); }
    tuple<size_t, size_t> get_duration() const { return duration_; }
    Operator get_operator() const { return oper_; }
    string get_operator_name() const { return operator_get_name(oper_); }
    tuple<size_t, size_t> get_qubits() const { return qubits_; }

   private:
    Operator oper_;
    tuple<size_t, size_t> qubits_;
    tuple<size_t, size_t> duration_;  // <from, to>
};

ostream& operator<<(ostream&, const Operation&);
void to_json(json& j, const Operation& op);

bool op_order(const Operation& a, const Operation& b);
class AStarNode {
   public:
    friend class AStarComp;
    AStarNode(size_t cost, size_t id, bool swtch)
        : est_cost_(cost), id_(id), swtch_(swtch) {}
    AStarNode(const AStarNode& other)
        : est_cost_(other.est_cost_), id_(other.id_), swtch_(other.swtch_) {}

    AStarNode& operator=(const AStarNode& other) {
        est_cost_ = other.est_cost_;
        id_ = other.id_;
        swtch_ = other.swtch_;
        return *this;
    }

    bool get_swtch() const { return swtch_; }
    size_t get_id() const { return id_; }
    size_t get_cost() const { return est_cost_; }

   private:
    size_t est_cost_;
    size_t id_;
    bool swtch_;  // false q0 propagate, true q1 propagate
};

class AStarComp {
   public:
    bool operator()(const AStarNode& a, const AStarNode& b) {
        return a.est_cost_ > b.est_cost_;
    }
};

class Qubit {
   public:
    friend ostream& operator<<(ostream& os, const Qubit& q);
    Qubit(const size_t i);
    Qubit(const Qubit& other);
    Qubit(Qubit&& other);

    size_t get_id() const;
    size_t get_avail_time() const;
    bool is_adj(const Qubit& other) const;
    size_t get_topo_qubit() const;

    void add_adj(size_t i);
    void set_topo_qubit(size_t i);
    void set_occupied_time(size_t t);
    const vector<size_t>& get_adj_list() const;

    // A*
    size_t get_cost() const;
    bool is_marked() const;
    bool is_taken() const;
    bool get_swtch() const;
    size_t get_pred() const;
    size_t get_swap_time() const;
    void mark(bool swtch, size_t pred);
    void reset();
    void take_route(size_t cost, size_t swap_time);

   private:
    size_t id_;
    vector<size_t> adj_list_;
    size_t topo_qubit_;
    size_t occupied_until_;

    // for A*
    bool marked_;
    size_t pred_;
    size_t cost_;
    size_t swap_time_;
    bool swtch_;
    bool taken_;
};

ostream& operator<<(ostream& os, const Qubit& q);

class Device {
   public:
    Device(fstream& file, size_t r, size_t s, size_t cx) noexcept;
    Device(vector<vector<size_t>>&, size_t r, size_t s, size_t cx) noexcept;
    Device(const Device& other) noexcept;
    Device(Device&& other) noexcept;

    const size_t SINGLE_CYCLE, SWAP_CYCLE, CX_CYCLE;

    size_t get_num_qubits() const;
    vector<size_t> mapping() const;
    size_t get_shortest_cost(size_t i, size_t j) const;

    Qubit& get_qubit(size_t i);
    const Qubit& get_qubit(size_t i) const;

    Operation execute_single(Operator op, size_t q);
    vector<Operation> duostra_routing(Operator op,
                                      tuple<size_t, size_t> qs,
                                      bool orient);
    vector<Operation> apsp_routing(Operator op,
                                   tuple<size_t, size_t> qs,
                                   bool orient);

    void print_device_state(ostream& out);
    void place(const vector<size_t>& assign);  // topo2device
    void init_apsp();
    void reset();

   private:
    // A*

    // return <if touch target, target id>, swtch: false q0
    // propagate, true q1 propagate
    tuple<bool, size_t> touch_adj(
        Qubit& qubit,
        priority_queue<AStarNode, vector<AStarNode>, AStarComp>& pq,
        bool swtch);
    void update_trace(size_t trace, size_t qubit, vector<Operation>& ops) const;
    vector<Operation> traceback(Operator op,
                                Qubit& q0,
                                Qubit& q1,
                                Qubit& t0,
                                Qubit& t1);  // standalone

    // apsp
    tuple<size_t, size_t> next_swap_cost(size_t source, size_t target);

    // general
    void apply_gate(const Operation& op);

    // data member
    vector<Qubit> qubits_;
    vector<vector<size_t>> shortest_path_, shortest_cost_;
};
}  // namespace device
