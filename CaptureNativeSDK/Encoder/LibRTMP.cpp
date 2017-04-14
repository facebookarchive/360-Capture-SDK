/****************************************************************************************************************

Filename	:	LibRTMP.cpp
Content		:	Streaming video which is encoded from HW(NVIDIA/AMD) encoder parts. 
Created		:	Feb 2, 2017
Authors		:	Homin Lee

Copyright	:

****************************************************************************************************************/
#include "LibRTMP.h"
#include <math.h>

LibRTMP::LibRTMP() {
	file_ = NULL;
}

int LibRTMP::readU8(uint32_t *u8, FILE*fp) {
	if (fread(u8, 1, 1, fp) != 1)
		return 0;
	return 1;
}

int LibRTMP::readU16(uint32_t *u16, FILE*fp) {
	if (fread(u16, 2, 1, fp) != 1)
		return 0;
	*u16 = HTON16(*u16);
	return 1;
}

int LibRTMP::readU24(uint32_t *u24, FILE*fp) {
	if (fread(u24, 3, 1, fp) != 1)
		return 0;
	*u24 = HTON24(*u24);
	return 1;
}

int LibRTMP::readU32(uint32_t *u32, FILE*fp) {
	if (fread(u32, 4, 1, fp) != 1)
		return 0;
	*u32 = HTON32(*u32);
	return 1;
}

int LibRTMP::peekU8(uint32_t *u8, FILE*fp) {
	if (fread(u8, 1, 1, fp) != 1)
		return 0;
	fseek(fp, -1, SEEK_CUR);
	return 1;
}

int LibRTMP::readTime(uint32_t *utime, FILE*fp) {
	if (fread(utime, 4, 1, fp) != 1)
		return 0;
	*utime = HTONTIME(*utime);
	return 1;
}

bool LibRTMP::initSocket()
{
#ifdef WIN32     
	WORD version;
	WSADATA wsaData;
	version = MAKEWORD(1, 1);
	return (WSAStartup(version, &wsaData) == 0);
#else     
	return TRUE;
#endif     
}

bool LibRTMP::initSockets()
{
#ifdef WIN32     
	WORD version;
	WSADATA wsaData;
	version = MAKEWORD(2, 2);
	return (WSAStartup(version, &wsaData) == 0);
#else     
	return TRUE;
#endif     
}

int LibRTMP::readBuffer(unsigned char* buf, int bufSize) {
	if (!feof(file_)) {
		int true_size = fread(buf, 1, bufSize, file_);
		return true_size;
	}
	else {
		return -1;
	}
}

