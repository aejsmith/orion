Import('env')

# Build the engine.
engine = SConscript(dirs = ['engine'])

# Build the game.
game = SConscript(dirs = ['game'])
env.Program('orion', engine + game, LINK = '$CXX')
