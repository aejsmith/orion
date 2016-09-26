Import('env')

# Build the engine.
engine = SConscript(dirs = ['engine'])

# Build applications.
SConscript(dirs = ['apps'], exports = ['engine'])
