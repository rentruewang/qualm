#include "flow.hxx"

#include "checker.hxx"
#include "qft_mapper.hxx"

using namespace std;

using device::Device;
using topo::AlgoTopology;
using topo::QFTTopology;
using topo::Topology;
using Scheduler = scheduler::Base;

static unique_ptr<Topology> new_topo(const json& conf);
static Device create_device(const json& conf);
static vector<size_t> place_device(const json& conf_mapper,
                                   const vector<size_t>& assign,
                                   Device& device);
static unique_ptr<Scheduler> new_scheduler(const json& conf_mapper,
                                           unique_ptr<Topology> topo,
                                           const string& scheduler_typ);

static unique_ptr<QFTRouter> new_router(const json& conf_mapper,
                                        const string& scheduler_typ,
                                        Device&& device);

void check_result(const Topology& topo,
                  const Device& device,
                  const Scheduler& sched,
                  const vector<size_t>& assign);

static void dump_result(const json& conf,
                        const vector<size_t>& assign,
                        const unique_ptr<Scheduler>& sched);

size_t flow(const json& conf, vector<size_t> assign, bool io) {
    if (!io) {
        cout.setstate(ios_base::failbit);
    }
    // create topology
    auto topo = new_topo(conf);

    // create device
    auto device{create_device(conf)};

    // Copy device and topo for checks.
    auto topo_copy = topo->clone();
    auto device_copy{device};

    if (topo->get_num_qubits() > device.get_num_qubits()) {
        cerr << "You cannot assign more QFT qubits than the device." << endl;
        abort();
    }

    // create mapper
    json conf_mapper = json_get<json>(conf, "mapper");

    // place
    assign = place_device(conf_mapper, assign, device);

    // scheduler
    const string scheduler_typ = json_get<string>(conf_mapper, "scheduler");
    auto sched = new_scheduler(conf_mapper, move(topo), scheduler_typ);

    // router
    auto router = new_router(conf_mapper, scheduler_typ, move(device));

    // routing
    cout << "routing..." << endl;
    sched->assign_gates_and_sort(move(router));

    // checker
    bool check = json_get<bool>(conf, "check");
    if (check) {
        check_result(*topo_copy, device_copy, *sched, assign);
    }

    // dump
    bool dump = json_get<bool>(conf, "dump");
    if (dump) {
        dump_result(conf, assign, sched);
    }

    if (json_get<bool>(conf, "stdio")) {
        sched->write_assembly(cout);
    }

    cout << "final cost: " << sched->get_final_cost() << "\n";
    cout << "total time: " << sched->get_total_time() << "\n";
    cout << "total swaps: " << sched->get_swap_num() << "\n";

    cout.clear();
    return sched->get_final_cost();
}

size_t device_num(const json& conf) {
    // create device
    auto device{create_device(conf)};
    return device.get_num_qubits();
}

size_t topo_num(const json& conf) {
    // create topology
    cout << "creating topology..." << endl;
    unique_ptr<Topology> topo;
    if (conf["algo"].type() == json::value_t::null) {
        cerr << "Necessary key \"algo\" does not exist." << endl;
        abort();
    } else if (conf["algo"].type() == json::value_t::number_unsigned) {
        size_t num_qubit = json_get<size_t>(conf, "algo");
        topo = make_unique<QFTTopology>(num_qubit);
    } else {
        fstream algo_file;
        auto algo_filename = json_get<string>(conf, "algo");
        cout << algo_filename << endl;
        algo_file.open(algo_filename, fstream::in);
        if (!algo_file.is_open()) {
            cerr << "There is no file " << algo_filename << endl;
            abort();
        }
        bool onlyIBM = json_get<size_t>(conf, "IBM_Gate");
        topo = make_unique<AlgoTopology>(algo_file, onlyIBM);
    }

    return topo->get_num_qubits();
}

unique_ptr<Topology> new_topo(const json& conf) {
    cout << "creating topology..." << endl;
    unique_ptr<Topology> topo;
    if (conf["algo"].type() == json::value_t::null) {
        cerr << "Necessary key \"algo\" does not exist." << endl;
    } else if (conf["algo"].type() == json::value_t::number_unsigned) {
        size_t num_qubit = json_get<size_t>(conf, "algo");
        return make_unique<QFTTopology>(num_qubit);
    } else {
        fstream algo_file;
        auto algo_filename = json_get<string>(conf, "algo");
        cout << algo_filename << endl;
        algo_file.open(algo_filename, fstream::in);
        if (!algo_file.is_open()) {
            cerr << "There is no file " << algo_filename << endl;
            abort();
        }
        bool onlyIBM = json_get<bool>(conf, "IBM_Gate");
        return make_unique<AlgoTopology>(algo_file, onlyIBM);
    }

    abort();
}

Device create_device(const json& conf) {
    cout << "creating device..." << endl;
    json cycle_conf = json_get<json>(conf, "cycle");
    size_t SINGLE_CYCLE = json_get<size_t>(cycle_conf, "SINGLE_CYCLE");
    size_t SWAP_CYCLE = json_get<size_t>(cycle_conf, "SWAP_CYCLE");
    size_t CX_CYCLE = json_get<size_t>(cycle_conf, "CX_CYCLE");

    fstream device_file;
    string device_filename = json_get<string>(conf, "device");
    device_file.open(device_filename, fstream::in);
    if (!device_file.is_open()) {
        cerr << "There is no file " << device_filename << endl;
        abort();
    }
    return {device_file, SINGLE_CYCLE, SWAP_CYCLE, CX_CYCLE};
}

vector<size_t> place_device(const json& conf_mapper,
                            const vector<size_t>& assign,
                            Device& device) {
    cout << "creating placer..." << endl;
    if (assign.empty()) {
        string placer_typ = json_get<string>(conf_mapper, "placer");
        auto plc = placer::get(placer_typ);
        return plc->place_and_assign(device);
    } else {
        device.place(assign);
        return assign;
    }
}

unique_ptr<Scheduler> new_scheduler(const json& conf_mapper,
                                    unique_ptr<Topology> topo,
                                    const string& scheduler_typ) {
    cout << "creating scheduler..." << endl;
    json greedy_conf = json_get<json>(conf_mapper, "greedy_conf");
    return scheduler::get(scheduler_typ, topo->clone(), greedy_conf);
}

unique_ptr<QFTRouter> new_router(const json& conf_mapper,
                                 const string& scheduler_typ,
                                 Device&& device) {
    cout << "creating router..." << endl;

    string router_typ = json_get<string>(conf_mapper, "router");
    bool orient = json_get<bool>(conf_mapper, "orientation");
    string cost = (scheduler_typ == "greedy" || scheduler_typ == "onion")
                      ? json_get<string>(conf_mapper, "cost")
                      : "start";

    return make_unique<QFTRouter>(move(device), router_typ, cost, orient);
}

void check_result(const Topology& topo,
                  const Device& device,
                  const Scheduler& sched,
                  const vector<size_t>& assign) {
    Checker checker{topo, device, sched.get_operations(), assign};

    checker.test_operations();
    cout << "Check passed." << endl;
}

void dump_result(const json& conf,
                 const vector<size_t>& assign,
                 const unique_ptr<Scheduler>& sched) {
    cout << "dumping..." << endl;
    fstream out_file;
    out_file.open(json_get<string>(conf, "output"), fstream::out);
    json jj;
    jj["initial"] = assign;
    sched->to_json(jj);
    jj["final_cost"] = sched->get_final_cost();
    out_file << jj;
}
