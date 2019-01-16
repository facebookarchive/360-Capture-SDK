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

#define NOMINMAX
#include <Audioclient.h>
#include <stdint.h>

namespace FBCapture {
  namespace Audio {

    enum BufferIndex {
      BufferIndex_Headphones = 0,
      BufferIndex_Microphone,
      BufferIndex_Max
    };

    enum BufferNums {
      No_Device = 0,
      Output_Device_Only,
      Input_Output_Device
    };

    class AudioBuffer {
    public:
      AudioBuffer();
      ~AudioBuffer();

      void initizalize(int numBuffers);
      void initializeBuffer(int index, int channelCount); // WAVEFORMATEX *bufferFormat, IAudioClock* clock);
      void write(int index, const int16_t* data, size_t length);
      void getBuffer(const int16_t** buffer, size_t* length, bool enabledAudioCapture, bool enabledMicCapture);
      void releaseBuffers();

    private:
      struct Buffer {
        int16_t* data_;
        int channelCount_;
        int length_;
        int positionFrames_;
      };

      Buffer* buffers_;
      int numBuffers_;
      int16_t* mixBuffer_;
      static const size_t kMIX_BUFFER_LENGTH = 4096; // PAS
      static const int kSTEREO = 2;
    };

  }
}
