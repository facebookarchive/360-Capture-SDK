/****************************************************************************************************************

Filename	:	LibRTMP.cpp
Content		:	Streaming video which is encoded from HW(NVIDIA/AMD) encoder parts.
Created		:	Feb 2, 2017
Authors		:	Homin Lee, Dennis Won

Copyright	:

****************************************************************************************************************/
#include "LibRTMP.h"

namespace FBCapture {
  namespace Video {

    LibRTMP::LibRTMP() {
      file_ = NULL;
      rtmp_ = NULL;
      packet_ = NULL;
      initializedRTMPSession_ = false;
      timestamp_ = 0;
      preFrameTime_ = 0;
      lastFrameTime_ = 0;
    }

    LibRTMP::~LibRTMP() {
      close();
    }

    int LibRTMP::ReadU8(uint32_t *u8, FILE* fp) {
      if (fread(u8, 1, 1, fp) != 1)
        return 0;
      return 1;
    }

    int LibRTMP::ReadU16(uint32_t *u16, FILE* fp) {
      if (fread(u16, 2, 1, fp) != 1)
        return 0;
      *u16 = HTON16(*u16);
      return 1;
    }

    int LibRTMP::ReadU24(uint32_t *u24, FILE*fp) {
      if (fread(u24, 3, 1, fp) != 1)
        return 0;
      *u24 = HTON24(*u24);
      return 1;
    }

    int LibRTMP::ReadU32(uint32_t *u32, FILE*fp) {
      if (fread(u32, 4, 1, fp) != 1)
        return 0;
      *u32 = HTON32(*u32);
      return 1;
    }

    int LibRTMP::PeekU8(uint32_t *u8, FILE*fp) {
      if (fread(u8, 1, 1, fp) != 1)
        return 0;
      fseek(fp, -1, SEEK_CUR);
      return 1;
    }

    int LibRTMP::ReadTime(uint32_t *utime, FILE*fp) {
      if (fread(utime, 4, 1, fp) != 1)
        return 0;
      *utime = HTONTIME(*utime);
      return 1;
    }

    bool LibRTMP::initSocket() {
#ifdef WIN32
      WORD version;
      WSADATA wsaData;
      version = MAKEWORD(2, 2);
      return (WSAStartup(version, &wsaData) == 0);
#else
      return TRUE;
#endif
    }

    FBCAPTURE_STATUS LibRTMP::initializeRTMP(const string& streamUrl) {
      int has_audio, has_video;
      int packetSize = 1024 * 512;

      rtmp_ = (RTMP*)malloc(sizeof(RTMP));
      RTMP_Init(rtmp_);
      rtmp_->Link.timeout = 60;

      flvtagInit(&tag_);

      DEBUG_LOG_VAR("Live streaming url: ", streamUrl);
      if (!RTMP_SetupURL(rtmp_, (char*)streamUrl.c_str())) {
        DEBUG_ERROR("Failed to setup RTMP with streaming url.");
        return FBCAPTURE_STATUS_INVALID_STREAM_URL;
      }

      //if unable,the AMF command would be 'play' instead of 'publish'
      RTMP_EnableWrite(rtmp_);

      if (!RTMP_Connect(rtmp_, NULL)) {
        DEBUG_ERROR("Failed to connect to RTMP");
        return FBCAPTURE_STATUS_RTMP_CONNECTION_FAILED;
      }

      if (!RTMP_ConnectStream(rtmp_, 0)) {
        DEBUG_ERROR("Failed to connect stream");
        return FBCAPTURE_STATUS_RTMP_CONNECTION_FAILED;
      }

      packet_ = (RTMPPacket*)malloc(sizeof(RTMPPacket));
      memset(packet_, 0, sizeof(RTMPPacket));
      RTMPPacket_Reset(packet_);
      RTMPPacket_Alloc(packet_, packetSize);

      return FBCAPTURE_STATUS_OK;
    }

