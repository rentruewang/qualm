#include "qft_scheduler.hxx"

#include <omp.h>
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <vector>
#include "treenode.hxx"
#include "util.hxx"

using namespace std;
using namespace scheduler;

Dora::Dora(unique_ptr<Topology> topo, const json& conf)
    : Greedy(move(topo), conf),
      look_ahead(json_get<int>(conf, "depth")),
      never_cache_(json_get<bool>(conf, "never_cache")),
      exec_single_(json_get<bool>(conf, "exec_single")) {
    cache_only_when_necessary();
}

Dora::Dora(const Dora& other)
    : Greedy(other),
      look_ahead(other.look_ahead),
      never_cache_(other.never_cache_),
      exec_single_(other.exec_single_) {}

Dora::Dora(Dora&& other)
    : Greedy(other),
      look_ahead(other.look_ahead),
      never_cache_(other.never_cache_),
      exec_single_(other.exec_single_) {}

unique_ptr<Base> Dora::clone() const {
    return make_unique<Dora>(*this);
}

void Dora::cache_only_when_necessary() {
    if (!never_cache_ && look_ahead == 1) {
        cerr << "When look_ahead = 1, 'never_cache' is used by default.\n";
        never_cache_ = true;
    }
}

void Dora::assign_gates(unique_ptr<QFTRouter> router) {
    auto total_gates = topo_->get_num_gates();

    auto root = make_unique<TreeNode>(
        TreeNodeConf{never_cache_, exec_single_, conf_.candidates},
        vector<size_t>{}, router->clone(), clone(), 0);

    // For each step. (all nodes + 1 dummy)
    TqdmWrapper bar{total_gates + 1};
    do {
        // Update the candidates.
        auto selected_node =
            make_unique<TreeNode>(root->best_child(look_ahead));

        root = move(selected_node);

        for (size_t gate_idx : root->executed_gates()) {
            route_one_gate(*router, gate_idx);
            ++bar;
        }
    } while (!root->done());
}
