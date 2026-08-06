# SmartGA

SmartGA is a C++20 genetic algorithm that searches for mathematical
expressions fitting a set of input and output values. Expressions are encoded
as small instructions, so mutations can replace one operation without parsing
an expression tree.

The product name remains **SmartGA**. Its local directory is named `smart-ga`
only to follow the workspace's lowercase kebab-case naming convention.

The core genetic algorithm has no third-party dependencies. OpenCV-based
circle detection is an optional library and is built only when OpenCV is
available.

The related Python implementation is available in
[SmartGA_Python](https://github.com/AABBCCDKG/SmartGA_Python).

## Features

- Instruction-based formula representation and mutation
- Reproducible runs through an optional random seed
- Input validation and finite similarity scoring
- Standalone CMake/CTest build with no downloaded test dependencies
- Optional OpenCV circle-detection target

## Build

Requirements:

- CMake 3.16 or newer
- A C++20 compiler such as Clang 10+, GCC 10+, or recent MSVC

Configure, build, and run the tests:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Run the core example:

```bash
./build/smartga
```

### Build without OpenCV

OpenCV detection is enabled for discovery by default, but a missing OpenCV
installation does not fail configuration. To explicitly disable detection:

```bash
cmake -S . -B build \
  -DSMARTGA_ENABLE_OPENCV=OFF \
  -DSMARTGA_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

This always builds `SmartGA::core`, the `smartga` example, and the core tests.

### Build with OpenCV detection

Install OpenCV with its CMake package configuration, then configure normally
or explicitly enable detection:

```bash
cmake -S . -B build -DSMARTGA_ENABLE_OPENCV=ON
cmake --build build --parallel
```

When OpenCV's `core`, `imgproc`, and `imgcodecs` components are found, CMake
also creates the `SmartGA::detection` target. Otherwise, CMake prints a status
message and continues with the core-only build.

## Install and Consume

Install SmartGA to a local prefix:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PWD/install"
cmake --build build --parallel
cmake --install build
```

The install contains the `smartga` example, public headers, libraries, and the
`SmartGAConfig.cmake`, version, and target export files. A separate CMake
project can consume the core library without referring to this source tree:

```cmake
find_package(SmartGA 1 CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE SmartGA::core)
```

Configure that consumer with the install prefix:

```bash
cmake -S path/to/consumer -B consumer-build \
  -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build consumer-build
```

`SmartGA::core` is always part of the installed package and has no third-party
dependency. `SmartGA::detection` and `object_detector.h` are installed only
when OpenCV detection was built. In that case, `find_package(SmartGA)` also
requires OpenCV's `core`, `imgproc`, and `imgcodecs` components so the exported
detection target remains usable.

CTest includes an isolated installed-package consumer test. It installs the
current build into a temporary prefix, configures a separate project with
`find_package(SmartGA CONFIG REQUIRED)`, then builds and runs that consumer.

## Project Structure

- `include/expression_function.h`: instruction-based mathematical function
- `include/genetic_algorithm.h`: genetic algorithm public API
- `include/object_detector.h`: OpenCV-independent detection API declaration
- `cmake/SmartGAConfig.cmake.in`: installed package configuration template
- `cmake/VerifyInstalledPackage.cmake`: isolated install/consumer verification
- `src/expression_function.cpp`: expression parsing and evaluation
- `src/genetic_algorithm.cpp`: population evolution and scoring
- `src/object_detector.cpp`: optional OpenCV circle detection
- `src/main.cpp`: core example executable
- `tests/expression_function_test.cpp`: expression-function checks
- `tests/genetic_algorithm_test.cpp`: genetic-algorithm checks
- `tests/consumer`: standalone `find_package` consumer fixture