bool LibRTMP::connectRTMPWithFlv(const TCHAR* url, const TCHAR* streamKey, const TCHAR* videoFile) {	
	RTMPPacket *packet = NULL;
	uint32_t start_time = 0;
	uint32_t now_time = 0;
	//the timestamp of the previous frame
	long pre_frame_time = 0;
	long lasttime = 0;
	int nextIsKey = 1;
	uint32_t preTagsize = 0;

	//packet attributes
	uint32_t type = 0;
	uint32_t datalength = 0;
	uint32_t timestamp = 0;
	uint32_t streamid = 0;

	char videoFileName[_MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0, videoFile, -1, videoFileName, _MAX_PATH, NULL, NULL);

	FILE* fp = NULL;
	fp = fopen(videoFileName, "rb");
	if (!fp) {
		RTMP_LogPrintf("Open File Error.\n");
		WSACleanup(); 
		return false;
	}

	if (!initSockets()) {
		RTMP_LogPrintf("Init Socket Err\n");
		return false;
	}

	char urlName[_MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0, url, -1, urlName, _MAX_PATH, NULL, NULL);

	char keyName[_MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0, streamKey, -1, keyName, _MAX_PATH, NULL, NULL);

	rtmp_ = RTMP_Alloc();
	RTMP_Init(rtmp_);

	//set connection timeout,default 30s
	rtmp_->Link.timeout = 5;
	if (RTMP_SetupURL2(rtmp_, (char*)urlName, (char*)keyName) == FALSE) { 	
		RTMP_Log(RTMP_LOGERROR, "SetupURL Err\n");
		RTMP_Free(rtmp_);
		WSACleanup();
		return false;
	}

	//if unable,the AMF command would be 'play' instead of 'publish'
	RTMP_EnableWrite(rtmp_);

	if (!RTMP_Connect(rtmp_, NULL)) {
		RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
		RTMP_Free(rtmp_);
		WSACleanup();
		return false;
	}

	if (!RTMP_ConnectStream(rtmp_, 0)) {
		RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
		RTMP_Close(rtmp_);
		RTMP_Free(rtmp_);
		WSACleanup();
		return false;
	}

	packet = (RTMPPacket*)malloc(sizeof(RTMPPacket));
	RTMPPacket_Alloc(packet, 1024 * 64);
	RTMPPacket_Reset(packet);

	packet->m_hasAbsTimestamp = 0;
	packet->m_nChannel = 0x04;
	packet->m_nInfoField2 = rtmp_->m_stream_id;

	RTMP_LogPrintf("Start to send data ...\n");

	//jump over FLV Header
	fseek(fp, 9, SEEK_SET);
	//jump over previousTagSizen
	fseek(fp, 4, SEEK_CUR);
	start_time = RTMP_GetTime();
	while (1)
	{
		if ((((now_time = RTMP_GetTime()) - start_time)
			<(pre_frame_time)) && nextIsKey) {
			//wait for 1 sec if the send process is too fast
			//this mechanism is not very good,need some improvement
			if (pre_frame_time>lasttime) {
				RTMP_LogPrintf("TimeStamp:%8lu ms\n", pre_frame_time);
				lasttime = pre_frame_time;
			}
			Sleep(1000);
			continue;
		}

		//not quite the same as FLV spec
		if (!readU8(&type, fp))
			break;
		if (!readU24(&datalength, fp))
			break;
		if (!readTime(&timestamp, fp))
			break;
		if (!readU24(&streamid, fp))
			break;

		if (type != 0x08 && type != 0x09) {
			//jump over non_audio and non_video frame，
			//jump over next previousTagSizen at the same time
			fseek(fp, datalength + 4, SEEK_CUR);
			continue;
		}

		if (fread(packet->m_body, 1, datalength, fp) != datalength)
			break;

		packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
		packet->m_nTimeStamp = timestamp;
		packet->m_packetType = type;
		packet->m_nBodySize = datalength;
		pre_frame_time = timestamp;

		if (!RTMP_IsConnected(rtmp_)) {
			RTMP_Log(RTMP_LOGERROR, "rtmp is not connect\n");
			break;
		}
		if (!RTMP_SendPacket(rtmp_, packet, 0)) {
			RTMP_Log(RTMP_LOGERROR, "Send Error\n");
			break;
		}

		if (!readU32(&preTagsize, fp))
			break;

		if (!peekU8(&type, fp))
			break;
		if (type == 0x09) {
			if (fseek(fp, 11, SEEK_CUR) != 0)
				break;
			if (!peekU8(&type, fp)) {
				break;
			}
			if (type == 0x17)
				nextIsKey = 1;
			else
				nextIsKey = 0;

			fseek(fp, -11, SEEK_CUR);
		}
	}

	RTMP_LogPrintf("\nSend Data Over\n");

	if (fp)
		fclose(fp);

	if (rtmp_ != NULL) {
		RTMP_Close(rtmp_);
		RTMP_Free(rtmp_);
		rtmp_ = NULL;
	}
	if (packet != NULL) {
		RTMPPacket_Free(packet);
		free(packet);
		packet = NULL;
	}

	WSACleanup(); 
	return true;

}


bool LibRTMP::connectRTMPWithH264(const TCHAR* url, const TCHAR* streamKey, const TCHAR* videoFile) {

	char videoFileName[_MAX_PATH];	
	WideCharToMultiByte(CP_UTF8, 0, videoFile, -1, videoFileName, _MAX_PATH, NULL, NULL);	
	
	char urlName[_MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0, url, -1, urlName, _MAX_PATH, NULL, NULL);

	char keyName[_MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0, streamKey, -1, keyName, _MAX_PATH, NULL, NULL);

	file_ = fopen(videoFileName, "rb");
	headPos_ = 0;
	fileBufferSize_ = BUFFER_SIZE;
	fileBuffer_ = (unsigned char*)malloc(BUFFER_SIZE);
	tmpFileBuffer_ = (unsigned char*)malloc(BUFFER_SIZE);
	initSocket();

	rtmp_ = RTMP_Alloc();
	RTMP_Init(rtmp_);

	if (RTMP_SetupURL2(rtmp_, (char*)urlName, (char*)keyName) == FALSE) {  // 
		RTMP_Free(rtmp_);
		return false;
	}

	RTMP_EnableWrite(rtmp_);

	if (RTMP_Connect(rtmp_, NULL) == FALSE)
	{
		RTMP_Free(rtmp_);
		return false;
	}

	int retTest = RTMP_IsConnected(rtmp_);

	if (RTMP_ConnectStream(rtmp_, 0) == FALSE)
	{
		RTMP_Close(rtmp_);
		RTMP_Free(rtmp_);
		return false;
	}

	int retTest2 = RTMP_IsConnected(rtmp_);

	sendH264StreamingData(&LibRTMP::readBuffer);

	return true;
}

