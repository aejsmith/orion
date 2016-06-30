import os, sys

# Add the path to our build utilities to the path.
sys.path = [os.path.abspath(os.path.join('dev', 'build'))] + sys.path

# Configurable build options.
opts = Variables('.options.cache')
opts.AddVariables(
    ('GPU_API', 'GPU API to use (gl, vulkan)', 'gl'),
    BoolVariable('DEBUG', 'Whether to perform a debug build', 1),
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

env['CC'] = 'clang'
env['CXX'] = 'clang++'

env['CCFLAGS'] += [
    # Optimization flags.
    '-O2',

    # Warning flags.
    '-Wall', '-Wextra', '-Wno-variadic-macros', '-Wno-unused-parameter',
    '-Wwrite-strings', '-Wmissing-declarations', '-Wredundant-decls',
    '-Wno-format', '-Wno-unused-function', '-Wno-comment',
    '-Wno-unused-private-field',
]

env['CXXFLAGS'] += [
    '-Wsign-promo', '-std=c++14'
]

env['CPPDEFINES'] = {}
env['CPPPATH'] = []
env['LIBS'] = []

# Set debug build flags.
if env['DEBUG']:
    env['CCFLAGS'] += ['-g']
    env['CPPDEFINES']['ORION_BUILD_DEBUG'] = None

#########################
# Platform dependencies #
#########################

if os.uname()[0] == 'Darwin':
    env['CXXFLAGS'] += ['-stdlib=libc++']
    env['LINKFLAGS'] += ['-stdlib=libc++']
    env['CPPPATH'] += ['/opt/local/include']

##############
# Main build #
##############

Export('env')
SConscript('SConscript', variant_dir = 'build')
