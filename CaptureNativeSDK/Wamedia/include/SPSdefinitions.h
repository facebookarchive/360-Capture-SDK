#pragma once
#include "h264parserenums.h"
#include <inttypes.h>

#ifdef __cplusplus
namespace libwamediastreams
{
#endif // __cplusplus

#define EXTENDED_SAR (255)
#define NAL_UNIT_START_MAX_BYTE_LENGTH (4)

typedef struct 
{
  uint32_t NALUnitDelimiter;
  uint8_t  bForbiddenBit;
  uint8_t  NALRefIDC;
  uint8_t  NALUnitType;
} NAL_UNIT_START_LAYOUT;

typedef struct
{
  uint8_t  reserved_zero_2bits : 2;
  uint8_t  constraint_set5_flag: 1;
  uint8_t  constraint_set4_flag: 1;
  uint8_t  constraint_set3_flag: 1;
  uint8_t  constraint_set2_flag: 1;
  uint8_t  constraint_set1_flag: 1;
  uint8_t  constraint_set0_flag: 1;
} CONSTRAINT_FLAGS_BYTE;

typedef struct
{
  uint32_t chroma_format_idc;
  // if (chroma_format_idc == 3)
  // {
  uint8_t  residual_colour_transform_flag;// 1 bit
  // }

  uint32_t bit_depth_luma_minus8;
  uint32_t bit_depth_chroma_minus8;
  uint8_t  qpprime_y_zero_transform_bypass_flag; // 1 bit
  uint8_t  seq_scaling_matrix_present_flag; // 1 bit
  // if( seq_scaling_matrix_present_flag )
  // {
  //SCALING_MATRIX  scalingMatrix;
} PROFILE_SPECIFIC_ADDITION_LAYOUT;

typedef struct
{
  uint32_t nElementOffsets[NUMBER_OF_SCHED_SEL_SYNTAX_ELEMENTS];

  uint32_t bit_rate_value_minus1;
  uint32_t cpb_size_value_minus1;
  int8_t   cbr_flag;

} SCHED_SEL_LAYOUT;

typedef struct
{
  uint32_t cpb_cnt_minus1;

  uint8_t  bit_rate_scale; // 4 bits
  uint8_t  cpb_size_scale; // 4 bits

  SCHED_SEL_LAYOUT* pSchedSel;
  uint8_t  initial_cpb_removal_delay_length_minus1; // 5 bits
  uint8_t  cpb_removal_delay_length_minus1; // 5 bits
  uint8_t  dpb_output_delay_length_minus1; // 5 bits
  uint8_t  time_offset_length; // 5 bits
} HRD_PARAMETERS_LAYOUT;

typedef struct
{
  uint8_t  aspect_ratio_info_present_flag; // 1 bit

//  if( aspect_ratio_info_present_flag ) {

  uint8_t  aspect_ratio_idc;       // 8 bits
  // if( aspect_ratio_idc = = Extended_SAR ) {
  uint16_t sar_width;
  uint16_t sar_height;
  //    }
  // }

  uint8_t  overscan_info_present_flag;   // 1 bit
  // if( overscan_info_present_flag )
  uint8_t  overscan_appropriate_flag;    // 1 bit

  uint8_t  video_signal_type_present_flag; // 1 bit
  // if( video_signal_type_present_flag ) {
  uint8_t  video_format;         // 3 bits
  uint8_t  video_full_range_flag;      // 1 bits
  uint8_t  colour_description_present_flag;// 1 bit
  // if( colour_description_present_flag ) {
  uint8_t  colour_primaries;       // 8 bits
  uint8_t  transfer_characteristics;   // 8 bits
  uint8_t  matrix_coefficients;      // 8 bits
  uint8_t  chroma_loc_info_present_flag; // 1 bit
  // if( chroma_loc_info_present_flag ) {
  uint32_t chroma_sample_loc_type_top_field;
  uint32_t chroma_sample_loc_type_bottom_field;

  uint8_t  timing_info_present_flag;   // 1 bit
  // if( timing_info_present_flag ) {
  uint32_t num_units_in_tick;        // 32 bits
  uint32_t time_scale;           // 32 bits
  uint8_t  fixed_frame_rate_flag;      // 1 bit

  uint8_t  nal_hrd_parameters_present_flag;// 1 bit
  //if( nal_hrd_parameters_present_flag )
  HRD_PARAMETERS_LAYOUT nal_hrd_parameters;

  uint8_t  vcl_hrd_parameters_present_flag;// 1 bit
  // if( vcl_hrd_parameters_present_flag )
  HRD_PARAMETERS_LAYOUT vcl_hrd_parameters;
  // if( nal_hrd_parameters_present_flag | | vcl_hrd_parameters_present_flag )
  uint8_t  low_delay_hrd_flag;       // 1 bit
  uint8_t  pic_struct_present_flag;    // 1 bit
  uint8_t  bitstream_restriction_flag;   // 1 bit
  //if( bitstream_restriction_flag ) {
  uint8_t  motion_vectors_over_pic_boundaries_flag; // 1 bit
  uint32_t max_bytes_per_pic_denom;
  uint32_t max_bits_per_mb_denom;
  uint32_t log2_max_mv_length_horizontal;
  uint32_t log2_max_mv_length_vertical;
  uint32_t num_reorder_frames;
  uint32_t max_dec_frame_buffering;

} VUI_PARAMETERS_LAYOUT;

typedef struct
{
  uint32_t nElementOffsets[NUMBER_OF_SPS_SYNTAX_ELEMENTS];

  NAL_UNIT_START_LAYOUT nal_unit_start;

  uint8_t profile_idc;

  CONSTRAINT_FLAGS_BYTE constraintFlags;

  uint8_t  level_idc;

  uint32_t seq_parameter_set_id;
  PROFILE_SPECIFIC_ADDITION_LAYOUT  profile_specific_addition;
  uint32_t log2_max_frame_num_minus4;
  uint32_t pic_order_cnt_type;
  // if(pic_order_cnt_type == 0)
  uint32_t log2_max_pic_order_cnt_lsb_minus4;
  // else {
  uint8_t  delta_pic_order_always_zero_flag; // 1 bit
  int32_t  offset_for_non_ref_pic;       // ************ SIGNED se(v)
  int32_t  offset_for_top_to_bottom_field;     // ************ SIGNED se(v)
  uint32_t num_ref_frames_in_pic_order_cnt_cycle;
  // for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
  //    offset_for_ref_frame[ i ] 0 se(v)
  int32_t* p_offset_for_ref_frame;       // array holding SIGNNED se(v)
  // }

  uint32_t max_num_ref_frames;
  uint8_t  gaps_in_frame_num_value_allowed_flag; // 1 bit;
  uint32_t pic_width_in_mbs_minus1;
  uint32_t pic_height_in_map_units_minus1;
  uint8_t  frame_mbs_only_flag; // 1 bit;
  // if(0 == frame_mbs_only_flag )
  uint8_t  mb_adaptive_frame_field_flag; // 1 bit
  uint8_t  direct_8x8_inference_flag; // 1 bit
  uint8_t  frame_cropping_flag; // 1 bit

  // if( frame_cropping_flag ) {
  uint32_t frame_crop_left_offset;
  uint32_t frame_crop_right_offset;
  uint32_t frame_crop_top_offset;
  uint32_t frame_crop_bottom_offset;
  // }

  uint8_t  vui_parameters_present_flag; // 1 bit
  // if( vui_parameters_present_flag )
  VUI_PARAMETERS_LAYOUT vui_parameters;
  uint8_t  rbsp_stop_one_bit;           // 1 bit
  uint32_t number_of_rbsp_trailing_bits;

} SPS_LAYOUT;


#ifdef __cplusplus
}; // namespace libwamediastreams
#endif //__cplusplus