int LibRTMP::readFirstNaluFromBuf(NaluUnit &nalu)
{
	int tailPos = headPos_;
	memset(tmpFileBuffer_, 0, BUFFER_SIZE);
	while (headPos_<fileBufferSize_)
	{
		//search for nal header
		if (fileBuffer_[headPos_++] == 0x00 &&
			fileBuffer_[headPos_++] == 0x00)
		{
			if (fileBuffer_[headPos_++] == 0x01)
				goto gotnal_head;
			else
			{
				//cuz we have done an i++ before,so we need to roll back now
				headPos_--;
				if (fileBuffer_[headPos_++] == 0x00 &&
					fileBuffer_[headPos_++] == 0x01)
					goto gotnal_head;
				else
					continue;
			}
		}
		else
			continue;

		//search for nal tail which is also the head of next nal
	gotnal_head:
		//normal case:the whole nal is in this m_pFileBuf
		tailPos = headPos_;
		while (tailPos<fileBufferSize_)
		{
			if (fileBuffer_[tailPos++] == 0x00 &&
				fileBuffer_[tailPos++] == 0x00)
			{
				if (fileBuffer_[tailPos++] == 0x01)
				{
					nalu.size = (tailPos - 3) - headPos_;
					break;
				}
				else
				{
					tailPos--;
					if (fileBuffer_[tailPos++] == 0x00 &&
						fileBuffer_[tailPos++] == 0x01)
					{
						nalu.size = (tailPos - 4) - headPos_;
						break;
					}
				}
			}
		}

		nalu.type = fileBuffer_[headPos_] & 0x1f;
		memcpy(tmpFileBuffer_, fileBuffer_ + headPos_, nalu.size);
		nalu.data = tmpFileBuffer_;
		headPos_ = tailPos;
		return true;
	}
}

