import os, sys

# Add the path to our build utilities to the path.
sys.path = [os.path.abspath(os.path.join('utilities', 'build'))] + sys.path

env = Environment(ENV = os.environ)

if not ARGUMENTS.get('V'):
    env['ARCOMSTR']     = ' AR     $TARGET'
    env['CCCOMSTR']     = ' CC     $SOURCE'
    env['SHCCCOMSTR']   = ' CC     $SOURCE'
    env['CXXCOMSTR']    = ' CXX    $SOURCE'
    env['SHCXXCOMSTR']  = ' CXX    $SOURCE'
    env['LINKCOMSTR']   = ' LINK   $TARGET'
    env['SHLINKCOMSTR'] = ' SHLINK $TARGET'
    env['RANLIBCOMSTR'] = ' RANLIB $TARGET'

env['CCFLAGS'] += [
    '-Wall', '-Wextra', '-Wno-variadic-macros', '-Wno-unused-parameter',
    '-Wwrite-strings', '-Wmissing-declarations', '-Wredundant-decls',
    '-Wno-format', '-Wno-unused-function', '-g', '-Wno-comment',
    '-Wno-unused-private-field',
]

env['CXXFLAGS'] += [
    '-Wsign-promo', '-std=c++11'
]

env['CPPDEFINES'] = {
    'ORION_BUILD_DEBUG': None,
    'GLM_FORCE_CXX11': None,
    'GLM_FORCE_RADIANS': None,
}

env['CPPPATH'] = [
    Dir('engine/include'),
    Dir('3rdparty/glm'),
]

env['LIBS'] = []

if os.uname()[0] == 'Darwin':
    env['CC'] = 'clang'
    env['CXX'] = 'clang++'
    env['CXXFLAGS'] += ['-stdlib=libc++']
    env['LINKFLAGS'] += ['-stdlib=libc++']
    env['CPPPATH'] += ['/opt/local/include']
    env['FRAMEWORKS'] = ['OpenGL']
else:
    env['LIBS'] += ['GL']

# Configure libraries.
env['LIBS'] += ['GLEW']
env.ParseConfig('sdl2-config --cflags --libs')

Export('env')
SConscript('SConscript', variant_dir = 'build')
