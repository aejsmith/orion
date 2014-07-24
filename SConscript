Import('env')

engine = SConscript(dirs = ['engine'])
game = SConscript(dirs = ['game'])

env.Program('orion', [game, engine], LINK = '$CXX')
