# How to build tests

To build the existing tests, simply run the following:

```
mkdir build && cd build
cmake ..
cmake --build .
```

# How to generate coverage

First, when building the tests, make sure `COVERAGE=ON` as follows:

```
cmake -D COVERAGE=ON ..
```

Then after running all tests, run the following:

```
gcovr --root '../..' --filter '\.\./\.\./include/.*' --html-details 'coverage.html' .
```
