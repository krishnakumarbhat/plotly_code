#include "../../../CommonFiles/inc/SRR3_MUDP_API.h"
#include "hil_embedded_utility.h"
#include "PackUDPLogging.h"
#include <algorithm>
#include <functional>
using namespace std;

#include "../../../CommonFiles/inc/fixmac.h"


const int MAX_STREAM_SIZE						= 100000; //100k

#define UDP_RECORD_VERSION (unsigned8_T)0xA1
//  UDPRecord_Header's Version Info, which is a combination of the version and size of this UDP record header.
#define UDP_RECORD_VERSIONINFO (uint16_t)(UDP_RECORD_VERSION << 8 | sizeof(UDPRecord_Header))

//static UDP_FRAME_STRUCTURE_T m_nFrame_st;
static unsigned char s_mudp_frame[MUDP_PACKET_TX_SZ] = {0};
#ifndef min_size
#define min(a,b) (((a)<(b))? (a) : (b))
#endif

static unsigned short  B2L_s(unsigned short arg) {
	unsigned short ret = 0;
	unsigned short i = 0;

	char* dst = (char*)(&ret);
	const char* src = (const char*)(&arg + 1);

	for (; i < sizeof(unsigned short); i++)
		*dst++ = *--src;

	return ret;
};
static unsigned long  B2L_l(unsigned long arg) {
	unsigned long ret = 0;
	unsigned short i = 0;

	char* dst = (char*)(&ret);
	const char* src = (const char*)(&arg + 1);

	for (; i < sizeof(unsigned long); i++)
		*dst++ = *--src;

	return ret;
};

/*****************************************************************************
*. Name: Calculate_CRC32
*. This function calculates CRC32 - method defined by AUTOSAR
*.
*. Parameters: Data(uint8) and Length(uint32)
*.
*. Return Value: unsigned32_T.
*.
*. Shared Variables: None.
*.
*. SCR Information:
*. 26354
******************************************************************************/
static unsigned32_T Calculate_CRC32(unsigned8_T* dataptr, unsigned32_T length)
{
   /* Initial Value */
   unsigned32_T CrcValue_u32 = CRC_XOR_VALUE;
   while (length--)
   {
      CrcValue_u32 ^= 0xFFuL & (unsigned32_T)(*(dataptr++));
      CrcValue_u32 = (CrcValue_u32 >> 8 )^ (Crc_Table[(unsigned8_T)(CrcValue_u32)]);
   }
   return CRC_XOR_VALUE ^ CrcValue_u32;
}

int PackUdpLogging(void* pStream,
								  unsigned int data_size,
								  unsigned char source_info,
								  unsigned char stream_number,
								  unsigned short RefIndex,
								  unsigned char ver,
								  ARSTREAMS& Queue)
{
		static unsigned short m_sourceTxCnt = 0;
		int idx = 0;
		unsigned int crc = 0;
		unsigned int data_index = 0;
		unsigned int chunk_size = UDP_MAX_LOGGING_DATA_SIZE;

		unsigned char total_chunk = ((data_size / chunk_size) ? ((data_size / chunk_size) + 1) : (data_size / chunk_size));
        int offset = 0;

        UDPRecord_Header* p_frame_header = (UDPRecord_Header*)(&s_mudp_frame[offset]);
        offset += sizeof(UDPRecord_Header);

#if ENABLE_CUSTHDR
        udp_custom_frame_header_t* p_cust_frame_header = (udp_custom_frame_header_t*)(&s_mudp_frame[offset]); /* Customer defined frame header */
        offset += sizeof(udp_custom_frame_header_t);
#endif
        unsigned char* pData = (unsigned char*)(&s_mudp_frame[offset]);
        offset += 1424;

        unsigned int* pCrc = (unsigned int*)(&s_mudp_frame[offset]);

        offset = 0; //reset
        
		do
		{
			memset((void*)s_mudp_frame, 0x00, sizeof(UDP_FRAME_STRUCTURE_T));
            
            chunk_size = min(data_size-offset,1424);

            //UDP HEADER
			p_frame_header->versionInfo = B2L_s(UDP_RECORD_VERSIONINFO);
			p_frame_header->sourceTxCnt = B2L_s(++m_sourceTxCnt);
			p_frame_header->sourceTxTime = B2L_l(time_t());
			p_frame_header->Platform = source_info;
            p_frame_header->streamRefIndex = B2L_l(RefIndex);
			p_frame_header->streamDataLen = B2L_s(chunk_size);
			p_frame_header->streamTxCnt = (unsigned char)idx;
			p_frame_header->streamNumber = stream_number;
			p_frame_header->streamVersion = ver;
			p_frame_header->streamChunks = total_chunk;
			p_frame_header->streamChunkIdx = (unsigned char)idx;

#if ENABLE_CUSTHDR
            //AUDI CUSTOM HEADER
			p_cust_frame_header->packetnumber = B2L_l(idx);
			p_cust_frame_header->utc_time = B2L_s(time_t());
			p_cust_frame_header->timestamp = B2L_s(time_t());
			p_cust_frame_header->sensorid = B2L_s(source_info);
			p_cust_frame_header->sensorstatus = 0;
			p_cust_frame_header->detectioncnt = 0;
			p_cust_frame_header->Cal_Running_Cnt = 0;
			p_cust_frame_header->Total_Cal_Chunk_Cnt = 0; 
			p_cust_frame_header->UDP_Version_info.MajorRevision = 0;
			p_cust_frame_header->UDP_Version_info.MinorRevision = 0;
			//p_cust_frame_header->middle_of_dwell_time = 0;
			// update data into the frame.
#endif//ENABLE_CUSTHDR

            //extract the input stream to chunk
			memcpy(pData,((char*)pStream+offset), min(data_size,chunk_size));
			offset += chunk_size;

			*pCrc = B2L_l(Calculate_CRC32((unsigned char*)&s_mudp_frame[0], sizeof(UDP_FRAME_STRUCTURE_T)));

			Queue.push_back(new CByteStream((unsigned char*)s_mudp_frame,MUDP_PACKET_TX_SZ));

            ++idx;

        } while (offset < data_size); // verified.
	
	return 0;
}