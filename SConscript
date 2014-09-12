Import('env')

engine = SConscript(dirs = ['engine'])
game = SConscript(dirs = ['game'])

env.Program('orion', engine + game, LINK = '$CXX')
