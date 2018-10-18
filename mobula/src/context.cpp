#include "context/context.h"

namespace mobula {}

#if USING_CUDA
void set_device(const int device_id) {
  int current_device;
  CHECK_CUDA(cudaGetDevice(&current_device));
  if (current_device != device_id) {
    CHECK_CUDA(cudaSetDevice(device_id));
  }
}
#elif USING_HIP
void set_device(const int device_id) {
  int current_device;
  HIP_CHECK(hipGetDevice(&current_device));
  if (current_device != device_id) {
    HIP_CHECK(hipSetDevice(device_id));
  }
}
#else   // USING_CUDA else
void set_device(const int /*device_id*/) {
  throw "Doesn't support setting device on CPU mode";
}
#endif  // USING_CUDA endif
