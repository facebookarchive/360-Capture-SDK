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
