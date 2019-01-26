import mxnet as mx
from mxnet.base import _LIB
import ctypes
import time

a = mx.nd.array([1,2,3])

lib = ctypes.CDLL('./lib.so')
lib.SetMXTVMBridge.argtypes = [ctypes.c_void_p]
lib.SetMXTVMBridge(_LIB.MXTVMBridge)
lib.RegisterFunc.restype = ctypes.c_void_p
pfunc = lib.RegisterFunc('Hello')
lib.CallPackedFunc.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
lib.CallPackedFunc(pfunc, a.handle)

mx.nd.waitall()
print(a)
