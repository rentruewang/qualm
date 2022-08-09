#pragma once

#include <memory>
#include <vector>
#include "qft_router.hxx"
#include "qft_scheduler.hxx"

struct TreeNodeConf {
    // Never cache any children unless children() is called.
    bool never_cache;
    // Execute the single gates when they are available.
    bool exec_single;
    // The number of childrens to consider,
    // selected with some ops_cost heuristic.
    size_t candidates;
};

// This is a node of the heuristic search tree.
class TreeNode {
   public:
    TreeNode(TreeNodeConf conf,
             size_t gate_idx,
             std::unique_ptr<QFTRouter> router,
             std::unique_ptr<scheduler::Base> scheduler,
             size_t max_cost);
    TreeNode(TreeNodeConf conf,
             std::vector<size_t>&& gate_indices,
             std::unique_ptr<QFTRouter> router,
             std::unique_ptr<scheduler::Base> scheduler,
             size_t max_cost);
    TreeNode(const TreeNode& other);
    TreeNode(TreeNode&& other);

    TreeNode& operator=(const TreeNode& other);
    TreeNode& operator=(TreeNode&& other);

    TreeNode best_child(int depth);

    size_t best_cost(int depth);
    size_t best_cost() const;

    const QFTRouter& router() const { return *router_; }
    const scheduler::Base& scheduler() const { return *scheduler_; }

    const std::vector<size_t>& executed_gates() const { return gate_indices_; }

    bool done() const { return scheduler().get_avail_gates().empty(); }
    bool is_leaf() const { return children_.empty(); }
    void grow_if_needed();

    bool can_grow() const;

   private:
    TreeNodeConf conf_;

    // The head of the node.
    std::vector<size_t> gate_indices_;

    // Using vector to pointer so that frequent cache misses
    // won't be as bad in parallel code.
    std::vector<TreeNode> children_;

    // The state of duostra.
    size_t max_cost_;
    std::unique_ptr<QFTRouter> router_;
    std::unique_ptr<scheduler::Base> scheduler_;

    std::vector<TreeNode>&& children();

    void grow();
    void route_internal_gates();
    size_t immediate_next() const;
};