int LibRTMP::readOneNaluFromBuf(NaluUnit &nalu, int(LibRTMP::*readBuffers)(uint8_t *buf, int buf_size)) {
	int tailPos = headPos_;
	int ret;
	int nalustart;
	memset(tmpFileBuffer_, 0, BUFFER_SIZE);
	nalu.size = 0;
	while (1)
	{
		if (headPos_ == NO_MORE_BUFFER_TO_READ)
			return false;
		while (tailPos<fileBufferSize_)
		{
			//search for nal tail
			if (fileBuffer_[tailPos++] == 0x00 &&
				fileBuffer_[tailPos++] == 0x00)
			{
				if (fileBuffer_[tailPos++] == 0x01)
				{
					nalustart = 3;
					goto gotnal;
				}
				else
				{
					//cuz we have done an i++ before,so we need to roll back now
					tailPos--;
					if (fileBuffer_[tailPos++] == 0x00 &&
						fileBuffer_[tailPos++] == 0x01)
					{
						nalustart = 4;
						goto gotnal;
					}
					else
						continue;
				}
			}
			else
				continue;

		gotnal:
			//  special case1:parts of the nal lies in a m_pFileBuf and we have to read from buffer
			//  again to get the rest part of this nal			
			if (headPos_ == GOT_A_NAL_CROSS_BUFFER || headPos_ == GOT_A_NAL_INCLUDE_A_BUFFER)
			{
				nalu.size = nalu.size + tailPos - nalustart;
				if (nalu.size>BUFFER_SIZE)
				{
					prevTmpFileBuffer_ = tmpFileBuffer_;	// save pointer in case realloc fails
					if ((tmpFileBuffer_ = (unsigned char*)realloc(tmpFileBuffer_, nalu.size)) == NULL)
					{
						free(prevTmpFileBuffer_);  // free original block
						return false;
					}
				}
				memcpy(tmpFileBuffer_ + nalu.size + nalustart - tailPos, fileBuffer_, tailPos - nalustart);
				nalu.data = tmpFileBuffer_;
				headPos_ = tailPos;
				return true;
			}
			//normal case:the whole nal is in this m_pFileBuf
			else
			{
				nalu.type = fileBuffer_[headPos_] & 0x1f;
				nalu.size = tailPos - headPos_ - nalustart;
				if (nalu.type == 0x06)
				{
					headPos_ = tailPos;
					continue;
				}
				memcpy(tmpFileBuffer_, fileBuffer_ + headPos_, nalu.size);
				nalu.data = tmpFileBuffer_;
				headPos_ = tailPos;
				return true;
			}
		}

		if (tailPos >= fileBufferSize_ && headPos_ != GOT_A_NAL_CROSS_BUFFER && headPos_ != GOT_A_NAL_INCLUDE_A_BUFFER)
		{
			nalu.size = BUFFER_SIZE - headPos_;
			nalu.type = fileBuffer_[headPos_] & 0x1f;
			memcpy(tmpFileBuffer_, fileBuffer_ + headPos_, nalu.size);

			if ((ret = (this->*readBuffers)(fileBuffer_, fileBufferSize_))<BUFFER_SIZE) {
				memcpy(tmpFileBuffer_ + nalu.size, fileBuffer_, ret);
				nalu.size = nalu.size + ret;
				nalu.data = tmpFileBuffer_;
				headPos_ = NO_MORE_BUFFER_TO_READ;
				return false;
			}
			tailPos = 0;
			headPos_ = GOT_A_NAL_CROSS_BUFFER;
			continue;
		}
		if (headPos_ == GOT_A_NAL_CROSS_BUFFER || headPos_ == GOT_A_NAL_INCLUDE_A_BUFFER) {
			nalu.size = BUFFER_SIZE + nalu.size;

			prevTmpFileBuffer_ = tmpFileBuffer_;  // save pointer in case realloc fails
			if ((tmpFileBuffer_ = (unsigned char*)realloc(tmpFileBuffer_, nalu.size)) == NULL)
			{
				free(prevTmpFileBuffer_);  // free original block
				return false;
			}

			memcpy(tmpFileBuffer_ + nalu.size - BUFFER_SIZE, fileBuffer_, BUFFER_SIZE);

			if ((ret = (this->*readBuffers)(fileBuffer_, fileBufferSize_))<BUFFER_SIZE) {
				memcpy(tmpFileBuffer_ + nalu.size, fileBuffer_, ret);
				nalu.size = nalu.size + ret;
				nalu.data = tmpFileBuffer_;
				headPos_ = NO_MORE_BUFFER_TO_READ;
				return false;
			}
			tailPos = 0;
			headPos_ = GOT_A_NAL_INCLUDE_A_BUFFER;
			continue;
		}
	}
	return 0;
}

bool LibRTMP::sendH264StreamingData(int(LibRTMP::*readBuffers)(unsigned char *buf, int buf_size)) {

	int ret;
	uint32_t now, last_update;

	memset(&metaData_, 0, sizeof(RTMPMetaData));
	memset(fileBuffer_, 0, BUFFER_SIZE);
	if ((ret = (this->*readBuffers)(fileBuffer_, fileBufferSize_))<0)	
	{
		return false;
	}

	NaluUnit naluUnit;

	readFirstNaluFromBuf(naluUnit);
	metaData_.nSpsLen = naluUnit.size;
	metaData_.Sps = NULL;
	metaData_.Sps = (unsigned char*)malloc(naluUnit.size);
	memcpy(metaData_.Sps, naluUnit.data, naluUnit.size);

	readOneNaluFromBuf(naluUnit, &LibRTMP::readBuffer);	
	metaData_.nPpsLen = naluUnit.size;
	metaData_.Pps = NULL;
	metaData_.Pps = (unsigned char*)malloc(naluUnit.size);
	memcpy(metaData_.Pps, naluUnit.data, naluUnit.size);

	int width = 0, height = 0, fps = 0;
	h264Decode(metaData_.Sps, metaData_.nSpsLen, width, height, fps);

	if (fps)
		metaData_.frameRate = fps;
	else
		metaData_.frameRate = 25;


	unsigned int tick = 0;
	unsigned int tick_gap = 1000 / metaData_.frameRate;
	readOneNaluFromBuf(naluUnit, &LibRTMP::readBuffer);	
	int keyframe = (naluUnit.type == 0x05) ? TRUE : FALSE;
	char buffer[50];
	while (sendH264Packet(naluUnit.data, naluUnit.size, keyframe, tick))
	{
	got_sps_pps:
		//if(naluUnit.size==8581)
		sprintf(buffer, "NALU size:%8d\n", naluUnit.size);
		OutputDebugStringA(buffer);
		//printf("NALU size:%8d\n", naluUnit.size);
		last_update = RTMP_GetTime();
		if (!readOneNaluFromBuf(naluUnit, &LibRTMP::readBuffer))		
			goto end;
		if (naluUnit.type == 0x07 || naluUnit.type == 0x08)
			goto got_sps_pps;
		keyframe = (naluUnit.type == 0x05) ? TRUE : FALSE;
		tick += tick_gap;
		now = RTMP_GetTime();
		msleep(tick_gap - now + last_update);
	}
end:
	free(metaData_.Sps);
	free(metaData_.Pps);

	return true;
}

