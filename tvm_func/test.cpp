#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include "tvm_packed_func.h"
using namespace std;
using namespace tvm;

class MXNetAsyncFunc {
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

thread_local MXNetAsyncFunc mxnet_async_func[3];

using TVMFunc = void (*)(TVMArgs, TVMRetValue*);
std::map<std::string, tvm::PackedFunc> PACKED_FUNCTIONS;

void set_stream(TVMArgs args, TVMRetValue* rv) {
  int dev_type = args.values[0].v_int64;
  int dev_id = args.values[1].v_int64;
  void* strm = args.values[2].v_handle;
  mxnet_async_func[dev_type].set_stream(dev_type, dev_id, strm);
}

void add_func(float *a) {
  a[0] += 1;
}

PackedFunc WrapAsyncCall;

extern "C" {

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

void SetMXTVMBridge(int (*MXTVMBridge)(PackedFunc)) {
  MXTVMBridge(PackedFunc([](TVMArgs args, TVMRetValue*) {
    if (strcmp(args.values[0].v_str, "WrapAsyncCall") == 0) {
      WrapAsyncCall = *static_cast<PackedFunc*>(args.values[1].v_handle);
      return;
    }
    throw runtime_error("Not register WrapAsyncCall");
  }));
}

PackedFunc* RegisterTVMFunc(const char* name, TVMFunc pfunc, int num_const, int* const_loc) {
  PackedFunc func(pfunc);
  PackedFunc fset_stream(set_stream);
  const int num_args = 3 + num_const;
  TVMValue values[num_args];
  int type_codes[num_args];
  values[0].v_handle = &func; type_codes[0] = kFuncHandle;
  values[1].v_handle = &fset_stream; type_codes[1] = kFuncHandle;
  values[2].v_int64 = num_const; type_codes[2] = kDLInt;
  for (int i = 0; i < num_const; ++i) {
    values[i + 3].v_int64 = const_loc[i];
    type_codes[i + 3] = kDLInt;
  }
  TVMArgs args(values, type_codes, num_args);
  TVMRetValue rv;
  WrapAsyncCall.CallPacked(args, &rv);
  PACKED_FUNCTIONS[name] = *static_cast<PackedFunc*>(rv.value_.v_handle);
  return &PACKED_FUNCTIONS[name];
}

void CallPackedFunc(PackedFunc* pfunc, void* p) {
  /*
  TVMValue values[1];
  values[0].v_handle = p;
  int type_codes[1]{kTVMNDArrayTypeCode};
  TVMArgs args(&values[0], &type_codes[0], 1);
  TVMRetValue rv;
  PackedFunc& packed_func = *pfunc;
  packed_func.CallPacked(args, &rv);
  return;
  */
  PackedFunc& packed_func = *pfunc;
  NDArrayHandle p_ndarray = static_cast<NDArrayHandle>(p);
  packed_func(p_ndarray);
}

};

/*
template <typename F, typename = typename std::enable_if<std::is_function<F>::value>::type>
void RegisterFunc(const char* name, F func) {
  RegisterTVMFunc(name, [=](TVMArgs args, TVMRetValue* rv){
  });
} 
*/
