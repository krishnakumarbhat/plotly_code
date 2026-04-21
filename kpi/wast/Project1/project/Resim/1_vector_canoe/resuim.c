#include <stdint.h>
#include <memory.h>


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
} Abs_BrkBst_Data_T;

typedef union {
    Abs_BrkBst_Data_T str;
    uint8_t payload[8];
} Message_4b0_T;

int main() {
    Message_4b0_T message;
    uint8_t arr[8] = {
        0xED, 0x00, 0x80, 0x00, 0xE0, 0x00, 0x18, 0xEE
    };
    memcpy(&message, &arr, 8);
}
