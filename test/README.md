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
