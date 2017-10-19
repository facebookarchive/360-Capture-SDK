#pragma once
#include <limits.h>
#include "h264definitions.h"
#include "mp4streamsdefinitions.h"

#ifdef __cplusplus
namespace libwamediastreams
{
#endif // __cplusplus

#define TAG_EXAMINATION_BUFFER_BYTES         (12)

#define H263_PICTURE_START_CODE              (0x000080)
#define H263_PICTURE_START_CODE_BYTES        (3)
#define H263_PEEK_BYTES                      (H263_PICTURE_START_CODE_BYTES + 2)

#define MPEG4_PART2_PEEK_BYTES               (5)

/*
name  start code value (hexadecimal)

video_object_start_code           00 through 1F
video_object_layer_start_code     20 through 2F
reserved                          30 through 3F
fgs_bp_start_code                 40 through 5F
reserved                          60 through AF
visual_object_sequence_start_code B0
visual_object_sequence_end_code   B1
user_data_start_code              B2
group_of_vop_start_code           B3
video_session_error_code          B4
visual_object_start_code          B5
vop_start_code                    B6
slice_start_code                  B7
extension_start_code              B8
fgs_vop_start_code                B9
fba_object_start_code             BA
fba_object_plane_start_code       BB
mesh_object_start_code            BC
mesh_object_plane_start_code      BD
still_texture_object_start_code   BE
texture_spatial_layer_start_code  BF
texture_snr_layer_start_code      C0
texture_tile_start_code           C1
texture_shape_layer_start_code    C2
stuffing_start_code               C3
reserved                          C4-C5
System start codes (see note)     C6 through FF
*/
#define MPEG4_PART2_VIDEO_OBJECT_START_CODE_MIN           (0x00000100)
#define MPEG4_PART2_VIDEO_OBJECT_START_CODE_MAX           (0x0000011F)
#define MPEG4_PART2_VIDEO_OBJECT_LAYER_START_CODE_MIN     (0x00000120)
#define MPEG4_PART2_VIDEO_OBJECT_LAYER_START_CODE_MAX     (0x0000012F)
#define MPEG4_PART2_VISUAL_OBJECT_SEQUENCE_START_CODE     (0x000001B0)
#define MPEG4_PART2_VISUAL_OBJECT_SEQUENCE_END_CODE       (0x000001B1)
#define MPEG4_PART2_USER_DATA_START_CODE                  (0x000001B2)
#define MPEG4_PART2_GROUP_OF_VOP_START_CODE               (0x000001B3)
#define MPEG4_PART2_VIDEO_SESSION_ERROR_CODE              (0x000001B4)
#define MPEG4_PART2_VISUAL_OBJECT_START_CODE              (0x000001B5)
#define MPEG4_PART2_VOP_START_CODE                        (0x000001B6)
#define MPEG4_PART2_SLICE_START_CODE                      (0x000001B7)
#define MPEG4_PART2_EXTENSION_START_CODE                  (0x000001B8)
#define MPEG4_PART2_FGS_VOP_START_CODE                    (0x000001B9)
#define MPEG4_PART2_FBA_OBJECT_START_CODE                 (0x000001BA)
#define MPEG4_PART2_FBA_OBJECT_PLANE_START_CODE           (0x000001BB)
#define MPEG4_PART2_MESH_OBJECT_START_CODE                (0x000001BC)
#define MPEG4_PART2_MESH_OBJECT_PLANE_START_CODE          (0x000001BD)
#define MPEG4_PART2_STILL_TEXTURE_OBJECT_PLANE_START_CODE (0x000001BE)
#define MPEG4_PART2_TEXTURE_SPATIAL_LAYER_START_CODE      (0x000001BF)
#define MPEG4_PART2_TEXTURE_SNR_LAYER_START_CODE          (0x000001C0)
#define MPEG4_PART2_TEXTURE_TILE_LAYER_START_CODE         (0x000001C1)
#define MPEG4_PART2_TEXTURE_SHAPE_LAYER_START_CODE        (0x000001C2)
#define MPEG4_PART2_STUFFING_START_CODE                   (0x000001C3)
#define MPEG4_PART2_RESERVED_START_CODE_MIN               (0x000001C4)
#define MPEG4_PART2_RESERVED_START_CODE_MAX               (0x000001C5)

#define MPEG4_PART2_VOP_CODING_METHOD_I      (0)
#define MPEG4_PART2_VOP_CODING_METHOD_P      (1)
#define MPEG4_PART2_VOP_CODING_METHOD_B      (2)
#define MPEG4_PART2_VOP_CODING_METHOD_SPRITE (3)

#define QCELP_NUMBER_OF_CHANNELS             (1)
#define QCELP_BITS_PER_SAMPLE                (16)
#define QCELP_FRAME_DURATION                 (0.02f)
#define QCELP_SAMPLING_RATE                  (8000)
#define QCELP_SAMPLES_PER_FRAME              (160)
#define QCELP_FRAMES_PER_SECOND              (50)
#define MAX_QCELP_FRAME_BYTE_LENGTH          (35)
#define NUMBER_OF_QCELP_FRAME_START_BYTES    (5)
#define QCELP_TAG_MIN_BYTE_LENGTH            (8)

#define AMR_NUMBER_OF_CHANNELS               (1)
#define AMR_BITS_PER_SAMPLE                  (16)
#define AMR_FRAME_DURATION                   (0.02f)
#define AMRNB_CHANNEL_SAMPLES_PER_FRAME      (160)
#define AMRWB_CHANNEL_SAMPLES_PER_FRAME      (320)
#define AMRNB_SAMPLING_RATE                  (8000)
#define AMRWB_SAMPLING_RATE                  (16000)
#define NUMBER_OF_USED_AMRWB_FRAME_TYPES     (13)

#define MPEG_AUDIO_ID_VERSION_ISO_13818_3    (0)
#define MPEG_AUDIO_ID_VERSION_ISO_11172_3    (1)
#define MPEG_AUDIO_ID_VERSION_ISO_14496_3    (2)

#define MPEG_AUDIO_SAMPLES_PER_LAYER_1_FRAME (384)
#define MPEG_AUDIO_SAMPLES_PER_LAYER_2_FRAME (1152)
#define MPEG_AUDIO_SAMPLES_PER_LAYER_3_FRAME (576)

#define MPEG_AUDIO_MODE_STEREO               (0)
#define MPEG_AUDIO_MODE_JOINT_STEREO         (1)
#define MPEG_AUDIO_MODE_DUAL_CHANNEL         (2)
#define MPEG_AUDIO_MODE_SINGLE_CHANNEL       (3) 

#define MAX_EXPECTED_VIDEO_WIDTH             (1920)
#define MAX_EXPECTED_VIDEO_HEIGHT            (1080)

typedef enum
{
  ID_SCE   = 0, // single channel element
  ID_CPE, // 1     channel pair element
  ID_CCE, // 2     coupling channel element
  ID_LFE, // 3     LFE channel element
  ID_DSE, // 4     data stream element
  ID_PCE, // 5     program config element
  ID_FIL, // 6     fill element
  ID_END  // 7     term
} eAAC_RAW_DATA_BLOCK_ELEMENT_START_CODE;

#define AAC_FRAME_SAMPLES_PER_CHANNEL    (1024)


typedef enum 
{
  AAC_SAMPLING_FREQUENCY_INDEX_96000_Hz = 0,
  AAC_SAMPLING_FREQUENCY_INDEX_88200_Hz,   // 1
  AAC_SAMPLING_FREQUENCY_INDEX_64000_Hz,   // 2
  AAC_SAMPLING_FREQUENCY_INDEX_48000_Hz,   // 3
  AAC_SAMPLING_FREQUENCY_INDEX_44100_Hz,   // 4
  AAC_SAMPLING_FREQUENCY_INDEX_32000_Hz,   // 5
  AAC_SAMPLING_FREQUENCY_INDEX_24000_Hz,   // 6
  AAC_SAMPLING_FREQUENCY_INDEX_22050_Hz,   // 7
  AAC_SAMPLING_FREQUENCY_INDEX_16000_Hz,   // 8
  AAC_SAMPLING_FREQUENCY_INDEX_12000_Hz,   // 9
  AAC_SAMPLING_FREQUENCY_INDEX_11025_Hz,   // 10
  AAC_SAMPLING_FREQUENCY_INDEX__8000_Hz,   // 11
  AAC_SAMPLING_FREQUENCY_INDEX_RESERVED_1, // 0xC
  AAC_SAMPLING_FREQUENCY_INDEX_RESERVED_2, // 0xD
  AAC_SAMPLING_FREQUENCY_INDEX_RESERVED_3, // 0xE
  AAC_SAMPLING_FREQUENCY_INDEX_RESERVED_4  // 0xF
} eAAC_SAMPLING_FREQUENCY;

typedef enum
{
  AAC_CHANNEL_CONFIGURATION_DEFINED_IN_AOT_SPECIFIC_CONFIG = 0,
  AAC_CHANNEL_CONFIGURATION_1_CHANNEL,
  AAC_CHANNEL_CONFIGURATION_2_CHANNELS,
  AAC_CHANNEL_CONFIGURATION_3_CHANNELS,
  AAC_CHANNEL_CONFIGURATION_4_CHANNELS,
  AAC_CHANNEL_CONFIGURATION_5_CHANNELS,
  AAC_CHANNEL_CONFIGURATION_6_CHANNELS,
  AAC_CHANNEL_CONFIGURATION_8_CHANNELS,
  AAC_CHANNEL_CONFIGURATION_RESERVED1,
  AAC_CHANNEL_CONFIGURATION_RESERVED2,
  AAC_CHANNEL_CONFIGURATION_RESERVED3,
  AAC_CHANNEL_CONFIGURATION_RESERVED4,
  AAC_CHANNEL_CONFIGURATION_RESERVED5,
  AAC_CHANNEL_CONFIGURATION_RESERVED6,
  AAC_CHANNEL_CONFIGURATION_RESERVED7,
} eAAC_CHANNEL_CONFIGURATION;

class CProgramConfigElement
{
public: // Multichannel audio settings - probably will not be coming from the mobile device.
/*
Program_config_element() {
  element_instance_tag                                4 uimsbf
  object_type                                         2 uimsbf
  sampling_frequency_index                            4 uimsbf
  num_front_channel_elements                          4 uimsbf
  num_side_channel_elements                           4 uimsbf
  num_back_channel_elements                           4 uimsbf
  num_lfe_channel_elements                            2 uimsbf // this concludes 3 whole bytes
  num_assoc_data_elements                             3 uimsbf
  num_valid_cc_elements                               4 uimsbf

  mono_mixdown_present                                1 uimsbf // this concludes 4 whole bytes
  if ( mono_mixdown_present == 1 )
    mono_mixdown_element_number                       4 uimsbf
  
  stereo_mixdown_present                              1 uimsbf
  if ( stereo_mixdown_present == 1 )
    stereo_mixdown_element_number                     4 uimsbf
  
  matrix_mixdown_idx_present                          1 uimsbf
  if ( matrix_mixdown_idx_present == 1 ) {
    matrix_mixdown_idx                                2 uimsbf
    pseudo_surround_enable                            1 uimsbf
  }
  
  for ( i = 0; i < num_front_channel_elements; i++) {
   front_element_is_cpe[i];                           1 bslbf
   front_element_tag_select[i];                       4 uimsbf
  }

  for ( i = 0; i < num_side_channel_elements; i++) {
    side_element_is_cpe[i];                           1 bslbf
    side_element_tag_select[i];                       4 uimsbf
  }

  for ( i = 0; i < num_back_channel_elements; i++) {
    back_element_is_cpe[i];                           1 bslbf
    back_element_tag_select[i];                       4 uimsbf
  }

  for ( i = 0; i < num_lfe_channel_elements; i++)
    lfe_element_tag_select[i];                        4 uimsbf

  for ( i = 0; i < num_assoc_data_elements; i++)
    assoc_data_element_tag_select[i];                 4 uimsbf

  for ( i = 0; i < num_valid_cc_elements; i++) {
    cc_element_is_ind_sw[i];                          1 bslbf
    valid_cc_element_tag_select[i];                   4 uimsbf
  }

  byte_alignment() 
  comment_field_bytes                                 8 uimsbf
  for ( i = 0; i < comment_field_bytes; i++)
    comment_field_data[i];                            8 uimsbf
} 
*/
};

typedef struct
{
  uint8_t frameLengthFlag;					//  1 bit
  uint8_t dependsOnCoreCoder;				//  1 bit
  //if ( dependsOnCoreCoder ) {
  uint16_t coreCoderDelay;					// 14 bits
  // }
  uint8_t extensionFlag;					//  1 bit
  // if ( ! channelConfiguration ) { 
  	CProgramConfigElement program_config_element;
  //}
  //if ((audioObjectType == 6) || (audioObjectType == 20)) {
  uint8_t layerNr;							//  3 bits
  // }
  //if ( extensionFlag ) {
  //	if ( audioObjectType == 22 ) {
  uint8_t  numOfSubFrame;					//  5 bits
  uint16_t layer_length;					// 11 bslbf
  //	}
  //  if( audioObjectType==17||audioObjectType==19||
  //		audioObjectType == 20 || audioObjectType == 23 ) {
  uint8_t aacSectionDataResilienceFlag;		//  1 bit
  uint8_t aacScalefactorDataResilienceFlag;	//  1 bit
  uint8_t aacSpectralDataResilienceFlag;		//  1 bit
  //  }
  uint8_t extensionFlag3;					//  1 bit
  //  if ( extensionFlag3 ) {
		// tbd in version 3
  //  } 
  //}
} GA_SPECIFIC_CONFIG;

#define SL_CONFIG_DESCRIPTOR_MANDATORY_BYTES          (3)
#define SYNC_EXTENSION_TYPE_SPECTRUM_BAND_REPLICATION (0x2B7)
#define SYNC_EXTENSION_TYPE_PARAMETRIC_STEREO         (0x548)

typedef struct
{
  uint8_t audioObjectTypeFirst5Bits;	// 5 bits initially, if 0x1F, additional bits may be added
  uint8_t audioObjectTypeAddition;	    // 6 bits
  uint8_t audioObjectType;		    // my invention: resultant value
	
  uint8_t samplingFrequencyIndex;	    // 4 bits
  uint32_t samplingFrequency;			// 24 bits
  uint8_t channelConfiguration;		    // 4 bits

  int8_t sbrPresentFlag;                // must be signed type, initially set to -1
  int8_t psPresentFlag;                 // must be signed type, initially set to -1
  uint8_t extensionAudioObjectType5Bits;
  uint8_t extensionAudioObjectTypeAddition;
  uint8_t extensionAudioObjectType;   // my invention: resultant value
  uint8_t extensionSamplingFrequencyIndex;
  uint32_t extensionSamplingFrequency;
  uint8_t extensionChannelConfiguration;

  GA_SPECIFIC_CONFIG GASpecificConfig;

  uint32_t  nBitsToDecodeBeforeSyncExtensionType;
  uint16_t syncExtensionType1;          // 11 bits
  uint32_t  nBitsToDecodeAfterFirstSyncExtensionType;
  uint16_t syncExtensionType2;		    // 11 bits
} AUDIO_SPECIFIC_CONFIG;

/* 
AudioSpecificConfig () {
	audioObjectType = GetAudioObjectType();     5 uimsbf
	samplingFrequencyIndex;											4 bslbf
	if ( samplingFrequencyIndex == 0xf ) {
		samplingFrequency;										   24 uimsbf
	channelConfiguration;											4 bslbf

	sbrPresentFlag = -1;
	psPresentFlag = -1;
	if ( audioObjectType == 5 || audioObjectType == 29 ) { 
		extensionAudioObjectType = 5; 
		sbrPresentFlag = 1;
		if ( audioObjectType == 29 ) { 
			psPresentFlag = 1;
		}
		extensionSamplingFrequencyIndex;							4 uimsbf
		if ( extensionSamplingFrequencyIndex == 0xf )
			extensionSamplingFrequency;								24 uimsbf
		audioObjectType = GetAudioObjectType(); 
		if ( audioObjectType == 22 )
			extensionChannelConfiguration;							4 uimsbf
	} 
	else {
		extensionAudioObjectType = 0;
	}
	switch (audioObjectType) { 
	case 1:
	case 2:
	case 3:
	case 4:
	case 6:
	case 7:
	case 17:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
		GASpecificConfig();
		break: 
	case 8:
		CelpSpecificConfig();
		break; 
	case 9:
		HvxcSpecificConfig();
		break: 
	case 12:
		TTSSpecificConfig();
		break; 
	case 13: 
	case 14: 
	case 15: 
	case 16:
		StructuredAudioSpecificConfig();
		break; 
	case 24:
		ErrorResilientCelpSpecificConfig();
		break; 
	case 25:
		ErrorResilientHvxcSpecificConfig();
		break; 
	case 26: 
	case 27:
		ParametricSpecificConfig(); 
		break;
	case 28: 
		SSCSpecificConfig(); 
		break;
	case 30:
		sacPayloadEmbedding;
		SpatialSpecificConfig();
		break; 
	case 32: 
	case 33: 
	case 34:
		MPEG_1_2_SpecificConfig();
		break; 
	case 35:
		DSTSpecificConfig();
		break; 
	case 36:
		fillBits;
		ALSSpecificConfig();
		break; 
	case 37: 
	case 38:
		SLSSpecificConfig();
		break; 
	case 39:
		ELDSpecificConfig(channelConfiguration);
		break: 
	case 40: 
	case 41:
		SymbolicMusicSpecificConfig();
		break; 
	default:// reserved
	}
	switch (audioObjectType) { 
	case 17:
	case 19:
	case 20:
	case 21: 
	case 22: 
	case 23: 
	case 24: 
	case 25: 
	case 26: 
	case 27: 
	case 39:
		epConfig;													 2 bslbf
		if ( epConfig == 2 || epConfig == 3 ) { 
			ErrorProtectionSpecificConfig();
		}
		if ( epConfig == 3 ) {
			directMapping;											 1 bslbf
			if ( ! directMapping ) { 
				// tbd
			} 
		}
	}
	if ( extensionAudioObjectType != 5 && bits_to_decode() >= 16 ) {
		syncExtensionType;											11 bslbf
		if (syncExtensionType == 0x2b7) {
			extensionAudioObjectType = GetAudioObjectType(); 
			if ( extensionAudioObjectType == 5 ) {
				sbrPresentFlag;										 1 bslbf
				if (sbrPresentFlag == 1) {
					extensionSamplingFrequencyIndex;				 4 uimsbf
					if ( extensionSamplingFrequencyIndex == 0xf ) {
						extensionSamplingFrequency;					24 uimsbf
					}
					if ( bits_to_decode() >= 12 ) {
						syncExtensionType;							11 bslbf	
						if (syncExtensionType == 0x548) {
							psPresentFlag;							 1 uimsbf
						}
					}
				}
			}
			if ( extensionAudioObjectType == 22 ) {
				sbrPresentFlag;										 1 bslbf
				if (sbrPresentFlag == 1) {
					extensionSamplingFrequencyIndex;				 4 uimsbf
					if ( extensionSamplingFrequencyIndex == 0xf ) {
						extensionSamplingFrequency;					24 uimsbf
					}
				}
				extensionChannelConfiguration;						 4 uimsbf
			}
		}
	}
}

GetAudioObjectType() {
	audioObjectType;												 5 uimsbf
	if (audioObjectType == 31) {
		audioObjectType = 32 + audioObjectTypeExt;					 6 uimsbf
	}
	return audioObjectType; 
}

GASpecificConfig ( samplingFrequencyIndex, 
				   channelConfiguration,
				   audioObjectType )
{
	frameLengthFlag;												 1 bslbf
	dependsOnCoreCoder;												 1 bslbf
	if ( dependsOnCoreCoder ) {
		coreCoderDelay;												14 uimbsf
	}
	extensionFlag;													 1 bslbf
	if ( ! channelConfiguration ) { 
		program_config_element ();
	}
	if ((audioObjectType == 6) || (audioObjectType == 20)) {
		layerNr;													 3 uimbsf
	}
	if ( extensionFlag ) {
		if ( audioObjectType == 22 ) {
			numOfSubFrame;											 5 bslbf
			layer_length;											11 bslbf
		}
		if( audioObjectType==17||audioObjectType==19||
			audioObjectType == 20 || audioObjectType == 23 ) {
				aacSectionDataResilienceFlag;						 1 bslbf
				aacScalefactorDataResilienceFlag;					 1 bslbf
				aacSpectralDataResilienceFlag;						 1 bslbf
		}
		extensionFlag3;												 1 bslbf
		if ( extensionFlag3 ) {
			// tbd in version 3
		} 
	}
}
*/

#pragma pack(1)

typedef struct // fixed + variable
{
  uint8_t syncwordMSBs;

  uint8_t protection_absent                  : 1; // LSB
  uint8_t layer                              : 2;
  uint8_t ID                                 : 1;
  uint8_t syncwordLSBs                       : 4; // MSB
												
  uint8_t channel_configurationMSB           : 1; // LSB
  uint8_t private_bit                        : 1;
  uint8_t sampling_frequency_index           : 4;
  uint8_t profile                            : 2; // MSB

  // Lower half-byte belongs to variable header
  uint8_t frame_lengthMSBs                   : 2; // LSB
  uint8_t copyright_identification_bit       : 1;
  uint8_t copyright_identification_start     : 1; // MSB
												
  // Upper half-byte belongs to fixed header
  uint8_t home                               : 1; // last byte of fixed header // LSB
  uint8_t original_copy                      : 1;
  uint8_t channel_configurationLSBs          : 2; // MSB
												
  uint8_t frame_lengthMids                   : 8;	

  uint8_t adts_buffer_fullnessMSBs           : 5;
  uint8_t frame_lengthLSBs                   : 3; // MSB

  uint8_t number_of_raw_data_blocks_in_frame : 2;
  uint8_t adts_buffer_fullnessLSBs	     : 6;

} ADTS_HEADER;

typedef enum 
{
  MP3_SAMPLING_FREQUENCY_INDEX_44100_Hz    =  0,
  MP3_SAMPLING_FREQUENCY_INDEX_48000_Hz,   // 1
  MP3_SAMPLING_FREQUENCY_INDEX_32000_Hz,   // 2
  MP3_SAMPLING_FREQUENCY_INDEX_RESERVED    // 3
} eMP3_SAMPLING_FREQUENCY;

typedef struct
{
  uint8_t syncwordMSBs;

  uint8_t protection_bit     : 1;
  uint8_t layer              : 2;
  uint8_t ID                 : 2;
  uint8_t syncwordLSBs       : 3;

  uint8_t private_bit        : 1;
  uint8_t padding_bit        : 1;
  uint8_t sampling_frequency : 2;
  uint8_t bitrate_index      : 4;
	
  uint8_t emphasis           : 2;
  uint8_t original_home      : 1;
  uint8_t copyright          : 1;
  uint8_t mode_extension     : 2;
  uint8_t mode               : 2;
} MP3_HEADER;

typedef struct tagCRC_CHECKSUM
{
  uint8_t checkSum[2];
} CRC_CHECKSUM;

const uint8_t ALLOWED_AUDIO_SUBTYPES[] = 
{
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_14496_3,
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_13818_7_MAIN_PROFILE,
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_13818_7_LOW_COMPLEXITY,
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_13818_7_SCALABLE_SAMPLING_RATE,
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_13818_3,
  AUDIO_STREAM_OBJECT_SUBTYPE_ISO_IEC_11172_3,
};

const uint16_t ALLOWED_AUDIO_BITS_PER_SAMPLE[] =
{
   8,
  16,
  24,
  32,
}; 

const uint16_t ALLOWED_AUDIO_CHANNELS_CONFIGURATIONS[] = 
{
  1,
  2,
  3,
  4,
  5,
  6,
  8
};

const uint16_t ALLOWED_AUDIO_SAMPLING_RATES[] = 
{
    8000,
   11025,
   12000,
   16000,
   22050,
   24000,
   32000,
   44100,
   48000,
};

const uint8_t QCELP_FRAME_BYTE_LENGTHS[NUMBER_OF_QCELP_FRAME_START_BYTES] = 
{
   1,
   4,
   8,
  17,
  35,
};

typedef enum
{
  AMRNB_FRAME_TYPE_4_75_KBPS              = 0,
  AMRNB_FRAME_TYPE_5_15_KBPS,           //  1
  AMRNB_FRAME_TYPE_5_90_KBPS,           //  2
  AMRNB_FRAME_TYPE_6_70_KBPS_PDC_EFR,   //  3
  AMRNB_FRAME_TYPE_7_40_KBPS_TDMA_EFR,  //  4
  AMRNB_FRAME_TYPE_7_95_KBPS,           //  5
  AMRNB_FRAME_TYPE_10_2_KBPS,           //  6
  AMRNB_FRAME_TYPE_12_2_KBPS_GSM_EFR,   //  7
  AMRNB_FRAME_TYPE_AMR_SID,             //  8
  AMRNB_FRAME_TYPE_GSM_EFR_SID,         //  9
  AMRNB_FRAME_TYPE_TDMA_EFR_SID,        // 10
  AMRNB_FRAME_TYPE_PDC_EFR_SID,         // 11
  AMRNB_FRAME_TYPE_RESERVED_1,          // 12
  AMRNB_FRAME_TYPE_RESERVED_2,          // 13
  AMRNB_FRAME_TYPE_SPEECH_LOST,         // 14
  AMRNB_FRAME_TYPE_NO_DATA,             // 15
  NUMBER_OF_AMRNB_FRAME_TYPES           // 16
} eAMRNB_FRAME_TYPE;

typedef enum
{
  AMRWB_FRAME_TYPE_6_60_KBPS              = 0,
  AMRWB_FRAME_TYPE_8_85_KBPS,           //  1
  AMRWB_FRAME_TYPE_12_65_KBPS,          //  2
  AMRWB_FRAME_TYPE_14_25_KBPS,          //  3
  AMRWB_FRAME_TYPE_15_85_KBPS,          //  4
  AMRWB_FRAME_TYPE_18_25_KBPS,          //  5
  AMRWB_FRAME_TYPE_19_85_KBPS,          //  6
  AMRWB_FRAME_TYPE_23_05_KBPS,          //  7
  AMRWB_FRAME_TYPE_23_85_KBPS,          //  8
  AMRWB_FRAME_TYPE_SID,                 //  9
  AMRWB_FRAME_TYPE_RESERVED_1,          // 10
  AMRWB_FRAME_TYPE_RESERVED_2,          // 11
  AMRWB_FRAME_TYPE_RESERVED_3,          // 12
  AMRWB_FRAME_TYPE_RESERVED_4,          // 13
  AMRWB_FRAME_TYPE_SPEECH_LOST,         // 14
  AMRWB_FRAME_TYPE_NO_DATA,             // 15
  NUMBER_OF_AMRWB_FRAME_TYPES           // 16
} eAMRWB_FRAME_TYPE;

const uint8_t AMRNB_FRAME_BYTE_LENGTHS[NUMBER_OF_AMRNB_FRAME_TYPES] = 
{
  13, //  0  AMR 4.75 13
  14, //  1  AMR 5.15 14
  16, //  2  AMR 5.9  16
  18, //  3  AMR 6.7  18
  20, //  4  AMR 7.4  20
  21, //  5  AMR 7.95 21
  27, //  6  AMR 10.2 27
  32, //  7  AMR 12.2 32
   6, //  8  AMR      SID
   6, //  9  GSM-EFR  SID
   6, // 10 TMDA-EFR SID
   6, // 11 PDC-EFR  SID
   0, // 12 reserved
   0, // 13 reserved
   0, // 14 reserved
   1  // 15 NO DATA
};

const char AMRWB_FRAME_BYTE_LENGTHS[NUMBER_OF_USED_AMRWB_FRAME_TYPES] = 
{
  18, //  0 AMR-WB 6.60 kbit/s
  24, //  1 AMR-WB 8.85 kbit/s
  33, //  2 AMR-WB 12.65 kbit/s
  37, //  3 AMR-WB 14.25 kbit/s
  41, //  4 AMR-WB 15.85 kbit/s
  47, //  5 AMR-WB 18.25 kbit/s
  51, //  6 AMR-WB 19.85 kbit/s
  59, //  7 AMR-WB 23.05 kbit/s
  61, //  8 AMR-WB 23.85 kbit/s
   6, //  9 AMR-WB SID (Comfort Noise Frame)
   0, // 10 
   1, // 11 
   1, // 12
};

typedef struct
{
  uint8_t m_unused                 : 2;
  uint8_t m_nQuality               : 1;
  uint8_t m_nFrameTypeIndex        : 4;
  uint8_t m_bFollowedByOtherFrames : 1;
} AMR_BANDWIDTH_EFFICICENT_FRAME_HEADER; 

class CDAMRExtension
{
public:
  uint8_t  chVendor[4];
  uint8_t  decoder_version;
  uint16_t mode_set;
  uint16_t mode_change_period;
  uint8_t  frames_per_sample;
};

// QCELP-related definitions

typedef struct
{
  uint8_t  m_RIFF[4];
  uint32_t m_riff_size;
  uint8_t  m_QLCM[4];
} RIFF_QLCM;

//
// FMT chunk
//
const uint8_t QCELP_GUID_1[16] = {
  (uint8_t)0x41, (uint8_t)0x6D, (uint8_t)0x7F, (uint8_t)0x5E, 
  (uint8_t)0x15, (uint8_t)0xB1, (uint8_t)0xD0, (uint8_t)0x11, 
  (uint8_t)0xBA, (uint8_t)0x91, (uint8_t)0x00, (uint8_t)0x80,
  (uint8_t)0x5F, (uint8_t)0xB4, (uint8_t)0xB9, (uint8_t)0x7E}
;

const uint8_t QCELP_GUID_2[16] = {
  (uint8_t)0x42, (uint8_t)0x6D, (uint8_t)0x7F, (uint8_t)0x5E, 
  (uint8_t)0x15, (uint8_t)0xB1, (uint8_t)0xD0, (uint8_t)0x11, 
  (uint8_t)0xBA, (uint8_t)0x91, (uint8_t)0x00, (uint8_t)0x80,
  (uint8_t)0x5F, (uint8_t)0xB4, (uint8_t)0xB9, (uint8_t)0x7E
};

const uint8_t EVRC_GUID[16] = { 
  (uint8_t)0x8D, (uint8_t)0xD4, (uint8_t)0x89, (uint8_t)0xE6,
  (uint8_t)0x76, (uint8_t)0x90, (uint8_t)0xB5, (uint8_t)0x46,
  (uint8_t)0xEF, (uint8_t)0x91, (uint8_t)0x73, (uint8_t)0x6A,
  (uint8_t)0x51, (uint8_t)0x00, (uint8_t)0xCE, (uint8_t)0xB4
};

const uint8_t SMV_GUID[16]	= { 
  (uint8_t)0x75, (uint8_t)0x2B, (uint8_t)0x7C, (uint8_t)0x8D,
  (uint8_t)0x97, (uint8_t)0xA7, (uint8_t)0x49, (uint8_t)0xED,
  (uint8_t)0x5E, (uint8_t)0x98, (uint8_t)0xD5, (uint8_t)0x3C,
  (uint8_t)0x8C, (uint8_t)0xC7, (uint8_t)0x5F, (uint8_t)0x84
};

typedef struct
{
  uint8_t   m_codecGUID[16];
  uint16_t  m_codec_version;
  uint8_t   m_codec_name[80];
  uint16_t  m_average_bps;
  uint16_t  m_packet_size;
  uint16_t  m_block_size;
  uint16_t  m_sampling_rate;
  uint16_t  m_sample_size;
  uint32_t  m_num_rates;
} QCELP_CODEC_INFO;

typedef struct
{
  uint8_t m_rate_size;
  uint8_t m_rate_octet;
} QCELP_RATE_ITEM;

typedef struct
{
  uint8_t          m_FMT[4];
  uint32_t         m_chunk_size;
  uint8_t          m_major;
  uint8_t          m_minor;
  QCELP_CODEC_INFO m_codec_info;
  QCELP_RATE_ITEM  m_QCELP_rates[18]; // QCELP encoders prefer this allocation
} QCELP_FMT;

typedef struct
{
  uint8_t  m_VRAT[4];
  uint32_t m_chunk_size;
  uint32_t m_var_rate_flag;
  uint32_t m_size_in_packets;
} QCELP_VRAT;

typedef struct
{
  uint8_t  m_LABL[4];
  uint32_t m_chunk_size;
  uint8_t  m_label[48];
} QCELP_LABL;

typedef struct
{
  uint8_t  m_OFFS[4];
  uint32_t m_chunk_size;
  uint32_t m_step_size;
  uint32_t m_num_offsets;
} QCELP_OFFS;

typedef struct
{
  uint8_t  m_DATA[4];
  uint32_t m_chunk_size;
} QCELP_DATA;

typedef struct
{
  uint8_t  m_CNFG[4];
  uint32_t m_chunk_size;
  uint16_t m_config;
} QCELP_CNFG;

typedef struct
{
  uint8_t  m_TEXT[4];
  uint32_t m_chunk_size;
  uint8_t  m_string[PATH_MAX];
} QCELP_TEXT;

class CDQCPExtension
{
public:
  uint8_t  chVendor[4];
  uint8_t  decoder_version;
  uint8_t  frames_per_sample;
};

#pragma pack()

#ifdef __cplusplus
}; // namespace libwamediastreams
#endif //__cplusplus
