# Obtain the revision number from the Git repository.
def revision_id():
    from subprocess import Popen, PIPE
    git = Popen(['git', 'rev-parse', '--short', 'HEAD'], stdout = PIPE, stderr = PIPE)
    revision = git.communicate()[0].strip()
    if git.returncode != 0:
        return None
    return revision

# Check whether submodules are up to date.
def check_submodules():
    # FIXME: If SHA does not match, should check if it is ahead or behind
    # the one set in the repo. If it is ahead, that's acceptable.
    try:
        from subprocess import Popen, PIPE
        git = Popen(['git', 'submodule', 'status'], stdout = PIPE, stderr = PIPE)
        modules = git.communicate()[0].split('\n')
        for module in modules:
            if len(module) and module[0] != ' ':
                return False
        return True
    except:
        return True
