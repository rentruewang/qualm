#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include "q_device.hxx"
#include "qft_router.hxx"
#include "topo.hxx"
#include "util.hxx"

namespace scheduler {
using namespace std;
using namespace topo;

class Base {
   public:
    Base(const Base& other);
    Base(unique_ptr<Topology> topo);
    Base(Base&& other);
    virtual ~Base() {}

    Topology& topo() { return *topo_; }
    const topo::Topology& topo() const { return *topo_; }

    virtual unique_ptr<Base> clone() const;

    void assign_gates_and_sort(unique_ptr<QFTRouter> router) {
        assign_gates(std::move(router));
        sort();
    }

    void write_assembly(ostream& out) const;
    void to_json(json& j) const;

    size_t get_final_cost() const;
    size_t get_total_time() const;
    size_t get_swap_num() const;
    const vector<size_t>& get_avail_gates() const {
        return topo_->get_avail_gates();
    }

    const vector<device::Operation>& get_operations() const { return ops_; }
    size_t ops_cost() const;

    size_t route_one_gate(QFTRouter& router,
                          size_t gate_idx,
                          bool forget = false);

    size_t get_executable(QFTRouter& router) const;
    size_t get_executable(QFTRouter& router,
                          const vector<size_t>& wait_list) const;

   protected:
    unique_ptr<topo::Topology> topo_;
    vector<device::Operation> ops_;
    bool sorted_ = false;

    virtual void assign_gates(unique_ptr<QFTRouter> router);
    void sort();
};

class Random : public Base {
   public:
    Random(unique_ptr<topo::Topology> topo);
    Random(const Random& other);
    Random(Random&& other);
    ~Random() override {}

    unique_ptr<Base> clone() const override;

   protected:
    void assign_gates(unique_ptr<QFTRouter> router) override;
};

class Static : public Base {
   public:
    Static(unique_ptr<topo::Topology> topo);
    Static(const Static& other);
    Static(Static&& other);
    ~Static() override {}

    unique_ptr<Base> clone() const override;

   protected:
    void assign_gates(unique_ptr<QFTRouter> router) override;
};

struct ShortestPathConf {
    ShortestPathConf()
        : avail_typ(true),
          cost_typ(false),
          candidates(ERROR_CODE),
          apsp_coef(1) {}

    ShortestPathConf(const json& conf);

    bool avail_typ;  // true is max, false is min
    bool cost_typ;   // true is max, false is min
    size_t candidates;
    size_t apsp_coef;
};

class ShortestPath : public Base {
   public:
    ShortestPath(unique_ptr<topo::Topology> topo, const json& conf);
    ShortestPath(const ShortestPath& other);
    ShortestPath(ShortestPath&& other);
    ~ShortestPath() override {}

    unique_ptr<Base> clone() const override;

   protected:
    ShortestPathConf conf_;

    size_t greedy_fallback(const QFTRouter& router,
                           const std::vector<size_t>& wait_list,
                           size_t gate_idx) const;
    virtual size_t executable_with_fallback(
        QFTRouter& router,
        const std::vector<size_t>& wait_list) const;
    void assign_gates(unique_ptr<QFTRouter> router) override;
};

class Onion : public ShortestPath {
   public:
    Onion(unique_ptr<Topology> topo, const json& conf);
    Onion(const Onion& other);
    Onion(Onion&& other);
    ~Onion() override {}

    unique_ptr<Base> clone() const override;

   protected:
    bool first_mode_;

    void assign_gates(unique_ptr<QFTRouter> router) override;

    size_t executable_with_fallback(
        QFTRouter& router,
        const std::vector<size_t>& wait_list) const override;

    size_t assign_generation(
        QFTRouter& router,
        std::unordered_map<size_t, std::vector<size_t>>& gen_to_gates);
    void assign_from_wait_list(QFTRouter& router, vector<size_t>& wait_list);
};

class Dora : public ShortestPath {
   public:
    Dora(unique_ptr<Topology> topo, const json& conf);
    Dora(const Dora& other);
    Dora(Dora&& other);
    ~Dora() override {}

    const size_t look_ahead;

    unique_ptr<Base> clone() const override;

   protected:
    bool never_cache_;
    bool exec_single_;

    void assign_gates(unique_ptr<QFTRouter> router) override;
    void cache_only_when_necessary();
};

unique_ptr<Base> get(const string& typ, unique_ptr<Topology> topo, json& conf);
}  // namespace scheduler
