#include "qft_mapper.hxx"

namespace scheduler {

unique_ptr<Base> get(const string& typ, unique_ptr<Topology> topo, json& conf) {
    if (typ == "random") {
        return make_unique<Random>(move(topo));
    }

    if (typ == "old") {
        return make_unique<Base>(move(topo));
    }

    if (typ == "static" || typ == "fifo") {
        return make_unique<Static>(move(topo));
    }

    if (typ == "onion") {
        return make_unique<Onion>(move(topo), conf);
    }

    if (typ == "greedy" || typ == "apsp") {
        return make_unique<Greedy>(move(topo), conf);
    }

    if (typ == "dora" || typ == "cks") {
        return make_unique<Dora>(move(topo), conf);
    }

    cerr << typ << " is not a scheduler type" << endl;
    abort();
}

}  // namespace scheduler
