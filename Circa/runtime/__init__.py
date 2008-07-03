
# Contains standard Circa symbols
#

import boolean, comparison, containers
import ca_struct
import control_flow
import debugging
import ca_module
import feedback, files, python, ca_math, meta, variables
import strings

def createFunctions(codeUnit):
    boolean.createFunctions(codeUnit)
    ca_math.createFunctions(codeUnit)
    ca_module.createTerms(codeUnit)
    ca_struct.createTerms(codeUnit)
    comparison.createFunctions(codeUnit)
    control_flow.createFunctions(codeUnit)
    containers.createFunctions(codeUnit)
    debugging.createFunctions(codeUnit)
    feedback.createFunctions(codeUnit)
    files.createFunctions(codeUnit)
    meta.createFunctions(codeUnit)
    python.createFunctions(codeUnit)
    strings.createFunctions(codeUnit)
    variables.createFunctions(codeUnit)