bool LibRTMP::h264Decode(BYTE * buf, unsigned int length, int &width, int &height, int &fps) {
	UINT startBit = 0;
	fps = 0;
	deEmulationPrevention(buf, &length);

	int forbidden_zero_bit = u(1, buf, startBit);
	int nal_ref_idc = u(2, buf, startBit);
	int nal_unit_type = u(5, buf, startBit);
	if (nal_unit_type == 7)
	{
		int profile_idc = u(8, buf, startBit);
		int constraint_set0_flag = u(1, buf, startBit);  //(buf[1] & 0x80)>>7;
		int constraint_set1_flag = u(1, buf, startBit);  //(buf[1] & 0x40)>>6;
		int constraint_set2_flag = u(1, buf, startBit);  //(buf[1] & 0x20)>>5;
		int constraint_set3_flag = u(1, buf, startBit);  //(buf[1] & 0x10)>>4;
		int reserved_zero_4bits = u(4, buf, startBit);
		int level_idc = u(8, buf, startBit);

		int seq_parameter_set_id = Ue(buf, length, startBit);

		if (profile_idc == 100 || profile_idc == 110 ||
			profile_idc == 122 || profile_idc == 144)
		{
			int chroma_format_idc = Ue(buf, length, startBit);
			if (chroma_format_idc == 3)
				int residual_colour_transform_flag = u(1, buf, startBit);
			int bit_depth_luma_minus8 = Ue(buf, length, startBit);
			int bit_depth_chroma_minus8 = Ue(buf, length, startBit);
			int qpprime_y_zero_transform_bypass_flag = u(1, buf, startBit);
			int seq_scaling_matrix_present_flag = u(1, buf, startBit);

			int seq_scaling_list_present_flag[8];
			if (seq_scaling_matrix_present_flag)
			{
				for (int i = 0; i < 8; i++) {
					seq_scaling_list_present_flag[i] = u(1, buf, startBit);
				}
			}
		}
		int log2_max_frame_num_minus4 = Ue(buf, length, startBit);
		int pic_order_cnt_type = Ue(buf, length, startBit);
		if (pic_order_cnt_type == 0)
			int log2_max_pic_order_cnt_lsb_minus4 = Ue(buf, length, startBit);
		else if (pic_order_cnt_type == 1)
		{
			int delta_pic_order_always_zero_flag = u(1, buf, startBit);
			int offset_for_non_ref_pic = Se(buf, length, startBit);
			int offset_for_top_to_bottom_field = Se(buf, length, startBit);
			int num_ref_frames_in_pic_order_cnt_cycle = Ue(buf, length, startBit);

			int *offset_for_ref_frame = new int[num_ref_frames_in_pic_order_cnt_cycle];
			for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
				offset_for_ref_frame[i] = Se(buf, length, startBit);
			delete[] offset_for_ref_frame;
		}
		int num_ref_frames = Ue(buf, length, startBit);
		int gaps_in_frame_num_value_allowed_flag = u(1, buf, startBit);
		int pic_width_in_mbs_minus1 = Ue(buf, length, startBit);
		int pic_height_in_map_units_minus1 = Ue(buf, length, startBit);

		width = (pic_width_in_mbs_minus1 + 1) * 16;
		height = (pic_height_in_map_units_minus1 + 1) * 16;

		int frame_mbs_only_flag = u(1, buf, startBit);
		if (!frame_mbs_only_flag)
			int mb_adaptive_frame_field_flag = u(1, buf, startBit);

		int direct_8x8_inference_flag = u(1, buf, startBit);
		int frame_cropping_flag = u(1, buf, startBit);
		if (frame_cropping_flag)
		{
			int frame_crop_left_offset = Ue(buf, length, startBit);
			int frame_crop_right_offset = Ue(buf, length, startBit);
			int frame_crop_top_offset = Ue(buf, length, startBit);
			int frame_crop_bottom_offset = Ue(buf, length, startBit);
		}
		int vui_parameter_present_flag = u(1, buf, startBit);
		if (vui_parameter_present_flag)
		{
			int aspect_ratio_info_present_flag = u(1, buf, startBit);
			if (aspect_ratio_info_present_flag)
			{
				int aspect_ratio_idc = u(8, buf, startBit);
				if (aspect_ratio_idc == 255)
				{
					int sar_width = u(16, buf, startBit);
					int sar_height = u(16, buf, startBit);
				}
			}
			int overscan_info_present_flag = u(1, buf, startBit);
			if (overscan_info_present_flag)
				int overscan_appropriate_flagu = u(1, buf, startBit);
			int video_signal_type_present_flag = u(1, buf, startBit);
			if (video_signal_type_present_flag)
			{
				int video_format = u(3, buf, startBit);
				int video_full_range_flag = u(1, buf, startBit);
				int colour_description_present_flag = u(1, buf, startBit);
				if (colour_description_present_flag)
				{
					int colour_primaries = u(8, buf, startBit);
					int transfer_characteristics = u(8, buf, startBit);
					int matrix_coefficients = u(8, buf, startBit);
				}
			}
			int chroma_loc_info_present_flag = u(1, buf, startBit);
			if (chroma_loc_info_present_flag)
			{
				int chroma_sample_loc_type_top_field = Ue(buf, length, startBit);
				int chroma_sample_loc_type_bottom_field = Ue(buf, length, startBit);
			}
			int timing_info_present_flag = u(1, buf, startBit);
			if (timing_info_present_flag)
			{
				int num_units_in_tick = u(32, buf, startBit);
				int time_scale = u(32, buf, startBit);
				fps = time_scale / (2 * num_units_in_tick);
			}
		}
		return true;
	}
	else
		return false;
	
}

