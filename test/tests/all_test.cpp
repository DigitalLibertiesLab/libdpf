#include <cstdlib>

int main()
{
    system("./bin/dpf_key_test");                // # skipcq: CXX-A1002
    system("./bin/wildcard_test");               // # skipcq: CXX-A1002

    system("./bin/eval_point_test");             // # skipcq: CXX-A1002
    system("./bin/eval_interval_test");          // # skipcq: CXX-A1002
    system("./bin/eval_full_test");              // # skipcq: CXX-A1002
    system("./bin/eval_sequence_test");          // # skipcq: CXX-A1002

    system("./bin/eval_point_multi_test");       // # skipcq: CXX-A1002
    system("./bin/eval_interval_multi_test");    // # skipcq: CXX-A1002
    system("./bin/eval_full_multi_test");        // # skipcq: CXX-A1002
    system("./bin/eval_sequence_multi_test");    // # skipcq: CXX-A1002

    system("./bin/advice_bit_iterable_test");    // # skipcq: CXX-A1002
    system("./bin/parallel_bit_iterable_test");  // # skipcq: CXX-A1002
    system("./bin/setbit_index_iterable_test");  // # skipcq: CXX-A1002

    return 0;
}
