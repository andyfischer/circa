
import os

LOADED_MODULES = {}

def initialize():
    from Circa.core import (builtins, ca_codeunit)
    import Circa.core.bootstrap

    global LOADED_MODULES
    LOADED_MODULES['kernel'] = builtins.KERNEL

    _loadStandardModule('parsing')
    _loadStandardModule('master')
    _loadStandardModule('math')

    LOADED_MODULES['main'] = ca_codeunit.CodeUnit()

def importFunction(functionDef):
    from Circa.core import builtins
    from Circa.common import function_builder
    function_builder.importPythonFunction(LOADED_MODULES['main'], functionDef,
            instanceBased=True)

def getGlobal(name):
    for module in LOADED_MODULES.values():
        if module.containsName(name):
            return module.getNamed(name)

    return None

def startReplLoop():
    from Circa.common import command_line
    cl = command_line.CommandLine(LOADED_MODULES['main'])
    cl.doInputLoop()
    
def _loadStandardModule(name):
    from Circa import ca_parser

    filename = os.path.join(os.environ['CIRCA_HOME'], 'stdlib', name + '.ca')
    (errors, codeUnit) = ca_parser.parseFile(filename)

    if errors:
        print "Errors in %s module:" % name
        print "\n".join(map(str,errors))
        return

    global LOADED_MODULES
    LOADED_MODULES[name] = codeUnit

