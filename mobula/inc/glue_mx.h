#ifndef MOBULA_INC_GLUE_MX_H_ 
#define MOBULA_INC_GLUE_MX_H_

#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include "tvm_packed_func.h"


namespace mobula {
using namespace tvm;
using TVMFunc = void (*)(TVMArgs, TVMRetValue*);

class MXNetAsyncCtx {
public:
  void set_stream(int dev_type, int dev_id, void* strm) {
    dev_type_ = dev_type;
    dev_id_ = dev_id;
    strm_ = strm;
  }
private:
  int dev_type_, dev_id_;
  void* strm_;
};

thread_local MXNetAsyncCtx MXNetAsyncCtx[3];

void set_stream(TVMArgs args, TVMRetValue*) {
  int dev_type = args.values[0].v_int64;
  int dev_id = args.values[1].v_int64;
  void* strm = args.values[2].v_handle;
  MXNetAsyncCtx[dev_type].set_stream(dev_type, dev_id, strm);
}

} // namespace mobula

extern "C" {
using namespace mobula;
using namespace tvm;

std::map<std::string, PackedFunc> PACKED_FUNCTIONS;
PackedFunc WrapAsyncCall;

void SetMXTVMBridge(int (*MXTVMBridge)(PackedFunc)) {
  MXTVMBridge(PackedFunc([](TVMArgs args, TVMRetValue*) {
    if (strcmp(args.values[0].v_str, "WrapAsyncCall") == 0) {
      WrapAsyncCall = *static_cast<PackedFunc*>(args.values[1].v_handle);
      return;
    }
    throw std::runtime_error("Not register WrapAsyncCall");
  }));
}

PackedFunc* RegisterTVMFunc(const char* name, TVMFunc pfunc, int num_const, int* const_loc) {
  PackedFunc func(pfunc);
  PackedFunc fset_stream(set_stream);
  const int num_args = 3 + num_const;
  std::vector<TVMValue> values(num_args);
  std::vector<int> type_codes(num_args);
  values[0].v_handle = &func; type_codes[0] = kFuncHandle;
  values[1].v_handle = &fset_stream; type_codes[1] = kFuncHandle;
  values[2].v_int64 = num_const; type_codes[2] = kDLInt;
  for (int i = 0; i < num_const; ++i) {
    values[i + 3].v_int64 = const_loc[i];
    type_codes[i + 3] = kDLInt;
  }
  TVMArgs args(&values[0], &type_codes[0], num_args);
  TVMRetValue rv;
  WrapAsyncCall.CallPacked(args, &rv);
  PACKED_FUNCTIONS[name] = *static_cast<PackedFunc*>(rv.value_.v_handle);
  return &PACKED_FUNCTIONS[name];
}

}


#endif
