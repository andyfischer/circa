import pdb

from ctypes import *
#cdll.LoadLibrary("libcirca.dylib")
lib = CDLL("libcirca.dylib")

lib.ForeignFunctionTest()