void LibRTMP::deEmulationPrevention(BYTE* buf, unsigned int* buf_size) {
	int i = 0, j = 0;
	BYTE* tmpPtr = NULL;
	unsigned int tmpBufSize = 0;
	int val = 0;

	tmpPtr = buf;
	tmpBufSize = *buf_size;
	for (i = 0; i<(tmpBufSize - 2); i++)
	{		
		val = (tmpPtr[i] ^ 0x00) + (tmpPtr[i + 1] ^ 0x00) + (tmpPtr[i + 2] ^ 0x03);  // check for 0x000003
		if (val == 0)
		{			
			for (j = i + 2; j<tmpBufSize - 1; j++)  // kick out 0x03
				tmpPtr[j] = tmpPtr[j + 1];
					
			(*buf_size)--;  //and so we should devrease bufsize
		}
	}

	return;
}

DWORD LibRTMP::u(UINT BitCount, BYTE * buf, UINT &startBit) {
	DWORD dwRet = 0;
	for (UINT i = 0; i<BitCount; i++)
	{
		dwRet <<= 1;
		if (buf[startBit / 8] & (0x80 >> (startBit % 8)))
		{
			dwRet += 1;
		}
		startBit++;
	}
	return dwRet;
}

int LibRTMP::Se(BYTE *pBuff, UINT length, UINT &startBit) {
	int UeVal = Ue(pBuff, length, startBit);
	double k = UeVal;
	int nValue = ceil(k / 2);
	if (UeVal % 2 == 0)
		nValue = -nValue;
	return nValue;
}

UINT LibRTMP::Ue(BYTE *pBuff, UINT length, UINT &startBit) {
	UINT zeroNum = 0;
	while (startBit < length * 8)
	{
		if (pBuff[startBit / 8] & (0x80 >> (startBit % 8)))
		{
			break;
		}
		zeroNum++;
		startBit++;
	}
	startBit++;

	DWORD dwRet = 0;
	for (UINT i = 0; i<zeroNum; i++)
	{
		dwRet <<= 1;
		if (pBuff[startBit / 8] & (0x80 >> (startBit % 8)))
		{
			dwRet += 1;
		}
		startBit++;
	}
	return (1 << zeroNum) - 1 + dwRet;
}

