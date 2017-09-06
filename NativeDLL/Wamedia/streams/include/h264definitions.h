#pragma once
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string>
using namespace std;

#ifdef __cplusplus
namespace libwamediastreams
{
#endif // __cplusplus

#define NAL_UNIT_START_MAX_BYTES                (4)
#define EMULATION_PREVENTION_TEST_BYTES         (3)
#define H264_NAL_UNIT_START_CODE                (0x00000001)
#define H264_PIXELS_PER_MACROBLOCK              (16)
#define SLICE_IDENTIFICATION_REQUIRED_BYTES     (3)
	
typedef enum
{
  NAL_UNIT_TYPE_UNSPECIFIED = 0,
  NAL_UNIT_TYPE_CODED_SLICE_OF_NON_IDR_PICTURE,
  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A,
  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B,
  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C,
  NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE,
  NAL_UNIT_TYPE_SUPPLEMENTARY_ENHANCED_INFO,
  NAL_UNIT_TYPE_SEQUENCE_PARAMETER_SET,
  NAL_UNIT_TYPE_PICTURE_PARAMETER_SET,
  NAL_UNIT_TYPE_ACCESS_UNIT_DELIMITER,
  NAL_UNIT_TYPE_END_OF_SEQUENCE,
  NAL_UNIT_TYPE_END_OF_STREAM,
  NAL_UNIT_TYPE_FILLER_DATA,
  NAL_UNIT_TYPE_SEQUENCE_PARAMETER_SET_EXTENSION,
  NAL_UNIT_TYPE_PREFIX_NAL_UNIT,
  NAL_UNIT_TYPE_SUBSET_SEQUENCE_PARAMETER_SET,
  NAL_UNIT_TYPE_RESERVED_MIN = 16,
  NAL_UNIT_TYPE_RESERVED_MAX = 18,
  NAL_UNIT_TYPE_CODED_SLICE_OF_AUX_CODED_PICTURE_WITHOUT_PARTITIONING,
  NAL_UNIT_TYPE_CODED_SLICE_EXTENSION,
  NAL_UNIT_TYPE_RESERVED_1_MIN = 21,
  NAL_UNIT_TYPE_RESERVED_1_MAX = 23,
  NAL_UNIT_TYPE_UNSPECIFIED_MIN = 24,
  NAL_UNIT_TYPE_UNSPECIFIED_MAX = 31,
} eNAL_UNIT_TYPE;

typedef enum
{
  H264_SLICE_TYPE_P = 0, //	  P-slice. Consists of P-macroblocks (each macro block is predicted using one reference frame) and / or I-macroblocks.
  H264_SLICE_TYPE_B,     // 1 B-slice. Consists of B-macroblocks (each macroblock is predicted using one or two reference frames) and / or I-macroblocks.
  H264_SLICE_TYPE_I,     // 2 I-slice. Contains only I-macroblocks. Each macroblock is predicted from previously coded blocks of the same slice.
  H264_SLICE_TYPE_SP,    // 3 SP-slice. Consists of P and / or I-macroblocks and lets you switch between encoded streams.
  H264_SLICE_TYPE_SI,	 // 4 SI-slice. It consists of a special type of SI-macroblocks and lets you switch between encoded streams.
  H264_SLICE_TYPE_P_1,   // 5 P-slice.
  H264_SLICE_TYPE_B_1,   // 6 B-slice.
  H264_SLICE_TYPE_I_1,   // 7 I-slice.
  H264_SLICE_TYPE_SP_1,  // 8 SP-slice.
  H264_SLICE_TYPE_SI_1,  // 9 SI-slice.
} eH264_SLICE_TYPE;

/* 

Abridged excerpts from ISO/IEC 14496-10 document:

....

9. Parsing process
=================================================================

Inputs to this process are bits from the RBSP.
Outputs of this process are syntax element values.
This process is invoked when the descriptor of a syntax element in 
the syntax tables in subclause 7.3 is equal to 
ue(v), me(v), se(v), te(v) (see subclause 9.1), 
ce(v) (see subclause 9.2), or 
ae(v) (see subclause 9.3).


9.1 Parsing process for Exp-Golomb codes

ce(v) (see subclause 9.2), or 
ae(v) (see subclause 9.3).

SUBCLAUSE 9.1:
==================================================================
The parsing process for these syntax elements begins with reading 
the bits starting at the current location in the bitstream up to 
and including the first non-zero bit, and counting the number of 
leading bits that are equal to 0. 

This process shall be equivalent to the following:

leadingZeroBits = -1;
for( b = 0; !b; leadingZeroBits++ )
    b = read_bits( 1 )
The variable codeNum is then assigned as follows:
           leadingZeroBits
codeNum = 2                ñ 1 + read_bits( leadingZeroBits )
where the value returned from read_bits( leadingZeroBits ) is 
interpreted as a binary representation of an unsigned integer 
with most significant bit written first.

Table 9-1 illustrates the structure of the Exp-Golomb code by 
separating the bit string into ìprefixî and ìsuffixî bits....

Bit string form 	Range of codeNum
          1			0
        0 1 x0 			1-2
      0 0 1 x1 x0 		3-6
    0 0 0 1 x2 x1 x0 		7-14
  0 0 0 0 1 x3 x2 x1 x0 	15-30
0 0 0 0 0 1 x4 x3 x2 x1 x0 	31-62

or, more explicitly:

Bit string 	      codeNum
              1 	0
          0 1 0 	1
          0 1 1 	2
      0 0 1 0 0 	3
      0 0 1 0 1 	4
      0 0 1 1 0 	5
      0 0 1 1 1 	6
  0 0 0 1 0 0 0 	7
  0 0 0 1 0 0 1 	8
  0 0 0 1 0 1 0 	9
  etc

If the syntax element is coded as ue(v), the value of the 
syntax element is equal to codeNum.
- Otherwise, if the syntax element is coded as se(v), the 
value of the syntax element is derived by invoking the 
mapping process for signed Exp-Golomb codes as specified 
in subclause 9.1.1 with codeNum as the input.

codeNum syntax element value
	0 		0
	1 		1
	2		-1
	3 		2
	4 		-2
	5 		3
	6 		-3
	....
		    k+1
    k 	(-1)   * Ceil( k / 2 )

- Otherwise, if the syntax element is coded as me(v), the value 
of the syntax element is derived by invoking the mapping process 
for coded block pattern as specified in subclause 9.1.2 with 
codeNum as the input.

Table 9-4 - 

Assignment of codeNum to values of coded_block_pattern for macroblock 
prediction modes:

_______________________________
codeNum     coded_block_pattern
            -------------------
 			Intra_4x4  |  Inter
-------------------------------
	0 			47 			 0
	1 			31 	 		16
	2 			15 	 		 1
	3 			 0 	 		 2
	4 			23 	 		 4
	5 			27 	 		 8
	6 			29 	 		32
	7 			30 	 		 3
	8 			7 	 		 5
	9 			11 	 		10
	10 			13 	 		12
	11 			14 	 		15
	12 			39 	 		47
	13 			43	 		 7
	14 			45 	 		11
	15 			46 	 		13
	16 			16 	 		14
	17 			 3	 		 6
	18 			 5	 		 9
	19 			10	 		31
	20 			12	 		35
	21 			19	 		37
	22 			21	 		42
	23 			26	 		44
	24 			28	 		33
	25 			35	 		34
	26 			37	 		36
	27 			42	 		40
	28 			44	 		39
	29 			 1	 		43
	30 			 2	 		45
	31 			 4	 		46
	32 			 8	 		17
	33 			17	 		18
	34 			18	 		20
	35 			20	 		24
	36 			24	 		19
	37 			 6	 		21
	38 			 9	 		26
	39 			22	 		28
	40 			25	 		23
	41 			32	 		27
	42 			33	 		29
	43 			34	 		30
	44 			36	 		22
	45 			40	 		25
	46 			38	 		38
	47 			41	 		41

Otherwise (the syntax element is coded as te(v)), the range 
of the syntax element shall be determined first. The range 
of this syntax element may be between 0 and x, with x being 
greater than or equal to 1 and is used in the derivation of 
the value of a syntax element as follows
	- If x is greater than 1, codeNum and the value of the 
	  syntax element shall be derived in the same way as for 
	  syntax elements coded as ue(v)
	- Otherwise (x is equal to 1), the parsing process for 
	  codeNum which is equal to the value of the syntax element 
	  is given by a process equivalent to:
		b = read_bits( 1 )
		codeNum = !b
*/

typedef enum 
{
	EXP_GOLOMB_INTERPRETATION_MODE_UE_OF_V = 0,
	EXP_GOLOMB_INTERPRETATION_MODE_ME_OF_V,
	EXP_GOLOMB_INTERPRETATION_MODE_SE_OF_V,
	EXP_GOLOMB_INTERPRETATION_MODE_TE_OF_V,
	EXP_GOLOMB_INTERPRETATION_MODE_CE_OF_V,
	EXP_GOLOMB_INTERPRETATION_MODE_AE_OF_V
} eEXP_GOLOMB_INTERPRETATION_MODES;

class CExpGolombCode
{
public:
    CExpGolombCode()
      : m_mode(EXP_GOLOMB_INTERPRETATION_MODE_UE_OF_V)
      , m_nValue(0)
    {};

