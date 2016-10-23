Import('manager')

# Build the engine.
SConscript(dirs = ['engine'])

# Build applications.
manager.baseEnv['APP_DIR'] = Dir('.')
SConscript(dirs = ['apps'])
