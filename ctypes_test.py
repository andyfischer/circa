
from ctypes import *
libcirca = cdll.LoadLibrary("lib/libcirca.dylib")
libcirca.evaluate_file(libcirca.new_branch(), "space.ca")
