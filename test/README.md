# How to build tests

To build the existing tests, simply run the following:

```
mkdir build && cd build
cmake ..
cmake --build .
```

# How to run tests

Specific test executables will be placed in the `bin` subdirectory of the build directory. These can be run individually.

Alternatively the `all_test` executable in the build directory can be run which automatically runs all test executables.

# How to generate coverage

First, when building the tests, make sure `COVERAGE=ON` as follows:

```
cmake -D COVERAGE=ON ..
```

Then after running all tests, run the following for `gcovr` coverage report:

```
mkdir coverage
gcovr --root '../..' --filter '\.\./\.\./include/.*' --html-details 'coverage/index.html' .
```

# Building specific binaries

To build a specific binary, for example `eval_point_test`, use the following:

```
cmake --build . --target eval_point_test
```

# Running specific tests

First, to list available tests use the following (using `eval_point_test` as the example executable):

```
./bin/eval_point_test --gtest_list_tests
```

Next, using the `--gtest_filter` option, specific tests can be run using pattern matching. `?` matches a single character, `*` matches any substring, `:` seperates two patterns, and `-` at the beginning of a pattern makes it a negative pattern. Following is an example to only run `Basic` tests and ensure `BasicPathMemoizer` tests are not run:

```
./bin/eval_point_test --gtest_filter='*Basic*:-*BasicPathMemoizer*'
```
