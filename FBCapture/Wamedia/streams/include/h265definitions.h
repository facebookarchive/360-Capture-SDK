#pragma once
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef __cplusplus
namespace libwamediastreams
{
#endif // __cplusplus

#define HEVC_NAL_UNIT_START_MAX_BYTES           (4)
#define HEVC_EMULATION_PREVENTION_TEST_BYTES    (3)
#define HEVC_NAL_UNIT_START_CODE                (0x00000001)
#define HEVC_PIXELS_PER_MACROBLOCK              (16)
	
typedef enum
{
  HEVC_NAL_UNIT_TYPE_TRAIL_N          = 0,
  HEVC_NAL_UNIT_TYPE_TRAIL_R,        // 1
  HEVC_NAL_UNIT_TYPE_TSA_N,          // 2
  HEVC_NAL_UNIT_TYPE_TSA_R,          // 3
  HEVC_NAL_UNIT_TYPE_STSA_N,         // 4
  HEVC_NAL_UNIT_TYPE_STSA_R,         // 5
  HEVC_NAL_UNIT_TYPE_RADL_N,         // 6
  HEVC_NAL_UNIT_TYPE_RADL_R,         // 7
  HEVC_NAL_UNIT_TYPE_RASL_N,         // 8
  HEVC_NAL_UNIT_TYPE_RASL_R,         // 9
  HEVC_NAL_UNIT_TYPE_RSV_VCL_N10,    // 10
  HEVC_NAL_UNIT_TYPE_RSV_VCL_R11,    // 11
  HEVC_NAL_UNIT_TYPE_RSV_VCL_N12,    // 12
  HEVC_NAL_UNIT_TYPE_RSV_VCL_R13,    // 13
  HEVC_NAL_UNIT_TYPE_RSV_VCL_N14,    // 14
  HEVC_NAL_UNIT_TYPE_RSV_VCL_R15,    // 15
  HEVC_NAL_UNIT_TYPE_BLA_W_LP,       // 16
  HEVC_NAL_UNIT_TYPE_BLA_W_RADL,     // 17
  HEVC_NAL_UNIT_TYPE_BLA_N_LP,       // 18
  HEVC_NAL_UNIT_TYPE_IDR_W_RADL,     // 19
  HEVC_NAL_UNIT_TYPE_IDR_N_LP,       // 20
  HEVC_NAL_UNIT_TYPE_CRA_NUT,        // 21
  HEVC_NAL_UNIT_TYPE_RSV_IRAP_VCL22, // 22
  HEVC_NAL_UNIT_TYPE_RSV_IRAP_VCL23, // 23
  HEVC_NAL_UNIT_TYPE_RSV_VCL24,      // 24
  HEVC_NAL_UNIT_TYPE_RSV_VCL25,      // 25
  HEVC_NAL_UNIT_TYPE_RSV_VCL26,      // 26
  HEVC_NAL_UNIT_TYPE_RSV_VCL27,      // 27
  HEVC_NAL_UNIT_TYPE_RSV_VCL28,      // 28
  HEVC_NAL_UNIT_TYPE_RSV_VCL29,      // 29
  HEVC_NAL_UNIT_TYPE_RSV_VCL30,      // 30
  HEVC_NAL_UNIT_TYPE_RSV_VCL31,      // 31
  HEVC_NAL_UNIT_TYPE_VPS_NUT,        // 32
  HEVC_NAL_UNIT_TYPE_SPS_NUT,        // 33
  HEVC_NAL_UNIT_TYPE_PPS_NUT,        // 34
  HEVC_NAL_UNIT_TYPE_AUD_NUT,        // 35
  HEVC_NAL_UNIT_TYPE_EOS_NUT,        // 36
  HEVC_NAL_UNIT_TYPE_EOB_NUT,        // 37
  HEVC_NAL_UNIT_TYPE_FD_NUT,         // 38
  HEVC_NAL_UNIT_TYPE_PREFIX_SEI_NUT, // 39
  HEVC_NAL_UNIT_TYPE_SUFFIX_SEI_NUT, // 40
  HEVC_NAL_UNIT_TYPE_RSV_NVCL41,     // 41
  HEVC_NAL_UNIT_TYPE_RSV_NVCL42,     // 42
  HEVC_NAL_UNIT_TYPE_RSV_NVCL43,     // 43
  HEVC_NAL_UNIT_TYPE_RSV_NVCL44,     // 44
  HEVC_NAL_UNIT_TYPE_RSV_NVCL45,     // 45
  HEVC_NAL_UNIT_TYPE_RSV_NVCL46,     // 46
  HEVC_NAL_UNIT_TYPE_RSV_NVCL47,     // 47
  HEVC_NAL_UNIT_TYPE_UNSPEC48,       // 48
  HEVC_NAL_UNIT_TYPE_UNSPEC49,       // 49
  HEVC_NAL_UNIT_TYPE_UNSPEC50,       // 50
  HEVC_NAL_UNIT_TYPE_UNSPEC51,       // 51
  HEVC_NAL_UNIT_TYPE_UNSPEC52,       // 52
  HEVC_NAL_UNIT_TYPE_UNSPEC53,       // 53
  HEVC_NAL_UNIT_TYPE_UNSPEC54,       // 54
  HEVC_NAL_UNIT_TYPE_UNSPEC55,       // 55
  HEVC_NAL_UNIT_TYPE_UNSPEC56,       // 56
  HEVC_NAL_UNIT_TYPE_UNSPEC57,       // 57
  HEVC_NAL_UNIT_TYPE_UNSPEC58,       // 58
  HEVC_NAL_UNIT_TYPE_UNSPEC59,       // 59
  HEVC_NAL_UNIT_TYPE_UNSPEC60,       // 60
  HEVC_NAL_UNIT_TYPE_UNSPEC61,       // 61
  HEVC_NAL_UNIT_TYPE_UNSPEC62,       // 62
  HEVC_NAL_UNIT_TYPE_UNSPEC63,       // 65
} eHEVC_NAL_UNIT_TYPE;

#pragma pack(1)

typedef struct 
{
  union
  {
    uint32_t nNALUnitStart;     // can be 00 00 00 01 (Annex B) or 4 bytes of size
    uint8_t  chNALUnitStart[NAL_UNIT_START_MAX_BYTES];
  } delimiter;

  uint8_t    nuh_layer_id_MSB      : 1;
  uint8_t    nal_unit_type         : 6;
  uint8_t    forbidden_zero_bit    : 1;

  uint8_t    nuh_temporal_id_plus1 : 3;
  uint8_t    nuh_layer_id_LSBs     : 5;
} HEVC_NAL_UNIT_START;


#pragma pack()

#ifdef __cplusplus
}; // namespace libwamediastreams
#endif //__cplusplus