    int LibRTMP::sendFlvData(RTMP* r, const char* buf, int size) {
      RTMPPacket* pkt = &r->m_write;
      char* enc;
      int s2 = size, ret, num;

      pkt->m_nChannel = 0x04;   /* source channel */
      pkt->m_nInfoField2 = r->m_stream_id;

      while (s2) {
        if (!pkt->m_nBytesRead) {
          if (size < 11) {
            /* FLV pkt too small */
            return 0;
          }

          if (buf[0] == 'F' && buf[1] == 'L' && buf[2] == 'V') {
            buf += 13;
            s2 -= 13;
          }

          pkt->m_packetType = *buf++;
          pkt->m_nBodySize = AMF_DecodeInt24(buf);
          buf += 3;
          pkt->m_nTimeStamp = AMF_DecodeInt24(buf);
          buf += 3;
          pkt->m_nTimeStamp |= *buf++ << 24;
          buf += 3;
          s2 -= 11;

          if (((pkt->m_packetType == RTMP_PACKET_TYPE_AUDIO
                || pkt->m_packetType == RTMP_PACKET_TYPE_VIDEO) &&
               !pkt->m_nTimeStamp) || pkt->m_packetType == RTMP_PACKET_TYPE_INFO) {
            pkt->m_headerType = RTMP_PACKET_SIZE_LARGE;
          } else {
            pkt->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
          }

          if (!RTMPPacket_Alloc(pkt, pkt->m_nBodySize)) {
            DEBUG_ERROR_VAR("failed to allocate packet", to_string(pkt->m_nBodySize));
            return FALSE;
          }

          enc = pkt->m_body;
        } else {
          enc = pkt->m_body + pkt->m_nBytesRead;
        }

        num = pkt->m_nBodySize - pkt->m_nBytesRead;

        if (num > s2) {
          num = s2;
        }

        memcpy(enc, buf, num);
        pkt->m_nBytesRead += num;
        s2 -= num;
        buf += num;

        if (pkt->m_nBytesRead == pkt->m_nBodySize) {
          ret = RTMP_SendPacket(r, pkt, FALSE);
          RTMPPacket_Free(pkt);
          pkt->m_nBytesRead = 0;

          if (!ret) {
            return -1;
          }

          buf += 4;
          s2 -= 4;

          if (s2 < 0) {
            break;
          }
        }
      }

      return size + s2;
    }

    void LibRTMP::flvtagInit(flvtag_t* tag) {
      memset(tag, 0, sizeof(flvtag_t));
    }

    void LibRTMP::flvtagFree(flvtag_t* tag) {
      if (tag->data) {
        free(tag->data);
      }

      flvtagInit(tag);
    }

    int LibRTMP::flvtagReserve(flvtag_t* tag, uint32_t size) {
      size += FLV_TAG_HEADER_SIZE + FLV_TAG_FOOTER_SIZE;

      if (size > tag->aloc) {
        tag->data = (uint8_t*)realloc(tag->data, size);
        tag->aloc = size;
      }

      return 0;
    }

    FBCAPTURE_STATUS LibRTMP::flvReadHeader(FILE* flv, int* has_audio, int* has_video) {
      uint8_t h[FLV_HEADER_SIZE];

      if (FLV_HEADER_SIZE != fread(&h[0], 1, FLV_HEADER_SIZE, flv)) {
        return FBCAPTURE_STATUS_INVALID_FLV_HEADER;
      }

      if ('F' != h[0] || 'L' != h[1] || 'V' != h[2]) {
        return FBCAPTURE_STATUS_INVALID_FLV_HEADER;
      }

      (*has_audio) = h[4] & 0x04;
      (*has_video) = h[4] & 0x01;

      return FBCAPTURE_STATUS_OK;
    }