int LibRTMP::sendH264Packet(unsigned char *data, unsigned int size, int isKeyFrame, unsigned int timeStamp) {
	if (data == NULL && size<11) {
		return false;
	}

	unsigned char *body = (unsigned char*)malloc(size + 9);
	memset(body, 0, size + 9);

	int i = 0;
	if (isKeyFrame) {
		body[i++] = 0x17;// 1:Iframe  7:AVC   
		body[i++] = 0x01;// AVC NALU   
		body[i++] = 0x00;
		body[i++] = 0x00;
		body[i++] = 0x00;


		// NALU size   
		body[i++] = size >> 24 & 0xff;
		body[i++] = size >> 16 & 0xff;
		body[i++] = size >> 8 & 0xff;
		body[i++] = size & 0xff;
		// NALU data   
		memcpy(&body[i], data, size);
		sendVideoSpsPps(metaData_.Pps, metaData_.nPpsLen, metaData_.Sps, metaData_.nSpsLen);
	}
	else {
		body[i++] = 0x27;// 2:Pframe  7:AVC   
		body[i++] = 0x01;// AVC NALU   
		body[i++] = 0x00;
		body[i++] = 0x00;
		body[i++] = 0x00;


		// NALU size   
		body[i++] = size >> 24 & 0xff;
		body[i++] = size >> 16 & 0xff;
		body[i++] = size >> 8 & 0xff;
		body[i++] = size & 0xff;
		// NALU data   
		memcpy(&body[i], data, size);
	}

	int bRet = sendPacket(RTMP_PACKET_TYPE_VIDEO, body, i + size, timeStamp);

	free(body);

	return bRet;

}

int LibRTMP::sendVideoSpsPps(unsigned char *pps, int pps_len, unsigned char * sps, int sps_len)
{
	RTMPPacket * packet = NULL;//rtmp관써뭐
	unsigned char * body = NULL;
	int i;
	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + 1024);

	memset(packet, 0, RTMP_HEAD_SIZE + 1024);
	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	body = (unsigned char *)packet->m_body;
	i = 0;
	body[i++] = 0x17;
	body[i++] = 0x00;

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	//  AVCDecoderConfigurationRecord
	body[i++] = 0x01;
	body[i++] = sps[1];
	body[i++] = sps[2];
	body[i++] = sps[3];
	body[i++] = 0xff;

	body[i++] = 0xe1;
	body[i++] = (sps_len >> 8) & 0xff;
	body[i++] = sps_len & 0xff;
	memcpy(&body[i], sps, sps_len);
	i += sps_len;

	body[i++] = 0x01;
	body[i++] = (pps_len >> 8) & 0xff;
	body[i++] = (pps_len) & 0xff;
	memcpy(&body[i], pps, pps_len);
	i += pps_len;

	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nBodySize = i;
	packet->m_nChannel = 0x04;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet->m_nInfoField2 = rtmp_->m_stream_id;

	int nRet = RTMP_SendPacket(rtmp_, packet, TRUE);
	free(packet);
	return nRet;
}

int LibRTMP::sendPacket(unsigned int packetType, unsigned char *data, unsigned int size, unsigned int timeStamp) {
	RTMPPacket* packet;

	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + size);
	memset(packet, 0, RTMP_HEAD_SIZE);
	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	packet->m_nBodySize = size;
	memcpy(packet->m_body, data, size);
	packet->m_hasAbsTimestamp = 0;
	packet->m_packetType = packetType;
	packet->m_nInfoField2 = rtmp_->m_stream_id;
	packet->m_nChannel = 0x04;

	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	if (RTMP_PACKET_TYPE_AUDIO == packetType && size != 4)
	{
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	}
	packet->m_nTimeStamp = timeStamp;

	int nRet = 0;
	if (RTMP_IsConnected(rtmp_))
	{
		nRet = RTMP_SendPacket(rtmp_, packet, TRUE);
	}

	free(packet);
	return nRet;
}


void LibRTMP::close() {
	if (rtmp_)
	{
		RTMP_Close(rtmp_);
		RTMP_Free(rtmp_);
		rtmp_ = NULL;
	}

	WSACleanup();  // Clean up sockets

	if (fileBuffer_ != NULL)
	{
		free(fileBuffer_);
	}
	if (tmpFileBuffer_ != NULL)
	{
		free(tmpFileBuffer_);
	}
}