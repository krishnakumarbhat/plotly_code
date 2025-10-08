#ifndef __PACK_UDP_LOGGING_H__
#define __PACK_UDP_LOGGING_H__

#include "../../../CommonFiles/inc/ByteStream.h"

int PackUdpLogging(void* pStream,
								  unsigned int data_size,
								  unsigned char source_info,
								  unsigned char stream_number,
								  unsigned short RefIndex,
								  unsigned char ver,
								  ARSTREAMS& Queue);
#endif // __PACK_UDP_LOGGING_H__