
import ca_module, builtins, token

def initialize():
    import bootstrap

def loadModule(file, **options):
    return ca_module.CircaModule.fromFile(file, **options)

tokenize = token.tokenize
