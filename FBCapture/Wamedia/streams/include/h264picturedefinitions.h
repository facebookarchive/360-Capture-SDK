#pragma once
#include "h264parserenums.h"
#include "h264definitions.h"
#include "SPSdefinitions.h"

#include <inttypes.h>

#ifdef __cplusplus
namespace libwamediastreams
{
#endif // __cplusplus

typedef struct
{
  CExpGolombCode first_mb_in_slice;    // 2 bits ue(v)
  CExpGolombCode slice_type;           // 2 ue(v)
  CExpGolombCode pic_parameter_set_id; // 2 ue(v)
  //if( separate_colour_plane_flag = = 1 )
  CExpGolombCode colour_plane_id;      // 2 u(2)

  uint8_t nNumberOfFrameNumBits;       // = sps->log2_max_frame_num_minus4 + 4
  uint8_t frame_num;                   // 2 u(v)
/*)
if( !frame_mbs_only_flag ) {
field_pic_flag
2
u(1)
if( field_pic_flag )
bottom_field_flag
2
u(1)
}
if( IdrPicFlag )
idr_pic_id
2
ue(v)
if( pic_order_cnt_type = = 0 ) {
pic_order_cnt_lsb
2
u(v)
if( bottom_field_pic_order_in_frame_present_flag && !field_pic_flag )
delta_pic_order_cnt_bottom
2
se(v)
}
if( pic_order_cnt_type = = 1 && !delta_pic_order_always_zero_flag ) {
delta_pic_order_cnt[ 0 ]
2
se(v)
if( bottom_field_pic_order_in_frame_present_flag && !field_pic_flag )
delta_pic_order_cnt[ 1 ]
2
se(v)
}
if( redundant_pic_cnt_present_flag )
redundant_pic_cnt
2
ue(v)
if( slice_type = = B )
direct_spatial_mv_pred_flag
2
u(1)
if(slice_type == P || slice_type == SP || slice_type == B){
num_ref_idx_active_override_flag
2
u(1)
if( num_ref_idx_active_override_flag ) {
num_ref_idx_l0_active_minus1
2
ue(v)
if( slice_type = = B )
num_ref_idx_l1_active_minus1
2
ue(v)
}
}
if( nal_unit_type = = 20 )
ref_pic_list_mvc_modification( ) // specified in Annex H
2
else
ref_pic_list_modification( )
2
if( ( weighted_pred_flag && ( slice_type = = P | | slice_type = = SP ) ) | | ( weighted_bipred_idc = = 1 && slice_type = = B ) )
pred_weight_table( )
2
if( nal_ref_idc != 0 )
© ISO/IEC 2012 – All rights reserved 49
ISO/IEC 14496-10:2012(E)
dec_ref_pic_marking( )
2
if( entropy_coding_mode_flag && slice_type != I && slice_type != SI )
cabac_init_idc
2
ue(v)
slice_qp_delta
2
se(v)
if(slice_type == SP || slice_type == SI){
if( slice_type = = SP )
sp_for_switch_flag
2
u(1)
slice_qs_delta
2
se(v)
}
if( deblocking_filter_control_present_flag ) {
disable_deblocking_filter_idc
2
ue(v)
if( disable_deblocking_filter_idc != 1 ) {
slice_alpha_c0_offset_div2
2
se(v)
slice_beta_offset_div2
2
se(v)
}
}
if( num_slice_groups_minus1 > 0 &&
slice_group_map_type >= 3 && slice_group_map_type <= 5)
slice_group_change_cycle
2
u(v)
}
*/
} SLICE_HEADER;

typedef struct
{
  SLICE_HEADER sliceHeader;
} SLICE;

typedef struct
{
  NAL_UNIT_START NALUnitStart;
  SLICE slice;
} PICTURE;

//
// Derived classes in which expGolomb codes have been extracted and evaluated
// Use case scenario: presentation/debug printing
//
typedef struct
{
  uint32_t nElementOffsets[NUMBER_OF_PICTURE_SYNTAX_ELEMENTS];

  NAL_UNIT_START_LAYOUT nal_unit_start;

  uint8_t first_mb_in_slice; // 2 bits ue(v)
  uint8_t slice_type;        // 2 ue(v)
  uint8_t pic_parameter_set_id;      // 2 ue(v)
  //if( separate_colour_plane_flag = = 1 )
  uint8_t colour_plane_id;   // 2 u(2)

  uint8_t nNumberOfFrameNumBits; // sps: log2_max_frame_num_minus4 + 4
  uint8_t frame_num;         // 2 u(v)
} SLICE_HEADER_LAYOUT;

typedef struct
{
  SLICE_HEADER_LAYOUT sliceHeaderLayout;
} SLICE_LAYOUT;

typedef struct
{
  SLICE_LAYOUT sliceLayout;
} PICTURE_FRAME_LAYOUT;

#ifdef __cplusplus
}; // namespace libwamediastreams
#endif //__cplusplus
