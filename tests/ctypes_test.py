import pdb

from ctypes import *
circa = CDLL("libcirca.dylib")

circa.initialize()

code = circa.CaCode_alloc(None)
constant = circa.CaCode_createConstant(code, circa.GetGlobal('int'), None, None)
#circa.CaCode_executeFunction(GetGlobal('to-string')
