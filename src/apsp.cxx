#include "apsp.hxx"

#include <cassert>
#include <cmath>
#include <iostream>
#include <utility>
#include "tqdm_wrapper.hxx"

using namespace std;
using namespace torch::indexing;

ostream& operator<<(ostream& out, const ShortestPath sp) {
    return out << "Cost:\n"
               << sp.cost << '\n'
               << "Pointer:\n"
               << sp.pointer << "\n\n";
}

static ShortestPath floyd_warshall(torch::Tensor adj_mat);

ShortestPath apsp(torch::Tensor adj_mat) {
    torch::NoGradGuard no_grad;

    return floyd_warshall(adj_mat);
}

static ShortestPath floyd_warshall(torch::Tensor adj_mat) {
    assert(adj_mat.sizes().size() == 2 &&
           "adjacency matrix must be of dimension 2");

    assert(adj_mat.size(0) == adj_mat.size(1) &&
           "adjacency matrix size 0 and size 1 are different");

    auto intOpts = torch::TensorOptions().dtype(torch::kInt64);
    auto floatOpts = torch::TensorOptions().dtype(torch::kFloat32);

    int dimensions = adj_mat.size(0);
    auto pointer =
        torch::where(adj_mat != 0, torch::arange(dimensions).index({None}),
                     torch::full_like(adj_mat, -1, intOpts));

    auto cost_mat =
        torch::where(adj_mat != 0, adj_mat.clone().to(torch::kFloat32),
                     torch::full_like(adj_mat, INFINITY, floatOpts));

    TqdmWrapper bar{dimensions};
    for (int i = 0; i < dimensions; ++i, ++bar) {
        auto alt_path = cost_mat.index({None, i, Slice()}) +
                        cost_mat.index({Slice(), i, None});

        auto new_cost = torch::minimum(cost_mat, alt_path);

        auto new_pointer = pointer.clone();
        pointer = torch::where(cost_mat < alt_path, pointer,
                               pointer.index({Slice(), i, None}));
        cost_mat = new_cost;
    }

    for (int i = 0; i < dimensions; ++i) {
        cost_mat.index({i, i}) = 0;
        pointer.index({i, i}) = -1;
    }

    return {cost_mat, pointer};
}
