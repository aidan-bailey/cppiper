# cppiper
Named pipe ipc library (WIP).

## Requirements
- `C++17` capable compiler
- [cmake](https://cmake.org/)
- [spdlog](https://github.com/gabime/spdlog) for logging

## Building

Create and enter the build directory.

``` sh
mkdir build
cd build
```

Initialise build files.

``` sh
cmake ..    # add the flag `-D DEV=ON` for debug logging to stdout
```

Build `cppiper`.

``` sh
cmake --build .
```

## Installing

After the build step, `cppiper` can be installed as a system lib.

``` sh
sudo cmake --install .
```

## TODO
- [ ] cppiper dynamic lib
- [ ] docs
- [ ] proper testing using [cache2](https://github.com/catchorg/Catch2)
- [ ] garbage collector for pipe manager
