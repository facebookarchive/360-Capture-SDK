#pragma once

#ifdef __cplusplus
namespace libwamediastreams
{
#endif // __cplusplus

typedef enum
{
  FLV_SOUND_FORMAT_UNDEFINED = -1,
  FLV_SOUND_FORMAT_LPCM_PLATFORM_ENDIAN = 0, // 0 = Linear PCM, platform endian
  FLV_SOUND_FORMAT_ADPCM,                    // 1 = ADPCM
  FLV_SOUND_FORMAT_MP3,                      // 2 = MP3
  FLV_SOUND_FORMAT_LPCM_LITTLE_ENDIAN,       // 3 = Linear PCM, little endian
  FLV_SOUND_FORMAT_NELLYMOSER_16kHz_MONO,    // 4 = Nellymoser 16 kHz mono
  FLV_SOUND_FORMAT_NELLYMOSER_8kHz_MONO,     // 5 = Nellymoser 8 kHz mono
  FLV_SOUND_FORMAT_NELLYMOSER,               // 6 = Nellymoser
  FLV_SOUND_FORMAT_G_711_A_LOG_PCM,          // 7 = G.711 A-law logarithmic PCM
  FLV_SOUND_FORMAT_G_711_MU_LOG_PCM,         // 8 = G.711 mu-law logarithmic PCM
  FLV_SOUND_FORMAT_RESERVED,                 // 9 = reserved
  FLV_SOUND_FORMAT_AAC,                      // 10 = AAC  , supported in FlashPlayer 9.0.115.0 and higher
  FLV_SOUND_FORMAT_SPEEX,                    // 11 = Speex, supported in FlashPlayer 10 and higher
  FLV_SOUND_FORMAT_MP3_8kHz = 14,            // 14 = MP3 8 kHz
  FLV_SOUND_FORMAT_DEVICE_SPECIFIC_SOUND     // 15 = Device-specific sound
} eFLV_SOUND_FORMAT;

typedef enum
{
  FLV_VIDEO_CODEC_ID_UNDEFINED = 0,
  FLV_VIDEO_CODEC_ID_JPEG = 1, // currently unused
  FLV_VIDEO_CODEC_ID_SORENSON_H263 = 2,
  FLV_VIDEO_CODEC_ID_SCREEN_VIDEO,
  FLV_VIDEO_CODEC_ID_ON2_VP6,
  FLV_VIDEO_CODEC_ID_ON2_VP6_WITH_ALPHA_CHANNEL,
  FLV_VIDEO_CODEC_ID_SCREEN_VIDEO_VERSION_2,
  FLV_VIDEO_CODEC_ID_AVC,
} eFLV_VIDEO_CODEC_ID;

#ifdef __cplusplus
}; // namespace libwamediastreams
#endif //__cplusplus
