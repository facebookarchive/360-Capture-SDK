/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include "AudioBuffer.h"

#include "Core/Common/Log.h"
#include "Core/Common/Common.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits>
#include <assert.h>

#define CHECK_HRESULT(hr, message) \
if (FAILED(hr)) \
{ \
    LOG(message ": %x.\n", hr); \
    return; \
} \
else \
    void(0)

using namespace FBCapture::Common;

namespace FBCapture {
  namespace Audio {
    AudioBuffer::AudioBuffer() : buffers_(nullptr), numBuffers_(0), mixBuffer_(nullptr) {}

    AudioBuffer::~AudioBuffer() {
      releaseBuffers();
    }

		void AudioBuffer::releaseBuffers()	{

			if (buffers_ == nullptr)
				return;

			for (int i = 0; i < numBuffers_; ++i) {
				Buffer& buff = buffers_[i];
				if (buff.data_ != nullptr) {
					free(buff.data_);
				}
			}

			delete[] buffers_;
			buffers_ = nullptr;

			if (mixBuffer_ != nullptr) {
				free(mixBuffer_);
				mixBuffer_ = nullptr;
			}
		}

    void AudioBuffer::initizalize(int numBuffers) {
      numBuffers_ = numBuffers;
      buffers_ = new Buffer[numBuffers];
      memset(buffers_, 0, numBuffers * sizeof(Buffer));
      mixBuffer_ = static_cast<int16_t*>(malloc(kMIX_BUFFER_LENGTH * kSTEREO * sizeof(int16_t)));
    }

    void AudioBuffer::initializeBuffer(int index, int channelCount) {
      if (index >= numBuffers_) {
        return; // out of bounds!
      }

      Buffer& buff = buffers_[index];

      buff.channelCount_ = channelCount;
    }

    void AudioBuffer::write(int index, const int16_t* data, size_t lengthFrames) {
      if (index >= numBuffers_) {
        return; // out of bounds!
      }

      Buffer& buff = buffers_[index];
      assert(buff.positionFrames_ >= 0);

      const int requiredLengthFrames = buff.positionFrames_ + lengthFrames;

      if (requiredLengthFrames > (1024 * 500)) {
        return; // sanity check!
      }

      if (buff.length_ < requiredLengthFrames) {
        buff.data_ = static_cast<int16_t*>(realloc(buff.data_, requiredLengthFrames * buff.channelCount_ * sizeof(int16_t)));
        buff.length_ = requiredLengthFrames;
      }

      memcpy(buff.data_ + (buff.positionFrames_ * buff.channelCount_), data, lengthFrames * buff.channelCount_ * sizeof(int16_t));
      buff.positionFrames_ += lengthFrames;

      assert(buff.positionFrames_ >= 0);
    }

    void AudioBuffer::getBuffer(const int16_t** buffer, size_t* length, bool enabledAudioCapture, bool enabledMicCapture)	{
      int len = kMIX_BUFFER_LENGTH;

      for (int i = 0; i < numBuffers_; ++i) {
        if (buffers_[i].positionFrames_ < len) {
          assert(buffers_[i].positionFrames_ >= 0);
          len = buffers_[i].positionFrames_;
        }
      }

      if (len <= 0) {
        return;
      }

      memset(mixBuffer_, 0, kMIX_BUFFER_LENGTH * sizeof(int16_t));

			for (int i = 0; i < numBuffers_; ++i) {
				Buffer& buff = buffers_[i];
				for (int frame = 0; frame < len; ++frame) {
					for (int chan = 0; chan < kSTEREO; ++chan) {
						if (enabledAudioCapture && !enabledMicCapture) {  // Mix buffer for input audio data only
							if (i == BufferIndex_Headphones) {
								mixBuffer_[(frame * kSTEREO) + chan] += buff.data_[(frame * buff.channelCount_) + chan % buff.channelCount_];
							}
						} else if (!enabledAudioCapture && enabledMicCapture) {  // Mix buffer for output audio data only
							if (i == BufferIndex_Microphone) {
								mixBuffer_[(frame * kSTEREO) + chan] += buff.data_[(frame * buff.channelCount_) + chan % buff.channelCount_];
							}
						} else if (enabledAudioCapture && enabledMicCapture) {  // Mix buffer for both of input and output audio data
							mixBuffer_[(frame * kSTEREO) + chan] += buff.data_[(frame * buff.channelCount_) + chan % buff.channelCount_];
						}
					}
				}

        // Compact the buffer
        memmove(buff.data_, buff.data_ + (len  * buff.channelCount_), (buff.length_ - len) * sizeof(int16_t) * buff.channelCount_);
        buff.positionFrames_ -= len;
        assert(buff.positionFrames_ >= 0);
      }

      *length = len * kSTEREO;
      *buffer = mixBuffer_;
    }
  }
}
