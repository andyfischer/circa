
def initialize():
    import Circa.core.bootstrap


def importFunction(functionDef):
    from Circa.core import builtins
    from Circa.common import function_builder
    function_builder.importPythonFunction(builtins.KERNEL, functionDef,
            instanceBased=True)
