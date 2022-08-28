#include "stackoverflow_terminate.hxx"

#include <boost/stacktrace.hpp>

using namespace std;

void on_terminate() {
    try {
        auto unknown = current_exception();
        if (unknown) {
            cout << boost::stacktrace::stacktrace() << "\n";
            abort();
        } else {
            cerr << "normal termination" << endl;
        }
    } catch (const exception& e) {  // for proper `` exceptions
        cerr << "unexpected exception: " << e.what() << endl;
    } catch (...) {  // last resort for things like `throw 1;`
        cerr << "unknown exception" << endl;
    }
}
