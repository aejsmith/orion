import os, sys

# Add the path to our build utilities to the path.
sys.path = [os.path.abspath(os.path.join('dev', 'build'))] + sys.path

env = Environment(ENV = os.environ)

if not ARGUMENTS.get('V'):
    def compile_str(msg, var):
        return '\033[1;34m%8s\033[0m %s' % (msg, var)
    env['ARCOMSTR']     = compile_str('AR',     '$TARGET')
    env['CCCOMSTR']     = compile_str('CC',     '$SOURCE')
    env['SHCCCOMSTR']   = compile_str('CC',     '$SOURCE')
    env['CXXCOMSTR']    = compile_str('CXX',    '$SOURCE')
    env['SHCXXCOMSTR']  = compile_str('CXX',    '$SOURCE')
    env['LINKCOMSTR']   = compile_str('LINK',   '$TARGET')
    env['SHLINKCOMSTR'] = compile_str('SHLINK', '$TARGET')
    env['RANLIBCOMSTR'] = compile_str('RANLIB', '$TARGET')
    env['GENCOMSTR']    = compile_str('GEN',    '$TARGET')
    env['OBJGENCOMSTR'] = compile_str('OBJGEN', '$TARGET')

env['CCFLAGS'] += [
    # Optimization/debugging flags.
    '-O2', '-g',

    # Warning flags.
    '-Wall', '-Wextra', '-Wno-variadic-macros', '-Wno-unused-parameter',
    '-Wwrite-strings', '-Wmissing-declarations', '-Wredundant-decls',
    '-Wno-format', '-Wno-unused-function', '-Wno-comment',
    '-Wno-unused-private-field',
]

env['CXXFLAGS'] += [
    '-Wsign-promo', '-std=c++14'
]

env['CPPDEFINES'] = {
    'ORION_BUILD_DEBUG': None,
    'GLM_FORCE_CXX11': None,
    'GLM_FORCE_RADIANS': None,
    'GLM_FORCE_SIZE_T_LENGTH': None,
}

env['CPPPATH'] = [
    Dir('engine/include'),
    Dir('engine/3rdparty/glm'),
    Dir('engine/3rdparty/rapidjson/include'),
    Dir('engine/3rdparty/imgui'),
    Dir('engine/3rdparty/mustache'),
]

env['LIBS'] = []

env['CC'] = 'clang'
env['CXX'] = 'clang++'
if os.uname()[0] == 'Darwin':
    env['CXXFLAGS'] += ['-stdlib=libc++']
    env['LINKFLAGS'] += ['-stdlib=libc++']
    env['CPPPATH'] += ['/opt/local/include']
    env['FRAMEWORKS'] = ['OpenGL']
else:
    env['LIBS'] += ['GL']

# Configure libraries.
env['LIBS'] += ['GLEW']
env.ParseConfig('sdl2-config --cflags --libs')
env.ParseConfig('pkg-config --cflags --libs freetype2')
env.ParseConfig('pkg-config --cflags --libs bullet')

Export('env')
SConscript('SConscript', variant_dir = 'build')