	eEXP_GOLOMB_INTERPRETATION_MODES m_mode;
	string   m_strBits;
	uint32_t m_nValue;
};

#pragma pack(1)

typedef struct 
{
  union
  {
    uint32_t nNALUnitStart;     // can be 00 00 00 01 (Annex B) or 4 bytes of size
    uint8_t  chNALUnitStart[NAL_UNIT_START_MAX_BYTES];
  } delimiter;

  uint8_t    NALUnitType   : 5;
  uint8_t    NALRefIDC     : 2;
  uint8_t    bForbiddenBit : 1; 
} NAL_UNIT_START;

typedef struct tagSEQUENCE_PARAMETER_SET_START
{
	NAL_UNIT_START  NALUnitStart;

	uint8_t	profile_idc;

    uint8_t  reserved_zero_2bits : 2;
    uint8_t  constraint_set5_flag: 1;
    uint8_t  constraint_set4_flag: 1;
    uint8_t  constraint_set3_flag: 1;
    uint8_t  constraint_set2_flag: 1;
    uint8_t  constraint_set1_flag: 1;
    uint8_t  constraint_set0_flag: 1;

	uint8_t	level_idc;
} SEQUENCE_PARAMETER_SET_START, *PSEQUENCE_PARAMETER_SET_START;

#pragma pack()

/*
hrd_parameters( ) 
{ 
	cpb_cnt_minus1 0 ue(v)
	bit_rate_scale 0 u(4)
	cpb_size_scale 0 u(4)
	for( SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; SchedSelIdx++ ) {
		bit_rate_value_minus1[ SchedSelIdx ] 0 ue(v)
		cpb_size_value_minus1[ SchedSelIdx ] 0 ue(v)
		cbr_flag[ SchedSelIdx ] 0 u(1)
	}
	initial_cpb_removal_delay_length_minus1 0 u(5)
	cpb_removal_delay_length_minus1 0 u(5)
	dpb_output_delay_length_minus1 0 u(5)
	time_offset_length 0 u(5)
}
*/

typedef struct tagSCHED_SEL
{
  CExpGolombCode bit_rate_value_minus1;
  CExpGolombCode cpb_size_value_minus1;
  int8_t         cbr_flag;
} SCHED_SEL, *PSCHED_SEL;

typedef struct tagHRD_PARAMETERS
{
  CExpGolombCode	cpb_cnt_minus1;
	
  uint8_t	bit_rate_scale;	// 4 bits
  uint8_t	cpb_size_scale; // 4 bits

  SCHED_SEL*		pSchedSel;		// must be dynamically allocated 
									// to contain cpb_cnt_minus1 elements
  uint8_t	initial_cpb_removal_delay_length_minus1; // 5 bits
  uint8_t	cpb_removal_delay_length_minus1; // 5 bits
  uint8_t	dpb_output_delay_length_minus1; // 5 bits
  uint8_t	time_offset_length; // 5 bits
} HRD_PARAMETERS, *PHRD_PARAMETERS;
/*
vui_parameters( ) 
{ 
	aspect_ratio_info_present_flag 0 u(1)
	if( aspect_ratio_info_present_flag ) {
		aspect_ratio_idc 0 u(8)
		if( aspect_ratio_idc = = Extended_SAR ) {
			sar_width 0 u(16)
			sar_height 0 u(16)
		}
	}
	overscan_info_present_flag 0 u(1)
	if( overscan_info_present_flag )
		overscan_appropriate_flag 0 u(1)
	video_signal_type_present_flag 0 u(1)
	if( video_signal_type_present_flag ) {
		video_format 0 u(3)
		video_full_range_flag 0 u(1)
		colour_description_present_flag 0 u(1)
		if( colour_description_present_flag ) {
			colour_primaries 0 u(8)
			transfer_int8_tacteristics 0 u(8)
			matrix_coefficients 0 u(8)
		}
	}
	chroma_loc_info_present_flag 0 u(1)
	if( chroma_loc_info_present_flag ) {
		chroma_sample_loc_type_top_field 0 ue(v)
		chroma_sample_loc_type_bottom_field 0 ue(v)
	}
	timing_info_present_flag 0 u(1)
	if( timing_info_present_flag ) {
		num_units_in_tick 0 u(32)
		time_scale 0 u(32)
		fixed_frame_rate_flag 0 u(1)
	}
	nal_hrd_parameters_present_flag 0 u(1)
	if( nal_hrd_parameters_present_flag )
		hrd_parameters( )
	vcl_hrd_parameters_present_flag 0 u(1)
	if( vcl_hrd_parameters_present_flag )
		hrd_parameters( )
	if( nal_hrd_parameters_present_flag | | vcl_hrd_parameters_present_flag )
		low_delay_hrd_flag 0 u(1)
	pic_struct_present_flag 0 u(1)
	bitstream_restriction_flag 0 u(1)
	if( bitstream_restriction_flag ) {
		motion_vectors_over_pic_boundaries_flag 0 u(1)
		max_bytes_per_pic_denom 0 ue(v)
		max_bits_per_mb_denom 0 ue(v)
		log2_max_mv_length_horizontal 0 ue(v)
		log2_max_mv_length_vertical 0 ue(v)
		num_reorder_frames 0 ue(v)
		max_dec_frame_buffering 0 ue(v)
	}
}
*/



#define EXTENDED_SAR	(255)

typedef struct tagVUI_PARAMETERS
{
  uint8_t	aspect_ratio_info_present_flag; // 1 bit

//	if( aspect_ratio_info_present_flag ) {

  uint8_t	aspect_ratio_idc;				// 8 bits
	// if( aspect_ratio_idc = = Extended_SAR ) {
  uint16_t	sar_width;
  uint16_t	sar_height;
	//    }
	// }

  uint8_t	overscan_info_present_flag;		// 1 bit
  // if( overscan_info_present_flag )
  uint8_t	overscan_appropriate_flag;		// 1 bit

  uint8_t	video_signal_type_present_flag;	// 1 bit
  // if( video_signal_type_present_flag ) {
  uint8_t	video_format;					// 3 bits
  uint8_t	video_full_range_flag;			// 1 bits
  uint8_t	colour_description_present_flag;// 1 bit
  // if( colour_description_present_flag ) {
  uint8_t	colour_primaries;				// 8 bits
  uint8_t	transfer_characteristics;		// 8 bits
  uint8_t	matrix_coefficients;			// 8 bits
  uint8_t	chroma_loc_info_present_flag;	// 1 bit
  // if( chroma_loc_info_present_flag ) {
  CExpGolombCode	chroma_sample_loc_type_top_field;
  CExpGolombCode	chroma_sample_loc_type_bottom_field;

  uint8_t	timing_info_present_flag;		// 1 bit
  // if( timing_info_present_flag ) {
  uint32_t	num_units_in_tick;				// 32 bits
  uint32_t	time_scale;						// 32 bits
  uint8_t	fixed_frame_rate_flag;			// 1 bit

  uint8_t	nal_hrd_parameters_present_flag;// 1 bit
  //if( nal_hrd_parameters_present_flag )
  HRD_PARAMETERS	nal_hrd_parameters;

  uint8_t	vcl_hrd_parameters_present_flag;// 1 bit
  // if( vcl_hrd_parameters_present_flag )
  HRD_PARAMETERS	vcl_hrd_parameters;			
  // if( nal_hrd_parameters_present_flag | | vcl_hrd_parameters_present_flag )
  uint8_t	low_delay_hrd_flag;				// 1 bit
  uint8_t	pic_struct_present_flag;		// 1 bit
  uint8_t	bitstream_restriction_flag;		// 1 bit
  //if( bitstream_restriction_flag ) {
  uint8_t	motion_vectors_over_pic_boundaries_flag; // 1 bit
  CExpGolombCode max_bytes_per_pic_denom;
  CExpGolombCode max_bits_per_mb_denom;
  CExpGolombCode log2_max_mv_length_horizontal;
  CExpGolombCode log2_max_mv_length_vertical;
  CExpGolombCode num_reorder_frames;
  CExpGolombCode max_dec_frame_buffering;
} VUI_PARAMETERS, *PVUI_PARAMETERS;

typedef struct tagSCALING_MATRIX_ELEMENT
{
  uint8_t seq_scaling_list_present_flag; // 1 bit
  // if( seq_scaling_list_present_flag[i] )
  // if(i < 6)
  //	scaling_list( ScalingList4x4[i], 16, UseDefaultScalingMatrix4x4Flag[i])	0
  // else
  //	scaling_list( ScalingList8x8[i- 6], 64, UseDefaultScalingMatrix8x8Flag[i - 6] )	
} SCALING_MATRIX_ELEMENT, *PSCALING_MATRIX_ELEMENT;

#define NUMBER_OF_SCALING_MATRIX_ELEMENTS	(8)

typedef struct tagSCALING_MATRIX
{
  SCALING_MATRIX_ELEMENT	matrixElements[NUMBER_OF_SCALING_MATRIX_ELEMENTS];
} SCALING_MATRIX, *PSCALING_MATRIX;

typedef struct tagPROFILE_SPECIFIC_ADDITION
{
  CExpGolombCode	chroma_format_idc;
  // if (chroma_format_idc == 3)
  // {
  uint8_t	residual_colour_transform_flag;// 1 bit
  // }

  CExpGolombCode	bit_depth_luma_minus8;
  CExpGolombCode	bit_depth_chroma_minus8;
  uint8_t	qpprime_y_zero_transform_bypass_flag; // 1 bit
  uint8_t	seq_scaling_matrix_present_flag; // 1 bit
  // if( seq_scaling_matrix_present_flag )
  // {
  SCALING_MATRIX	scalingMatrix;
  // }
} PROFILE_SPECIFIC_ADDITION, *PPROFILE_SPECIFIC_ADDITION;

/*
seq_parameter_set_rbsp( ) { C Descriptor
	profile_idc 0 u(8)
	constraint_set0_flag 0 u(1)
	constraint_set1_flag 0 u(1)
	constraint_set2_flag 0 u(1)
	constraint_set3_flag 0 u(1)
	reserved_zero_4bits  0 u(4) // equal to 0 
	level_idc 0 u(8)
	seq_parameter_set_id 0 ue(v)
	if( profile_idc = = 100 || profile_idc = = 110 || profile_idc = = 122 | | profile_idc = = 144 ) 
	{
		chroma_format_idc 0 ue(v)
		if( chroma_format_idc = = 3 )
			residual_colour_transform_flag 0 u(1)
		bit_depth_luma_minus8 0 ue(v)
		bit_depth_chroma_minus8 0 ue(v)
		qpprime_y_zero_transform_bypass_flag 0 u(1)
		seq_scaling_matrix_present_flag 0 u(1)
		if( seq_scaling_matrix_present_flag )
			for( i = 0; i < 8; i++ ) {
				seq_scaling_list_present_flag[ i ] 0 u(1)
				if( seq_scaling_list_present_flag[i] )
					if(i < 6)
						scaling_list( ScalingList4x4[i], 16, UseDefaultScalingMatrix4x4Flag[i])	0
					else
						scaling_list( ScalingList8x8[i- 6], 64, UseDefaultScalingMatrix8x8Flag[i - 6] )	0
			}
	}

	log2_max_frame_num_minus4 0 ue(v)
	pic_order_cnt_type 0 ue(v)
	if( pic_order_cnt_type = = 0 )
		log2_max_pic_order_cnt_lsb_minus4 0 ue(v)
	else if( pic_order_cnt_type = = 1 ) 
	{
		delta_pic_order_always_zero_flag 0 u(1)
		offset_for_non_ref_pic 0 se(v)
		offset_for_top_to_bottom_field 0 se(v)
		num_ref_frames_in_pic_order_cnt_cycle 0 ue(v)
		for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
			offset_for_ref_frame[ i ] 0 se(v)
	}
	num_ref_frames 0 ue(v)
	gaps_in_frame_num_value_allowed_flag 0 u(1)
	pic_width_in_mbs_minus1 0 ue(v)
	pic_height_in_map_units_minus1 0 ue(v)
	frame_mbs_only_flag 0 u(1)
	if( !frame_mbs_only_flag )
		mb_adaptive_frame_field_flag 0 u(1)
	direct_8x8_inference_flag 0 u(1)
	frame_cropping_flag 0 u(1)
	if( frame_cropping_flag ) {
		frame_crop_left_offset 0 ue(v)
		frame_crop_right_offset 0 ue(v)
		frame_crop_top_offset 0 ue(v)
		frame_crop_bottom_offset 0 ue(v)
	}
	vui_parameters_present_flag 0 u(1)
	if( vui_parameters_present_flag )
		vui_parameters( ) 0
	rbsp_trailing_bits( ) 0
}
*/

#define MIN_RBSP_TRAILING_BITS  (2)

typedef struct tagRBSP_TRAILING_BITS
{
  uint8_t  rbsp_stop_one_bit;          // 1 bit, mandatory value = 1
  uint32_t nNumberOfAlignmentZeroBits;
  uint8_t* rbsp_alignment_zero_bits;   // variable number of them
} RBSP_TRAILING_BITS, *PRBSP_TRAILING_BITS;

typedef struct tagSEQUENCE_PARAMETER_SET
{
  SEQUENCE_PARAMETER_SET_START	start;

  CExpGolombCode				seq_parameter_set_id;
  PROFILE_SPECIFIC_ADDITION		profile_specific_addition;
  CExpGolombCode				log2_max_frame_num_minus4;
  CExpGolombCode				pic_order_cnt_type;
  // if(pic_order_cnt_type == 0)
  CExpGolombCode				log2_max_pic_order_cnt_lsb_minus4;
  // else {
  uint8_t					    delta_pic_order_always_zero_flag;	// 1 bit
  CExpGolombCode				offset_for_non_ref_pic;				// ************ SIGNED se(v)
  CExpGolombCode				offset_for_top_to_bottom_field;	    // ************ SIGNED se(v)
  CExpGolombCode				num_ref_frames_in_pic_order_cnt_cycle;
  // for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
  //		offset_for_ref_frame[ i ] 0 se(v)
  CExpGolombCode*				p_offset_for_ref_frame;				// array holding SIGNNED se(v)
  // }

  CExpGolombCode				num_ref_frames;
  uint8_t					    gaps_in_frame_num_value_allowed_flag; // 1 bit;
  CExpGolombCode				pic_width_in_mbs_minus1;
  CExpGolombCode				pic_height_in_map_units_minus1;
  uint8_t					    frame_mbs_only_flag; // 1 bit;
  // if(0 == frame_mbs_only_flag )
  uint8_t					    mb_adaptive_frame_field_flag; // 1 bit
  uint8_t					    direct_8x8_inference_flag; // 1 bit
  uint8_t					    frame_cropping_flag; // 1 bit	

  // if( frame_cropping_flag ) {
  CExpGolombCode				frame_crop_left_offset;
  CExpGolombCode				frame_crop_right_offset;
  CExpGolombCode				frame_crop_top_offset;
  CExpGolombCode				frame_crop_bottom_offset;
  // }

  uint8_t					    vui_parameters_present_flag; // 1 bit
  // if( vui_parameters_present_flag )
  VUI_PARAMETERS				vui_parameters;
  RBSP_TRAILING_BITS			rbsp_trailing_bits;
} SEQUENCE_PARAMETER_SET;

#ifdef __cplusplus
}; // namespace libwamediastreams
#endif //__cplusplus

