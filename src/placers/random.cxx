#include "qft_placer.hxx"

#include <random>

using namespace placer;
using namespace std;

vector<size_t> Random::place(Device& device) const {
    vector<size_t> assign;
    assign.reserve(device.get_num_qubits());
    for (size_t i = 0; i < device.get_num_qubits(); ++i) {
        assign.push_back(i);
    }
    size_t seed = chrono::system_clock::now().time_since_epoch().count();

    shuffle(assign.begin(), assign.end(), default_random_engine(seed));
    return assign;
}
