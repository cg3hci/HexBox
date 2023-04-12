# Hexa Modeling Prototype

## Setup

1. Clone this repository:

```Shell
git clone https://github.com/francescozoccheddu/hexa-modeling-prototype.git
cd hexa-modeling-prototype
```

2. Generate the build system:

```Shell
mkdir out
cd out
cmake ..
```

3. Build:

```Shell
cmake --build .
```

4. Run the `gui` executable.

5. Enjoy! 😉

## Compatibility

Requires [CMake](https://cmake.org/) 3.14+, a modern C++20 compiler, OpenGL 2.0 support and an Internet connection.

Tested on MSVC v143 on Windows 11 and GCC 10 and Clang 14 on Ubuntu 22.10. 

> **NOTE:**  
> With this configuration Cinolib generates hundreds of warnings (OGDF might also generate some warnings on GCC). Compilation should succeed anyway; just ignore them.

> **NOTE:**  
> On Ubuntu, make sure to have `libgl1-mesa-dev`, `libglu1-mesa-dev` and `xorg-dev` installed.

## Future work

See [TODO.md](TODO.md).
