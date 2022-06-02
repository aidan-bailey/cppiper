# cppiper
Named pipe ipc library (WIP).

## Requirements
- `C++17` capable compiler
- [cmake](https://cmake.org/)
- [spdlog](https://github.com/gabime/spdlog) for logging

## Building

``` sh
mkdir build
cd build
cmake ..    # add the flag `-D DEV=ON` for debug logging to stdout
cmake --build .
```

## Installing

After the build step, `cppiper` can be installed as a system lib.

``` sh
sudo cmake --install .
```

## Benchmark

A benchmark executable, `benchmark`, will be built along with the library. This doubles as an example of usage, the source code of which can be found in the *benchmark* directory.

## TODO
- [ ] cppiper dynamic lib
- [ ] docs
- [ ] proper testing using [cache2](https://github.com/catchorg/Catch2)
- [ ] garbage collector for pipe manager
