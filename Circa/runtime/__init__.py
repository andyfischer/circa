
# Contains standard Circa symbols
#

import boolean
import ca_math
import ca_module
import comparison
import containers
import control_flow
import debugging
import feedback
import files
import language
import meta
import python
import strings
import variables

def createFunctions(codeUnit):
    # Make sure to do containers first
    containers.createFunctions(codeUnit)

    boolean.createFunctions(codeUnit)
    ca_math.createFunctions(codeUnit)
    ca_module.createTerms(codeUnit)
    comparison.createFunctions(codeUnit)
    control_flow.createFunctions(codeUnit)
    debugging.createFunctions(codeUnit)
    feedback.createFunctions(codeUnit)
    files.createFunctions(codeUnit)
    meta.createFunctions(codeUnit)
    python.createFunctions(codeUnit)
    strings.createFunctions(codeUnit)
    variables.createFunctions(codeUnit)
