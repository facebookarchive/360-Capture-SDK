#pragma once

#ifdef __cplusplus
namespace libwamediastreams
{
#endif // __cplusplus

typedef enum
{
  VIDEO_STREAM_TYPE_UNDEFINED = 0,
  VIDEO_STREAM_TYPE_H263,
  VIDEO_STREAM_TYPE_AVC,   // H.264
  VIDEO_STREAM_TYPE_MPEG4,
  VIDEO_STREAM_TYPE_ALL_OTHER_MPEGS,
  VIDEO_STREAM_TYPE_HEVC,
} eVIDEO_STREAM_TYPE;

typedef enum
{
  AUDIO_STREAM_TYPE_UNDEFINED = 0,
  AUDIO_STREAM_TYPE_MPEG_AUDIO,
  AUDIO_STREAM_TYPE_AMRNB,
  AUDIO_STREAM_TYPE_AMRWB,
  AUDIO_STREAM_TYPE_QCELP,
  AUDIO_STREAM_TYPE_EXTENDED_AC3
} eAUDIO_STREAM_TYPE;

typedef enum
{
  AUDIO_STREAM_OBJECT_SUBTYPE_UNDEFINED = 0,
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_14496_3 = 0x40,
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_13818_7_MAIN_PROFILE = 0x66,
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_13818_7_LOW_COMPLEXITY = 0x67,
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_13818_7_SCALABLE_SAMPLING_RATE = 0x68,
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_13818_3 = 0x69,
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_11172_3 = 0x6B,
} eAUDIO_STREAM_OBJECT_SUBTYPE;

typedef enum
{
  AUDIO_OBJECT_TYPE_NULL = 0,
  AUDIO_OBJECT_TYPE_AAC_MAIN,                           //  1
  AUDIO_OBJECT_TYPE_AAC_LC,                             //  2
  AUDIO_OBJECT_TYPE_AAC_SSR,                            //  3
  AUDIO_OBJECT_TYPE_AAC_LTP,                            //  4  
  AUDIO_OBJECT_TYPE_SBR,                                //  5   
  AUDIO_OBJECT_TYPE_AAC_SCALABLE,                       //  6
  AUDIO_OBJECT_TYPE_TwinVQ,                             //  7
  AUDIO_OBJECT_TYPE_CELP,                               //  8
  AUDIO_OBJECT_TYPE_HXVC,                               //  9
  AUDIO_OBJECT_TYPE_RESERVED_0,                         // 10
  AUDIO_OBJECT_TYPE_RESERVED_1,                         // 11
  AUDIO_OBJECT_TYPE_TTSI,                               // 12,
  AUDIO_OBJECT_TYPE_MAIN_SYNTHETIC,                     // 13
  AUDIO_OBJECT_TYPE_WAVETABLE_SYNTHESIS,                // 14
  AUDIO_OBJECT_TYPE_GENERAL_MIDI,                       // 15
  AUDIO_OBJECT_TYPE_ALGORITHMIC_SYNTHESIS_AND_AUDIO_FX, // 16
  AUDIO_OBJECT_TYPE_ER_AAC_LC,                          // 17
  AUDIO_OBJECT_TYPE_RESERVED_2,                         // 18
  AUDIO_OBJECT_TYPE_ER_AAC_LTP,                         // 19
  AUDIO_OBJECT_TYPE_ER_AAC_SCALABLE,                    // 20
  AUDIO_OBJECT_TYPE_ER_TWIN_VQ,                         // 21      
  AUDIO_OBJECT_TYPE_ER_BSAC,                            // 22
  AUDIO_OBJECT_TYPE_ER_AAC_LD,                          // 23
  AUDIO_OBJECT_TYPE_ER_CELP,                            // 24
  AUDIO_OBJECT_TYPE_ER_HVXC,                            // 25
  AUDIO_OBJECT_TYPE_ER_HLN,                             // 26
  AUDIO_OBJECT_TYPE_ER_PARAMETRIC,                      // 27
  AUDIO_OBJECT_TYPE_SSC,                                // 28
  AUDIO_OBJECT_TYPE_PS,                                 // 29
  AUDIO_OBJECT_TYPE_MPEG_SURROUND,                      // 30
  AUDIO_OBJECT_TYPE_RESERVED_3,                         // 31
  AUDIO_OBJECT_TYPE_LAYER_1,                            // 32
  AUDIO_OBJECT_TYPE_LAYER_2,                            // 33
  AUDIO_OBJECT_TYPE_LAYER_3,                            // 34   
  AUDIO_OBJECT_TYPE_DST,                                // 35
  AUDIO_OBJECT_TYPE_ALS,                                // 36
  AUDIO_OBJECT_TYPE_SLS,                                // 37
  AUDIO_OBJECT_TYPE_SLS_NON_CORE,                       // 38
  AUDIO_OBJECT_TYPE_ER_AAC_ELD,                         // 39
  AUDIO_OBJECT_TYPE_SMR_SIMPLE,                         // 40
  AUDIO_OBJECT_TYPE_SMR_MAIN,                           // 41
  AUDIO_OBJECT_TYPE_USAC_NO_SBR,                        // 42
  AUDIO_OBJECT_TYPE_SAOC,                               // 43
  AUDIO_OBJECT_TYPE_LD_MPEG_SURROUND,                   // 44
  AUDIO_OBJECT_TYPE_USAC                                // 45
} eAUDIO_OBJECT_TYPE; // as found in AudioSpecificConfig

// These four definitions do not come from the MPEG Audio specs:
#define SBR_TOOL_PRESENT (1)
#define PS_TOOL_PRESENT  (1 << 1)
#define AAC_HE_V1 (SBR_TOOL_PRESENT)
#define AAC_HE_V2 (SBR_TOOL_PRESENT | PS_TOOL_PRESENT)


#ifdef __cplusplus
}; // namespace libwamediastreams
#endif //__cplusplus
