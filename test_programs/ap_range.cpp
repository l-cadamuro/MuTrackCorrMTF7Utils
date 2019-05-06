#define AP_INT_MAX_W 2048
#include "ap_int.h"
#include <stdio.h>

// c++ -lm -o ap_range ap_range.cpp -I /home/madorsky/software/xilinx/Vivado/2018.3/include/

int main()
{
    // uint32_t n = 0xf000ffff; // 32 bits filled
    // ap_int<32> apn = n; // convert to apint

    ap_int<2048> apn = 0x000000000000000000000000000602a07c02858007000000000031340d182aa8007000000000060640e7c2e000070000000000415403842d280070000000000707e102006800080000000000302e02ec2e18009000000000060b20c00051000b00000000006088025c2a8800b000000000040480a24063000c000000000030820f9003e0010000000000041560bb007a00130000000000501c14e002e001900000000003028153c2c7802a0000000000602603f028e809b000000000060241498082809f000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001d607000401d6070004;

    // // check the lowest 16 bits
    // WRONG!
    // uint32_t n_low_to_high = apn.range(0,16).to_uint();
    // uint32_t n_high_to_low = apn.range(16,0).to_uint();
    // GOOD - notice the nbits-1 range
    uint32_t n_low_to_high = apn.range(0,15).to_uint();
    uint32_t n_high_to_low = apn.range(15,0).to_uint();

    // check the lowest 32 bits
    // uint32_t n_low_to_high = apn.range(0,31).to_uint();
    // uint32_t n_high_to_low = apn.range(31,0).to_uint();


    // printf("original    = %8x\n", n);
    printf("low to high = %8x\n", n_low_to_high); // bad
    printf("high to low = %8x\n", n_high_to_low); // good

}