    FBCAPTURE_STATUS LibRTMP::connectRTMPWithFlv(const wstring& streamUrl, const wstring& videoFile) {
      FBCAPTURE_STATUS status = FBCAPTURE_STATUS_OK;

      int has_audio, has_video;
      string serverURL;
      string serverStreamKey;
      uint32_t type = 0;
      uint32_t datalength = 0;
      uint32_t streamid = 0;

      long lastTime = 0;
      int nextIsKey = 1;
      int packetSize = 1024 * 512;
      uint32_t preTagSize = 0;


      wstring_convert<std::codecvt_utf8<wchar_t>> stringTypeConversion;
      videoFileName_ = stringTypeConversion.to_bytes(videoFile);

      file_ = fopen(videoFileName_.c_str(), "rb");
      if (!file_) {
        DEBUG_ERROR_VAR("Open File Error", videoFileName_.c_str());
        return FBCAPTURE_STATUS_FILE_READING_ERROR;
      }

      if ((status = flvReadHeader(file_, &has_audio, &has_video)) != FBCAPTURE_STATUS_OK) {
        DEBUG_ERROR("The video file is not flv");
        return status;
      }

      streamUrl_ = stringTypeConversion.to_bytes(streamUrl);
      if (!initializedRTMPSession_ && ((status = initializeRTMP(streamUrl_)) != FBCAPTURE_STATUS_OK)) {
        DEBUG_ERROR("Failed on initialzing RTMP");
        return status;
      }

      if (!RTMP_IsConnected(rtmp_)) {
        DEBUG_ERROR("RTMP_IsConnected() Error");
        return FBCAPTURE_STATUS_RTMP_DISCONNECTED;
      }

      packet_->m_hasAbsTimestamp = 0;
      packet_->m_nChannel = 0x04;
      packet_->m_nInfoField2 = rtmp_->m_stream_id;
      packet_->m_nTimeStamp = 0;

      fseek(file_, 9, SEEK_SET);
      fseek(file_, 4, SEEK_CUR);

      DEBUG_LOG("Start to send video data to rtmp server");
      while (1) {
        if (!ReadU8(&type, file_))
          break;
        if (!ReadU24(&datalength, file_))
          break;
        if (!ReadTime(&timestamp_, file_))
          break;
        if (!ReadU24(&streamid, file_))
          break;

        if (type != 0x08 && type != 0x09) {
          fseek(file_, datalength + 4, SEEK_CUR);
          continue;
        }

        RTMP_ClientPacket(rtmp_, packet_);

        if (fread(packet_->m_body, 1, datalength, file_) != datalength)
          break;

        // Sending header only once on first flv stream
        if (!initializedRTMPSession_) {
          packet_->m_headerType = RTMP_PACKET_SIZE_LARGE;
        }
        packet_->m_nTimeStamp = timestamp_ + lastFrameTime_;
        packet_->m_packetType = type;
        packet_->m_nBodySize = datalength;
        packet_->m_hasAbsTimestamp = 0;
        preFrameTime_ = packet_->m_nTimeStamp;

        if (!RTMP_IsConnected(rtmp_)) {
          DEBUG_ERROR("RTMP is discounneded");
          status = FBCAPTURE_STATUS_RTMP_DISCONNECTED;
          break;
        }

        if (!RTMP_SendPacket(rtmp_, packet_, true)) {
          DEBUG_ERROR("Failed to send packet");
          status = FBCAPTURE_STATUS_SENDING_RTMP_PACKET_FAILED;
          break;
        }

        if (!ReadU32(&preTagSize, file_))
          break;

        if (!PeekU8(&type, file_))
          break;
        if (type == 0x09) {
          if (fseek(file_, 11, SEEK_CUR) != 0)
            break;
          if (!PeekU8(&type, file_)) {
            break;
          }
          if (type == 0x17)
            nextIsKey = 1;
          else
            nextIsKey = 0;

          fseek(file_, -11, SEEK_CUR);
        }
      }

      lastFrameTime_ = preFrameTime_;

      DEBUG_LOG_VAR("last frame time: ", to_string(lastFrameTime_));
      DEBUG_LOG("Sent video data to rtmp server");

      if (file_) {
        fclose(file_);
        file_ = NULL;
      }
      initializedRTMPSession_ = true;
      remove(videoFileName_.c_str()); // remove flv file

      return status;
    }

    void LibRTMP::close() {

      initializedRTMPSession_ = false;
      lastFrameTime_ = 0;
      timestamp_ = 0;
      preFrameTime_ = 0;

      flvtagFree(&tag_);

      if (rtmp_) {
        RTMP_Close(rtmp_);
        RTMP_Free(rtmp_);
        rtmp_ = NULL;
      }

      if (packet_) {
        RTMPPacket_Free(packet_);
        packet_ = NULL;
      }
    }
  }
}
