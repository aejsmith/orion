# Orion

Orion is a hobby 3D game engine project. It currently supports Linux and Mac OS X (Windows support is planned).

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

    $ build/orion
