#include "AudioBuffer.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits>
#include <assert.h>

#define LOG() throw /* TODO: FIXME: implement this */

#define CHECK_HRESULT(hr, message) \
if (FAILED(hr)) \
{ \
    LOG(message ": %x.\n", hr); \
    return; \
} \
else \
    void(0)

namespace FBCapture {
  namespace Audio {

    AudioBuffer::~AudioBuffer() {
      for (int i = 0; i < numBuffers_; ++i) {
        Buffer& buff = buffers_[i];
        if (buff.data_ != nullptr) {
          free(buff.data_);
        }
      }

      if (mixBuffer_ != nullptr) {
        free(mixBuffer_);
      }
    }

    void AudioBuffer::initizalize(int numBuffers) {
      numBuffers_ = numBuffers;
      buffers_ = new Buffer[numBuffers];
      memset(buffers_, 0, numBuffers * sizeof(Buffer));
      mixBuffer_ = static_cast<float*>(malloc(kMIX_BUFFER_LENGTH * kSTEREO * sizeof(float)));
    }

    void AudioBuffer::initializeBuffer(int index, int channelCount) {
      if (index >= numBuffers_) {
        return; // out of bounds!
      }

      Buffer& buff = buffers_[index];

      buff.channelCount_ = channelCount;
    }

    void AudioBuffer::write(int index, const float* data, size_t lengthFrames) {
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
        buff.data_ = static_cast<float*>(realloc(buff.data_, requiredLengthFrames * buff.channelCount_ * sizeof(float)));
        buff.length_ = requiredLengthFrames;
      }

      memcpy(buff.data_ + buff.positionFrames_ * buff.channelCount_, data, lengthFrames * buff.channelCount_ * sizeof(float));
      buff.positionFrames_ += lengthFrames;
      assert(buff.positionFrames_ >= 0);
    }

    void AudioBuffer::getBuffer(const float** buffer, size_t* length, bool silenceMode) {
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

      memset(mixBuffer_, 0, kMIX_BUFFER_LENGTH * sizeof(float));

      for (int i = 0; i < numBuffers_; ++i) {
        Buffer& buff = buffers_[i];
        for (int frame = 0; frame < len; ++frame) {
          for (int chan = 0; chan < kSTEREO; ++chan) {
            if (!silenceMode) {
              mixBuffer_[(frame * kSTEREO) + chan] += buff.data_[(frame * buff.channelCount_) + chan % buff.channelCount_];
            }
          }
        }

        // Compact the buffer
        memmove(buff.data_, buff.data_ + (len  * buff.channelCount_), (buff.length_ - len) * sizeof(float) * buff.channelCount_);
        buff.positionFrames_ -= len;
        assert(buff.positionFrames_ >= 0);
      }

      *length = len * kSTEREO;
      *buffer = mixBuffer_;
    }
  }
}
