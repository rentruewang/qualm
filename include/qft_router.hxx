#pragma once

#include <string>
#include "q_device.hxx"
#include "topo.hxx"

class QFTRouter {
   public:
    QFTRouter(device::Device&& device,
              const std::string& typ,
              const std::string& cost,
              bool orient) noexcept;
    QFTRouter(const QFTRouter& other) noexcept;
    QFTRouter(QFTRouter&& other) noexcept;

    std::vector<device::Operation> assign_gate(const topo::Gate& gate);

    device::Device& get_device() { return device_; }
    const device::Device& get_device() const { return device_; }

    size_t get_gate_cost(const topo::Gate& gate,
                         bool min_max,
                         size_t apsp_coef) const;
    bool is_executable(const topo::Gate& gate) const;
    std::unique_ptr<QFTRouter> clone() const;

   private:
    bool greedy_type_;
    bool duostra_;
    bool orient_;
    bool apsp_;
    device::Device device_;
    std::vector<size_t> topo_to_dev_;

    void init(const std::string& typ, const std::string& cost);
    std::tuple<size_t, size_t> get_device_qubits_idx(
        const topo::Gate& gate) const;
};
