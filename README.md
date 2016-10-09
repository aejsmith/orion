# Orion

Orion is a hobby 3D game engine project. It currently supports Linux and Windows (64-bit).

## License

Orion is licensed under the ISC license, see [license.txt](https://github.com/aejsmith/orion/blob/master/documentation/license.txt) for details.

## Prerequisites

### Linux

You must also install the following requirements (including their development packages):

* SCons
* LLVM/clang
* SDL (2.x)
* freetype2
* Bullet (2.x)
* GLEW (when building GL support)
* Vulkan SDK (when building Vulkan support)

### Windows

Building on Windows requires Visual Studio (only 2015 has been tested) and SCons to be installed. All other library dependencies are pre-built and included in the repository. SCons needs to be added to your PATH.

To build Vulkan support, the LunarG Vulkan SDK must be installed.

## Building

After cloning the repository you first need to clone submodules containing some external libraries:

    $ git submodule update --init

You can then build by running SCons:

    $ scons

And finally run the test game (Cubes) with:

    $ build/release/cubes

A number of options can be passed to SCons to configure the build, which will be saved until the next time they are specified. The following options are supported:

* `APP`: Selects the application to build, look in the apps directory for what's available. Defaults to `cubes` (the test game).
* `BUILD`: Either `debug` or `release`. The built program will be in `build/$BUILD`. Debug builds disable optimisation and include many more checks.
* `GPU_API`: Either `gl` or `vulkan`. Selects the GPU backend to use.

Example:

    $ scons BUILD=debug GPU_API=vulkan
