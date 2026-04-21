#include<stdio.h>
#include<stdint.h>
typedef struct {
    uint8_t YawStabilityIndex_0:1;
    uint8_t BrkBstrVac_P_Actl:7;
    uint8_t YawStabilityIndex_1;
    uint8_t HsaTrnAout_Tq_Rq_0;
    uint8_t HsaTrnAout_Tq_Rq_1;
    uint8_t BrkHold_D_Stat:3;
    uint8_t BrkRearPCorrltn_B_Stat:1;
    uint8_t BrkTotTqRqDrv_No_Cnt:4;
    uint8_t BrkTot_Tq_RqDrv_0;
    uint8_t HsaStat_D_Display:3;
    uint8_t BrkTot_Tq_RqDrv_1:5;
    uint8_t BrkTotTqRqDrv_No_Cs;
} Encoded_Abs_BrkBst_Data_T;

typedef struct 
{
    uint16_t YawStabilityIndex;
    uint8_t BrkBstrVac_P_Actl;
    uint16_t HsaTrnAout_Tq_Rq;
    uint8_t BrkHold_D_Stat;
    uint8_t BrkRearPCorrltn_B_Stat;
    uint8_t BrkTotTqRqDrv_No_Cnt;
    uint16_t BrkTot_Tq_RqDrv;
    uint8_t HsaStat_D_Display;
    uint8_t BrkTotTqRqDrv_No_Cs;	
} Decoded_Abs_BrkBst_Data_T;


typedef union {
    Encoded_Abs_BrkBst_Data_T str;
    uint8_t payload[8];
} Message_4b0_T;

int main()
{
 Decoded_Abs_BrkBst_Data_T var_Decoded_Abs_BrkBst_Data;
 Message_4b0_T var_Encoded_Abs_BrkBst_Data = {.payload ={0xEF,0x00,0x80, 0x00,0x80, 0x0,0x08,0xf6}};
 
 var_Decoded_Abs_BrkBst_Data.YawStabilityIndex = (var_Encoded_Abs_BrkBst_Data.str.YawStabilityIndex_0 <<8) | (var_Encoded_Abs_BrkBst_Data.str.YawStabilityIndex_1);
 var_Decoded_Abs_BrkBst_Data.BrkBstrVac_P_Actl =  var_Encoded_Abs_BrkBst_Data.str.BrkBstrVac_P_Actl;
 var_Decoded_Abs_BrkBst_Data.HsaTrnAout_Tq_Rq = (var_Encoded_Abs_BrkBst_Data.str.HsaTrnAout_Tq_Rq_0 <<8 )| (var_Encoded_Abs_BrkBst_Data.str.HsaTrnAout_Tq_Rq_1);
 var_Decoded_Abs_BrkBst_Data.BrkHold_D_Stat = var_Encoded_Abs_BrkBst_Data.str.BrkHold_D_Stat;
 var_Decoded_Abs_BrkBst_Data.BrkRearPCorrltn_B_Stat = var_Encoded_Abs_BrkBst_Data.str.BrkHold_D_Stat;
 var_Decoded_Abs_BrkBst_Data.BrkTotTqRqDrv_No_Cnt= var_Encoded_Abs_BrkBst_Data.str.BrkTotTqRqDrv_No_Cnt;
 var_Decoded_Abs_BrkBst_Data.BrkTot_Tq_RqDrv=(var_Encoded_Abs_BrkBst_Data.str.BrkTot_Tq_RqDrv_0<<5)|(var_Encoded_Abs_BrkBst_Data.str.BrkTot_Tq_RqDrv_1);
 
 return 0;
 
}
