import mxnet as mx
from mxnet.base import _LIB
import ctypes
import time

a = mx.nd.array([1, 2, 3])

lib = ctypes.CDLL('./lib.so')
lib.SetMXTVMBridge.argtypes = [ctypes.c_void_p]
lib.SetMXTVMBridge(_LIB.MXTVMBridge)
lib.RegisterTVMFunc.restype = ctypes.c_void_p
lib.RegisterTVMFunc.argtypes = [ctypes.c_char_p, ctypes.c_void_p, ctypes.c_int, ctypes.c_void_p]
pfunc = lib.RegisterTVMFunc(b'Hello', lib.test_func, 0, 0)
lib.CallPackedFunc.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
lib.CallPackedFunc(pfunc, a.handle)

mx.nd.waitall()
print(a)
