#ifndef PACKED_FUNC_H_
#define PACKED_FUNC_H_

#include <functional>
#include "dlpack.h"

typedef enum {
  kHandle = 3U,
  kNull = 4U,
  kTVMType = 5U,
  kTVMContext = 6U,
  kArrayHandle = 7U,
  kFuncHandle = 10U,
  kTVMNDArrayTypeCode = 19U,
} TVMTypeCode;

typedef union {
  int64_t v_int64;
  double v_float64;
  void* v_handle;
  const char* v_str;
  DLDataType v_type;
  DLContext v_ctx;
} TVMValue;

class TVMArgs {
 public:
  const TVMValue* values; 
  const int* type_codes;
  int num_args;
  TVMArgs(const TVMValue* values, const int* type_codes, int num_args) : values(values), type_codes(type_codes), num_args(num_args) {}
  inline int size() const {return num_args;}
};

class TVMRetValue {
public:
  TVMValue value_;
  int type_code_;
  TVMRetValue() : type_code_(kNull) {}
};

class PackedFunc {
 public:
  using FType = std::function<void (TVMArgs args, TVMRetValue* rv)>;
  PackedFunc() {};
  explicit PackedFunc(FType body) : body_(body) {}
  inline void CallPacked(TVMArgs args, TVMRetValue* rv) const {
    body_(args, rv);
  }
 private:
  FType body_;
};

#endif
