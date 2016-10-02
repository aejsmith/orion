import os, sys

# Add the path to our build utilities to the path.
sys.path = [os.path.abspath(os.path.join('dev', 'build'))] + sys.path

import util

# Configurable build options.
opts = Variables('.options.cache')
opts.AddVariables(
    ('APP', 'Application to build (cubes)', 'cubes'),
    ('BUILD', 'Build type to perform (debug, release)', 'release'),
    ('GPU_API', 'GPU API to use (gl, vulkan)', 'gl'),
)

env = Environment(ENV = os.environ, variables = opts)
opts.Save('.options.cache', env)
Export('env')

helptext = \
    'The following build options can be set on the command line. These will be saved\n' + \
    'for later invocations of SCons, so you do not need to specify them every time:\n' + \
    opts.GenerateHelpText(env)
Help(helptext)

# Set up pretty build output.
if not ARGUMENTS.get('V'):
    def compileString(msg, var):
        if sys.platform.startswith('win32'):
            return '%8s %s' % (msg, var)
        else:
            return '\033[1;34m%8s\033[0m %s' % (msg, var)
    env['ARCOMSTR']     = compileString('AR',     '$TARGET')
    env['CCCOMSTR']     = compileString('CC',     '$SOURCE')
    env['SHCCCOMSTR']   = compileString('CC',     '$SOURCE')
    env['CXXCOMSTR']    = compileString('CXX',    '$SOURCE')
    env['SHCXXCOMSTR']  = compileString('CXX',    '$SOURCE')
    env['LINKCOMSTR']   = compileString('LINK',   '$TARGET')
    env['SHLINKCOMSTR'] = compileString('SHLINK', '$TARGET')
    env['RANLIBCOMSTR'] = compileString('RANLIB', '$TARGET')
    env['GENCOMSTR']    = compileString('GEN',    '$TARGET')
    env['OBJGENCOMSTR'] = compileString('OBJGEN', '$TARGET')

##################
# Compiler setup #
##################

build_types = {
    'debug': {
        'CPPDEFINES': {'ORION_BUILD_DEBUG': None},
    },
    'release': {
        'CPPDEFINES': {},
    }
}

if not env['BUILD'] in build_types:
    util.StopError("Invalid build type '%s'." % (env['BUILD']))

if sys.platform.startswith('linux'):
    env['PLATFORM'] = 'linux'
    env['CPPDEFINES'] = {'ORION_PLATFORM_LINUX': None}

    platform_build_types = {
        'debug': {
            'CCFLAGS': ['-g'],
        },
        'release': {
            'CCFLAGS': ['-O2'],
        }
    }

    env['CC'] = 'clang'
    env['CXX'] = 'clang++'
    env['CCFLAGS'] += [
        '-Wall', '-Wextra', '-Wno-variadic-macros', '-Wno-unused-parameter',
        '-Wwrite-strings', '-Wmissing-declarations', '-Wredundant-decls',
        '-Wno-format', '-Wno-unused-function', '-Wno-comment',
        '-Wno-unused-private-field',
    ]
    env['CXXFLAGS'] += [
        '-Wsign-promo', '-std=c++14'
    ]
    env['LINKFLAGS'] += ['-pthread']
elif sys.platform.startswith('win32'):
    env['PLATFORM'] = 'win32'
    env['CPPDEFINES'] = {'ORION_PLATFORM_WIN32': None}

    platform_build_types = {
        'debug': {
            'CCFLAGS': ['/Od', '/Z7'],
        },
        'release': {
            'CCFLAGS': ['/O2'],
        }
    }

    env['CCFLAGS'] += ['/W2', '/EHsc']
else:
    util.StopError("Unsupported platform.")

env['CCFLAGS'] += platform_build_types[env['BUILD']]['CCFLAGS']
env['CPPDEFINES'].update(build_types[env['BUILD']]['CPPDEFINES'])
env['CPPPATH'] = []
env['LIBS'] = []

#########################
# External dependencies #
#########################

SConscript(dirs = [os.path.join('deps', env['PLATFORM'])])

##############
# Main build #
##############

SConscript('SConscript', variant_dir = os.path.join('build', env['BUILD']))
