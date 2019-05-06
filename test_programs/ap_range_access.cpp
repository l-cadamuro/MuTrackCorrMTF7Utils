#define AP_INT_MAX_W 2048
#include "ap_int.h"
#include <stdio.h>

// c++ -lm -o ap_range_access ap_range_access.cpp -I /home/madorsky/software/xilinx/Vivado/2018.3/include/

// ap_range_ref<32, true> : first number is the width of the original number, second bool is true=signed, false=unsigned

ap_range_ref<32, true> get_ref_to_low_four (ap_int<32>& val)
{
    return val.range(3,0);
}

int main()
{
    ap_int<32> apn = 0x0;
    printf("apn = %i\n", apn.to_int());
    get_ref_to_low_four (apn) = 0x2;
    // ap_range_ref<32, true> refval = get_ref_to_low_four (apn);
    // refval = 0x2;
    // ap_int<4>* theval = get_ref_to_low_four (apn);
    // *theval = 0x11;
    printf("apn = %i\n", apn.to_int());
}
