# Flatten a list/tuple of lists.
def flatten(l):
    import itertools

    if len(l) == 0 or type(l[0]) != list:
        return l
    else:
        return list(itertools.chain.from_iterable(l))
