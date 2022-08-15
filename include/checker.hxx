#pragma once

#include <bits/stdc++.h>
#include <memory>
#include <vector>
#include "algo.hxx"
#include "q_device.hxx"
#include "qft_topo.hxx"

class Checker {
   public:
    Checker(const topo::Topology& topo,
            const device::Device& device,
            const std::vector<device::Operation>& ops,
            const std::vector<size_t>& assign);

    size_t get_cycle(Operator op_type) const;

    void apply_gate(const device::Operation& op, device::Qubit& q0) const;
    void apply_gate(const device::Operation& op,
                    device::Qubit& q0,
                    device::Qubit& q1) const;

    void apply_Swap(const device::Operation& op) const;

    bool apply_CX(const device::Operation& op, const topo::Gate& gate) const;

    bool apply_Single(const device::Operation& op,
                      const topo::Gate& gate) const;

    void test_operations() const;

   private:
    std::unique_ptr<topo::Topology> topo_;
    std::unique_ptr<device::Device> device_;
    const std::vector<device::Operation>& ops_;
};
