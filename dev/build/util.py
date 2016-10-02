import SCons.Defaults
import SCons.Errors
from SCons.Script import *

# Flatten a list/tuple of lists.
def flatten(l):
    import itertools

    if len(l) == 0 or type(l[0]) != list:
        return l
    else:
        return list(itertools.chain.from_iterable(l))

# Raise a stop error.
def StopError(str):
    # Don't break if the user is trying to get help.
    if GetOption('help'):
        Return()
    else:
        raise SCons.Errors.StopError(str)

# Copy a file.
def Copy(env, dest, src):
    # This silences the output of the command as opposed to using Copy directly.
    return env.Command(
        dest, src,
        Action(lambda target, source, env: SCons.Defaults.copy_func(target[0], source[0]), None))
