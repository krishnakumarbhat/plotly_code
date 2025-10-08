// srr3_dump_options.h
//
#pragma once

#include "../../../CommonFiles/inc/SRR3_MUDP_API.h"

void print_udp_header(FILE* fptr, UDPRecord_Header* p_udp_hdr, unsigned long pcTime);
void print_udp_header_All(FILE* fptr, UDPRecord_Header* p_udp_hdr, udp_custom_frame_header_t* p_cust_hdr, signed64_T pcTime);
void print_gen5_udp_header_ALL(FILE* fptr, GEN5_UDPRecord_Header_T* p_gen5_udp_hdr, signed64_T pcTime);
void print_gen7_udp_header_ALL(FILE* fptr, GEN7_UDPRecord_Header_T* p_gen7_udp_hdr, signed64_T pcTime);