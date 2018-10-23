/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

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
