# Orion

Orion is a hobby 3D game engine project. It currently supports Linux (Windows support is planned).

## License

Orion is licensed under the ISC license, see [license.txt](https://github.com/aejsmith/orion/blob/master/documentation/license.txt) for details.

## Building

After cloning the repository you first need to clone submodules containing some external libraries:

    $ git submodule update --init

You must also install the following requirements:

* SCons
* LLVM/clang
* SDL (2.x)
* freetype2
* Bullet (2.x)

You can then build by running SCons:

    $ scons

And finally run with:

    $ build/release/orion

A number of options can be passed to SCons to configure the build, which will be saved until the next time they are specified. The following options are supported:

* `BUILD`: Either `debug` or `release`. The built program will be in `build/$BUILD`. Debug builds disable optimisation and include many more checks.
* `GPU_API`: Either `gl` or `vulkan`. Selects the GPU backend to use.

Example:

    $ scons BUILD=debug GPU_API=vulkan
