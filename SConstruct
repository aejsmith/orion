import os, sys

# Add the path to our build utilities to the path.
sys.path = [os.path.abspath(os.path.join('dev', 'build'))] + sys.path

import util

# Configurable build options.
opts = Variables('.options.cache')
opts.AddVariables(
                ('APP',          'Application to build (cubes)', 'cubes'),
                ('BUILD',        'Build type to perform (debug, release)', 'release'),
                ('GPU_API',      'GPU API to use (gl, vulkan)', 'gl'),
    BoolVariable('MICROPROFILE', 'Enable MicroProfile', False),
)

env = Environment(ENV = os.environ, variables = opts)
opts.Save('.options.cache', env)

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
        'CPPDEFINES': {'ORION_BUILD_DEBUG': 1},
    },
    'release': {
        'CPPDEFINES': {},
    }
}

if not env['BUILD'] in build_types:
    util.StopError("Invalid build type '%s'." % (env['BUILD']))

if sys.platform.startswith('linux'):
    env['PLATFORM'] = 'linux'
    env['CPPDEFINES'] = {'ORION_PLATFORM_LINUX': 1}

    platform_build_types = {
        'debug': {
            'CCFLAGS': ['-g'],
            'LINKFLAGS': [],
        },
        'release': {
            'CCFLAGS': ['-O2'],
            'LINKFLAGS': [],
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
    env['CPPDEFINES'] = {'ORION_PLATFORM_WIN32': 1}

    platform_build_types = {
        'debug': {
            'CCFLAGS': ['/Od', '/Z7'],
            'LINKFLAGS': ['/DEBUG'],
        },
        'release': {
            'CCFLAGS': ['/O2'],
            'LINKFLAGS': ['/DEBUG'],
        }
    }

    env['CCFLAGS'] += ['/W2', '/EHsc', '/MT']
    env['LINKFLAGS'] += ['/SUBSYSTEM:CONSOLE']
else:
    util.StopError("Unsupported platform.")

env['CCFLAGS'] += platform_build_types[env['BUILD']]['CCFLAGS']
env['LINKFLAGS'] += platform_build_types[env['BUILD']]['LINKFLAGS']
env['CPPDEFINES'].update(build_types[env['BUILD']]['CPPDEFINES'])
env['CPPPATH'] = []
env['LIBPATH'] = []
env['LIBS'] = []

if env['MICROPROFILE']:
    env['CPPDEFINES']['ORION_MICROPROFILE'] = 1

########################
# Component management #
########################

class BuildManager:
    def __init__(self, baseEnv):
        self.components = {}
        self.baseEnv = baseEnv

    def AddComponent(self, **kwargs):
        name = kwargs['name']

        depends = kwargs['depends'] if 'depends' in kwargs else []
        if type(depends) != list:
            depends = [depends]

        include_path = kwargs['include_path'] if 'include_path' in kwargs else []
        if type(include_path) != list:
            include_path = [include_path]

        lib_path = kwargs['lib_path'] if 'lib_path' in kwargs else []
        if type(lib_path) != list:
            lib_path = [lib_path]

        libs = kwargs['libs'] if 'libs' in kwargs else []
        if type(libs) != list:
            libs = [libs]

        dlls = kwargs['dlls'] if 'dlls' in kwargs else []
        if type(dlls) != list:
            dlls = [dlls]

        objects = kwargs['objects'] if 'objects' in kwargs else []

        self.components[name] = {
            'depends': depends,
            'include_path': include_path,
            'lib_path': lib_path,
            'libs': libs,
            'dlls': dlls,
            'objects': objects,
        }

    def CreateEnvironment(self, **kwargs):
        env = self.baseEnv.Clone()

        env['COMPONENT_OBJECTS'] = []
        env['COMPONENT_DLLS'] = []

        def addDepends(depends):
            for dep in depends:
                if not dep in self.components:
                    util.StopError("Unknown component '%s'." % (dep))
                component = self.components[dep]
                addDepends(component['depends'])
                env['CPPPATH'] += component['include_path']
                env['LIBPATH'] += component['lib_path']
                env['LIBS'] += component['libs']
                env['COMPONENT_OBJECTS'] += component['objects']
                env['COMPONENT_DLLS'] += component['dlls']

        depends = kwargs['depends'] if 'depends' in kwargs else []
        if type(depends) != list:
            depends = [depends]
        addDepends(depends)

        return env

manager = BuildManager(env)
Export('manager')

def orionBaseApplicationMethod(env, dir, **kwargs):
    name = kwargs['name']
    sources = kwargs['sources']
    flags = kwargs['flags'] if 'flags' in kwargs else {}

    target = env.Program(os.path.join(str(dir), name), sources + env['COMPONENT_OBJECTS'], **flags)

    for dll in env['COMPONENT_DLLS']:
        copy = util.Copy(env, os.path.join(str(dir), os.path.basename(str(dll))), dll)
        Depends(target, copy)

    return target

def orionApplicationMethod(env, **kwargs):
    return orionBaseApplicationMethod(env, env['APP_DIR'], **kwargs)

def orionInternalApplicationMethod(env, **kwargs):
    return orionBaseApplicationMethod(env, Dir('.'), **kwargs)

env.AddMethod(orionApplicationMethod, 'OrionApplication')
env.AddMethod(orionInternalApplicationMethod, 'OrionInternalApplication')

#########################
# External dependencies #
#########################

SConscript(dirs = [os.path.join('deps', env['PLATFORM'])])

##############
# Main build #
##############

SConscript('SConscript', variant_dir = os.path.join('build', env['BUILD']))
