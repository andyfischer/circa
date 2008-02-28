
import ca_module

def initialize():
    import bootstrap

def loadModule(file, **options):
    return ca_module.CircaModule.fromFile(file, **options)

