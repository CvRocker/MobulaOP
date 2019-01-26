#include "tvm_packed_func.h"
#include <cstring>
#include <iostream>
using namespace std;
using namespace tvm;

using TVMFunc = void(*)(TVMArgs, TVMRetValue*);

void test_func(TVMArgs args, TVMRetValue* rv) {
  cout << "CALL FUNC" << endl;
  for (int i = 0; i < args.num_args; ++i) {
    // dltensor()
    DLTensor* dl_tensor = static_cast<DLTensor*>(args.values[i].v_handle);
    for (int j = 0; j < 3; ++j) {
      ((float*)dl_tensor->data)[j] += 1;
    }
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
    if (strcmp(args.values[0].v_str, "WrapAsyncCall") == 0) {
      WrapAsyncCall = *static_cast<PackedFunc*>(args.values[1].v_handle);
      return;
    }
    throw runtime_error("Not register WrapAsyncCall");
  }));
}

PackedFunc packed_func;

void RegisterFunc() {
  PackedFunc func(test_func);
  PackedFunc fset_stream(set_stream);
  TVMRetValue rv = WrapAsyncCall(func, fset_stream, 0);
  packed_func = *static_cast<PackedFunc*>(rv.value_.v_handle);
}


void CallPackedFunc(void* p) {
  TVMValue values[1];
  values[0].v_handle = p;
  int type_codes[1]{kTVMNDArrayTypeCode};
  TVMArgs args(&values[0], &type_codes[0], 1);
  TVMRetValue rv;
  packed_func.CallPacked(args, &rv);
}

};
