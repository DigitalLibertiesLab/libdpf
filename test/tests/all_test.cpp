#include <cstdlib>

int main()
{
    system("./bin/dpf_key_test");

    system("./bin/eval_point_test");
    system("./bin/eval_interval_test");
    system("./bin/eval_full_test");
    system("./bin/eval_sequence_test");

    system("./bin/eval_point_multi_test");
    system("./bin/eval_interval_multi_test");
    system("./bin/eval_full_multi_test");
    system("./bin/eval_sequence_multi_test");

    system("./bin/advice_bit_iterable_test");
    system("./bin/parallel_bit_iterable_test");
    system("./bin/setbit_index_iterable_test");

    return 0;
}
