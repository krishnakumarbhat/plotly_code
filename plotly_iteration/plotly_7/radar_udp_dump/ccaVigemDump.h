
#ifndef _CCA_VIGEM_DUMP_H_
#define  _CCA_VIGEM_DUMP_H_

#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <stdio.h>
#include "stdafx.h"
#include "ccaVigemDump.h"
#include <stdlib.h>
#include <fstream>
#include <iomanip>
#include <iterator>
#if defined(_WIN_PLATFORM)
#include <windows.h>
#elif defined(__GNUC__)
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif
#define MAX_CAN_EXT_ID (uint32_t)(0x01FFFFFFF)
#define MAX_CAN_BYTES (64)
#include "../cca_vigem_inc/vgm_cca.h"
#include "../../CommonFiles/helper/compute.h"
#include "../../CommonFiles/inc/radar_shared.h"
#include "../../CommonFiles/CommonHeaders/Radar_Config.h"
#include "../../CommonFiles/udp_headers/rr_cal_log.h"
#include "../../CommonFiles/udp_headers/Udp_record.h"

int csvFileCreate(std::string sFileName);
int csvFileClose();
int handleCCAEthMessage(uint32_t index, CcaMessage *msg, CcaTime timestamp, uint32_t busid);
int handleCCACanMessage(uint32_t index, CcaMessage *msg, CcaTime timestamp, uint32_t busid);
int printAptivUDPheader();
int printCANdataBytes();
extern uint64_t  start_timestamp_100ns;
extern uint64_t  start_timestamp_ns;
















#endif // #ifndef _CCA_VIGEM_DUMP_H_