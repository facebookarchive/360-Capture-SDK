#pragma once

namespace FBCapture {
namespace Common {

template <typename T> class ScopedCoMem {
public:
  ScopedCoMem() : ptr(nullptr) {}

  ScopedCoMem(T *p) { ptr = p; }

  T *operator=(T *p) {
    if (ptr) {
      CoTaskMemFree(ptr);
    }
    ptr = p;
    return ptr;
  }

  operator T *() { return ptr; }

  T *operator->() { return ptr; }

  T **operator&() { return &ptr; }

  ~ScopedCoMem() {
    if (ptr) {
      CoTaskMemFree(ptr);
    }
  }

private:
  T *ptr;
};

} // namespace Common
} // namespace FBCapture