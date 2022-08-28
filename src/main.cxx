#include "flow.hxx"
#include "sa_place.hxx"
#include "stackoverflow_terminate.hxx"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: ./qft_mapping <config.json>";
        return 1;
    }

    set_terminate(on_terminate);

    // config file
    ifstream ifs(argv[1]);
    json conf = json::parse(ifs);

    json mapper_conf = json_get<json>(conf, "mapper");

    // flow
    if (json_get<string>(mapper_conf, "placer") == "sa") {
        sa_place(conf);
    } else {
        vector<size_t> dummy;
        flow(conf, dummy, true);
    }
    return 0;
}
