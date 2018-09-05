#pragma once

namespace FBCapture {
namespace Common {

template <typename COMClass> class ScopedCOMPtr {
public:
  ScopedCOMPtr() : ptr(nullptr) {}

  ScopedCOMPtr(COMClass *p) { ptr = p; }

  COMClass *operator=(COMClass *p) {
    if (ptr) {
      ptr->Release();
    }
    ptr = p;
    return ptr;
  }

  operator COMClass *() { return ptr; }

  COMClass *operator->() { return ptr; }

  COMClass **operator&() { return &ptr; }

  ~ScopedCOMPtr() {
    if (ptr) {
      ptr->Release();
    }
  }

private:
  COMClass *ptr;
};

} // namespace Common
} // namespace FBCapture
