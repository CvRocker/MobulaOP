#include "packed_func.h"
#include <iostream>
using namespace std;

using TVMFunc = void(*)(TVMArgs, TVMRetValue*);

void test_func(TVMArgs args, TVMRetValue* rv) {
  cout << "CALL FUNC" << endl;
  for (int i = 0; i < args.num_args; ++i) {
    // dltensor()
    cout << args.values[i].v_handle << endl;
  }
}

void set_stream(TVMArgs args, TVMRetValue* rv) {
  int dev_type = args.values[0].v_int64;
  int dev_id = args.values[1].v_int64;
  void* strm = args.values[2].v_handle;
  cout << dev_type << ", " << dev_id << ", " << strm << endl;
}

PackedFunc WrapAsyncCall;

extern "C" {

void SetMXTVMBridge(int(*MXTVMBridge)(PackedFunc)) {
  MXTVMBridge(PackedFunc([](TVMArgs args, TVMRetValue*) {
    WrapAsyncCall = *static_cast<PackedFunc*>(args.values[1].v_handle);
  }));
}

PackedFunc packed_func;

void RegisterFunc() {
  PackedFunc *func = new PackedFunc(test_func);
  PackedFunc *fset_stream = new PackedFunc(set_stream);
  TVMValue *values = new TVMValue[3];
  int *type_codes = new int[3]{kFuncHandle, kFuncHandle, kDLInt};
  values[0].v_handle = func;
  values[1].v_handle = fset_stream;
  values[2].v_int64 = 0; // num_const
  TVMArgs args(values, type_codes, 3);
  TVMRetValue *rv = new TVMRetValue;
  WrapAsyncCall.CallPacked(args, rv);
  packed_func = *static_cast<PackedFunc*>(rv->value_.v_handle);
}


void CallPackedFunc(void* p) {
  TVMValue *values2 = new TVMValue[1];
  values2[0].v_handle = p;
  int *type_codes2 = new int[1]{kTVMNDArrayTypeCode};
  TVMArgs args2(values2, type_codes2, 1);
  TVMRetValue *rv2 = new TVMRetValue;
  packed_func.CallPacked(args2, rv2);
}

};
