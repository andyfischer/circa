

KERNEL_LOADED = False

KERNEL = None

LOADED_MODULES = {}

def getGlobal(name):
    if KERNEL.definesName(name):
        return KERNEL.getNamed(name)

    for module in LOADED_MODULES.values():
        if module.definesName(name):
            return module.getNamed(name)

    return None
