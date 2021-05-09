# PA193 project - parser of security certificates

## Buiding

To build the project CMake version at least 3.8 and a C++ compiler with support for C++17 and std::filesystem is needed.
Clang version at least 7 and gcc at least 8 should be sufficient.

Assuming one is in the root of the project, the parser can be built by running the following commands.

```shell
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ cd src/
$ make parser
```

Other build types include `Release` or `RelWithDebInfo`.

To use a different compiler then the defaul, set the `CXX` environment variable.
Fox example to use clang version 10:

```shell
$ CXX=clang++-10 cmake ..
```

or 

```shell
$ export CXX=clang++-10
$ cmake ..
```

The inluded cmake file provides two options. One can be used to enable building with sanitizers. This should be used only on linux.

```shell
$ cmake -DSANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug ..
```

And the other one generetes build configuration also for the fuzzing targets. These can be then used with any fuzzer, but we used afl++.

```shell
$ cmake -DFUZZ=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

Specifically for afl++ one would need to set the correct compiler as well.

```shell
$ CXX=afl-clang-fast++ cmake -DFUZZ=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```
