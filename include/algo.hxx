#pragma once

#include <assert.h>
#include <algorithm>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include "gate.hxx"
#include "topo.hxx"

namespace topo {
using namespace std;

class AlgoTopology : public Topology {
   public:
    AlgoTopology(fstream& f, bool IBMGate) : last_gate_({}) {
        parse(f, IBMGate);
    }
    AlgoTopology(const AlgoTopology& other) : Topology(other), last_gate_({}) {}
    AlgoTopology(AlgoTopology&& other)
        : Topology(move(other)), last_gate_({}) {}

    ~AlgoTopology() {}

    void parse(fstream&, bool IBMGate);

    unique_ptr<Topology> clone() const override {
        return make_unique<AlgoTopology>(*this);
    }

   private:
    vector<size_t> last_gate_;
};

};  // namespace topo
