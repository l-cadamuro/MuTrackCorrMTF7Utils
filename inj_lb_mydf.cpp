#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#define AP_INT_MAX_W 2048
#include "ap_int.h"

#include "pattern_generation.h" // utilities to fill the write buffer
#include "dataformat.h" // utilities to fill the write buffer

// c++ -lm -o inj_lb_mydf inj_lb_mydf.cpp -I /home/madorsky/software/xilinx/Vivado/2018.3/include/

// demo program for MTF7 skeleton firmware design

// compile command: 
// g++ -o inj_lb inj_lb.cpp

// give device name as parameter, like this:
// ./inj_lb /dev/utca_sp121

void debug_print_buffer(uint32_t* buf, int n, int latency, int step = 1)
{
  for (size_t i = latency; i < step*(n+latency); i+=step)
    printf("Idx %i -> %08x\n", i, buf[i]);
}

int main(int argc, char* argv[])
{
        
  // open device like a file
  int fd = ::open(argv[1], O_RDWR);
  if (fd < 0)
	{
	  printf("ERROR: Cannot open MTF7 device file: %s\n", argv[1]);
	  exit (-1);
	}
  else
	{
	  printf("opened device: %s\n", argv[1]);
      
	}
  
  // read core firmware time stamp
  uint32_t saddr = 0x000B60C0; // see excel address spreadsheet for addresses
  uint64_t value;

  pread(fd, &value, 8, saddr); // read timestamp register

  // decode time stamp
  uint32_t core_fw_version = value & 0xffffffff;
  uint32_t second = core_fw_version & 0x3f;
  uint32_t minute = (core_fw_version >> 6)  & 0x3f;
  uint32_t hour   = (core_fw_version >> 12) & 0x1f;
  uint32_t year   = ((core_fw_version >> 17) & 0x3f) + 2000;
  uint32_t month  = (core_fw_version >> 23) & 0xf;
  uint32_t day    = (core_fw_version >> 27) & 0x1f;
  printf("Firmware date: %04d-%02d-%02d time: %02d:%02d:%02d\n",
		 year, month, day, hour, minute, second);

  // injection memory address and parameters
  uint32_t mem_size  = 0x40000; //size of injection memory, bytes, from excel file
  uint32_t xfer_size = 0x800; //one transfer size
  uint32_t n_writes  = (mem_size / xfer_size); // how many transfers for entire buffer
  uint32_t mem_base  = 0xc0000; // base address of injection memory, from excel file
  
  // test buffers
  uint32_t write_buf[mem_size/4];
  uint32_t read_buf[mem_size/4];

  // fill test buffer with data
  // feel free to change test pattern 
  /*
   for (size_t i = 0; i < mem_size/4; i++)
	{
	  write_buf[i] = i;
	}
  */

  // wpatt_debugdata_correlator (write_buf, mem_size, true);
  // wpatt_simple_idx (write_buf, mem_size);
  // wpatt_null (write_buf, mem_size);
  // wpatt_idx_every_2048 (write_buf, mem_size);
  wpatt_load_from_file (write_buf, mem_size, "patterns/test_patterns.txt");
  // wpatt_allones(write_buf, mem_size);

  /*
  // check the content of the write buffer
  int check_bx = 0;
  for (uint ipart = 0; ipart < 64; ++ipart)
  {
    printf("BX %i, part %i -> %i\n", check_bx, ipart, write_buf[ipart + check_bx*64]);
  }

  printf("====================================\n");
  check_bx = 1;
  for (uint ipart = 0; ipart < 64; ++ipart)
  {
    printf("BX %i, part %i -> %i\n", check_bx, ipart, write_buf[ipart + check_bx*64]);
  }

  printf("====================================\n");
  check_bx = 2;
  for (uint ipart = 0; ipart < 64; ++ipart)
  {
    printf("BX %i, part %i -> %i\n", check_bx, ipart, write_buf[ipart + check_bx*64]);
  }
  
  printf("====================================\n");
  check_bx = 10;
  for (uint ipart = 0; ipart < 64; ++ipart)
  {
    printf("BX %i, part %i -> %i\n", check_bx, ipart, write_buf[ipart + check_bx*64]);
  }
  */

  /*
  // check the content of the write buffer - val , start, rst - only check part 0
  int check_bx = 0;
  printf("BX %2i, part %2i -> val = %2i, start = %2i, rst = %2i\n", check_bx, 0,
        (write_buf[0+check_bx*64] & 0xffff),    // first 16 bits
        (write_buf[0+check_bx*64] & 0x10000) >> 16, // the 16th bit only 
        (write_buf[0+check_bx*64] & 0x20000) >> 17 // the 17th bit only 
    );

  check_bx = 1;
  printf("BX %2i, part %2i -> val = %2i, start = %2i, rst = %2i\n", check_bx, 0,
        (write_buf[0+check_bx*64] & 0xffff),    // first 16 bits
        (write_buf[0+check_bx*64] & 0x10000) >> 16, // the 16th bit only 
        (write_buf[0+check_bx*64] & 0x20000) >> 17 // the 17th bit only 
    );

  check_bx = 2;
  printf("BX %2i, part %2i -> val = %2i, start = %2i, rst = %2i\n", check_bx, 0,
        (write_buf[0+check_bx*64] & 0xffff),    // first 16 bits
        (write_buf[0+check_bx*64] & 0x10000) >> 16, // the 16th bit only 
        (write_buf[0+check_bx*64] & 0x20000) >> 17 // the 17th bit only 
    );

  check_bx = 10;
  printf("BX %2i, part %2i -> val = %2i, start = %2i, rst = %2i\n", check_bx, 0,
        (write_buf[0+check_bx*64] & 0xffff),    // first 16 bits
        (write_buf[0+check_bx*64] & 0x10000) >> 16, // the 16th bit only 
        (write_buf[0+check_bx*64] & 0x20000) >> 17 // the 17th bit only 
    );
  */

  //write blocks of data

  for (uint32_t j = 0; j < n_writes; j++)
	{
	  pwrite(fd, 
			 &(write_buf[j * xfer_size/4]), 
			 xfer_size, 
			 mem_base + j * xfer_size);
	}

  // send inject command

  value = 2; // see mask for inject_test_data in excel file
  pwrite (fd, &value, 8, 0x000B6000); // inject_test_data register address from excel file
  value = 0; // remove inject command bit
  pwrite (fd, &value, 8, 0x000B6000);

  //read back data that came via inversion loopback in firmware

  for (uint32_t j = 0; j < n_writes; j++)
	{
	  pread(fd, 
			&(read_buf[j * xfer_size/4]), 
			xfer_size, 
			mem_base + j * xfer_size);
	}


  /*
  // scan the output buffer -> antyhing != 0 ?
  printf("Scanning buffer\n");
  int nfound = 0;
  for (size_t ib = 0; ib < mem_size/4; ++ib)
  {
    if (read_buf[ib] != 0){
      // printf("ib %i >> val = %i\n", ib, read_buf[ib]);
      ++nfound;
    }
  }
  printf("Number != 0 : %i\n", nfound);
  */

  printf("First chunks - INPUT\n");
  debug_print_buffer(write_buf, 64, 0, 1);

  //////////////////////////////////////////////////////////////
  // debug of the print function
  // ap_uint<68> val = 0xa0000000000000000;
  ap_uint<132> val = 0xffffeeeedddd1230;
  val.range(79,64) = 0xabcd;
  val.range(131,128) = 0xd;
  // ap_uint<16> val = 0xaaaa;
  std::string s = ap_uint_to_string<132> (val);
  printf("----> my value is %s\n", s.c_str());
  printf("%04x %04x %04x %04x %04x \n",
    val.range(131, 128).to_uint64(),
    val.range(127, 96).to_uint64(),
    val.range(95, 64).to_uint64(),
    val.range(63, 32).to_uint64(),
    val.range(31, 0).to_uint64()
  );
  ///////////////////////////////////////////////////////////////

  // printf("PROVA : %08x\n", write_buf[3] );

  // printf("First chunks - OUTPUT\n");
  // debug_print_buffer(read_buf, 64, 0, 1);


  // NOTE: the inherent latency of the memory is 2 clock cycles
  // (each clock writes 2048 bits, i.e. 64 addresses of my uint32_t buffer)
  // so the minimum below is 2*64 + (firmware_latency_in_clk)*64
  uint32_t latency = 2*64; // memory latency = 2 clocks by 64 32-bit words
  // uint32_t latency = 64*(2+4);

  // // check a few tracks
  // // shift read data by latency so it matches write data
  // for (int i = 0; i < 20*64; i+=64)
  // {
  //   printf ("i: %08x w: %08x r: %08x\n", i, write_buf[i], read_buf[i+latency]);  
  // }
  

  /*
  // print a few words, inverting read word to cancel firmware inversion
  // shift read data by latency so it matches write data
  // NOTE: %08x -> print 8 digits in hex form ==> 8*16 = 128 values
  const unsigned int mu_w_size   = 40;
  const unsigned int trk_w_size  = 100;
  const unsigned int tkmu_w_size = 100;
  const unsigned int n_mu        = 12;
  const unsigned int n_trk       = 15;
  const unsigned int trkoffset   = mu_w_size*n_mu;

  // loop on the first 20 events - NOTE: each one is built of 64 values
  // for (int i = 0; i < 20*64; i+=64)
  for (int ibx = 0; ibx < 20; ++ibx)
  {
    //// build a full in and out word into an ap_uint
    ap_uint<2048> in_word;
    ap_uint<2048> out_word;
    for (uint ipart = 0; ipart < 64; ++ipart)
    {
      // if (ipart == 0)
      //   printf("WORD %i - part 0 - val in %i, val out %i\n", i/64, write_buf[i], read_buf[i+latency]);
      uint64_t mem_addr = ibx*64+ipart; // get to the chunk with the 64 bits offset, and add offset from the specific piece

      // if (ipart < 2)
      //   printf("WORD @ bx %i - part %i - val in %i, val out %i\n", ibx/64, ipart, write_buf[mem_addr], read_buf[mem_addr+latency]);

      in_word.range((ipart+1)*32-1, (ipart)*32) = write_buf[mem_addr];
      out_word.range((ipart+1)*32-1, (ipart)*32) = read_buf[mem_addr+latency];
    }

    // NOTE about printf:
    // llx indicates to print a long long int (i.e., 64 bits)
    // 4 bits -> 1 hex digit, so that 64 bits -> 16 hex digits)
    // printf("Read word at bx %i : IN = %16llx,      OUT = %16llx\n", ibx/64, in_word.to_uint64(), out_word.to_uint64());

    // now extract the subsets i want
    ap_uint<trk_w_size>  trk1  = in_word.range(trk_w_size-1 + mu_w_size*n_mu, mu_w_size*n_mu);
    ap_uint<trk_w_size>  trk2  = in_word.range(trk_w_size*2-1 + mu_w_size*n_mu, trk_w_size + mu_w_size*n_mu);
    ap_uint<mu_w_size>   mu1   = in_word.range(mu_w_size-1, 0);;
    ap_uint<mu_w_size>   mu2   = in_word.range(mu_w_size*2-1, mu_w_size);;
    ap_uint<tkmu_w_size> tkmu1 = out_word.range(tkmu_w_size-1, 0);
    ap_uint<tkmu_w_size> tkmu2 = out_word.range(tkmu_w_size*2-1, tkmu_w_size);

    // printf ("WORD %i: mu1: %08x mu2: %08x trk1: %08x trk2: %08x tkmu1: %08x tkmu2: %08x \n",
    printf ("BX %i: mu1: %i mu2: %i trk1: %i trk2: %i tkmu1: %i tkmu2: %i \n",
      ibx,
      mu1.to_uint(),
      mu2.to_uint(),
      trk1.to_uint(),
      trk2.to_uint(),
      tkmu1.to_uint(),
      tkmu2.to_uint()
    );
  }
  */


  // now print the first 20 bx
  // for (size_t ibx = 0; ibx < mem_size/4/64; ++ibx)
  // for (size_t ibx = 0; ibx < 20; ++ibx)
  for (size_t ibx = 0; ibx < 1; ++ibx)
  {
    ap_uint<2048> in_word;
    ap_uint<2048> out_word;

    ap_uint<trk_w_size>  trk1  ;
    ap_uint<trk_w_size>  trk2  ;
    
    ap_uint<mu_w_size>   mu1   ;
    ap_uint<mu_w_size>   mu2   ;
    
    ap_uint<tkmu_w_size> tkmu1 ;
    ap_uint<tkmu_w_size> tkmu2 ;
    
    std::vector<ap_uint<trk_w_size> > all_tracks;
    std::vector<ap_uint<mu_w_size> >  all_muons ;
    std::vector<ap_uint<tkmu_w_size> >  all_tkmus ;

    readout_input_buf  (write_buf, ibx, in_word, trk1, trk2, mu1, mu2, all_tracks, all_muons);
    readout_output_buf (read_buf,  ibx, latency, out_word, tkmu1, tkmu2, all_tkmus);

    // printf ("BX %2i: mu1: %13x mu2: %13x trk1: %13x trk2: %13x tkmu1: %13x tkmu2: %13x (outw = %llx) \n",
    //   ibx,
    //   mu1.to_uint(),
    //   mu2.to_uint(),
    //   trk1.to_uint(),
    //   trk2.to_uint(),
    //   tkmu1.to_uint(),
    //   tkmu2.to_uint(),
    //   out_word.to_uint64() // will trim down to the 64 lsb, but OK for debugging
    // );

    // printf ("BX %2i: in = %2i, start = %2i, rst = %2i ===== out = %2i, ready = %2i, done = %2i, out_valid = %2i\n",
    //   ibx,
    //   in_word.range(15,0).to_uint64(),
    //   in_word.range(16,16).to_uint64(),
    //   in_word.range(17,17).to_uint64(),
    //   out_word.range(15,0).to_uint64(),
    //   out_word.range(18,18).to_uint64(),
    //   out_word.range(17,17).to_uint64(),
    //   out_word.range(16,16).to_uint64()
    // );

    // printf ("BX %2i: in = %2i, start = %2i, rst = %2i ===== out = %2i, idle = %2i, done = %2i, ready = %2i, out_valid = %2i\n",
    //   ibx,
    //   in_word.range(63,0).to_uint64(),
    //   in_word.range(1980,1980).to_uint64(),
    //   in_word.range(1981,1981).to_uint64(),
    //   out_word.range(63,0).to_uint64(),
    //   out_word.range(1983, 1983).to_uint64(), // idle
    //   out_word.range(1982, 1982).to_uint64(), // ready
    //   out_word.range(1981, 1981).to_uint64(), // done
    //   out_word.range(1980, 1980).to_uint64()  // out_valid
    // );

    // also check the properties of these tracks
    // printf (" >>>> BX %2i: mu1_pt = %2i, mu1_eta = %2i, mu1_phi = %2i, mu2_pt = %2i, mu2_eta = %2i, mu2_phi = %2i, trk1_pt = %2i, trk1_eta = %2i, trk1_phi = %2i, trk1_pt = %2i, trk1_eta = %2i, trk1_phi = %2i \n",
    //   ibx,
    //   get_mu_pt(all_muons.at(0)).to_int(),
    //   get_mu_theta(all_muons.at(0)).to_int(),
    //   get_mu_phi(all_muons.at(0)).to_int(),
    //   get_mu_pt(all_muons.at(1)).to_int(),
    //   get_mu_theta(all_muons.at(1)).to_int(),
    //   get_mu_phi(all_muons.at(1)).to_int(),
    //   get_trk_pt(all_tracks.at(0)).to_int(),
    //   get_trk_theta(all_tracks.at(0)).to_int(),
    //   get_trk_phi(all_tracks.at(0)).to_int(),
    //   get_trk_pt(all_tracks.at(1)).to_int(),
    //   get_trk_theta(all_tracks.at(1)).to_int(),
    //   get_trk_phi(all_tracks.at(1)).to_int()
    // );

    printf("Number of muons unpacked : %i\n", all_muons.size());
    for (uint imu = 0; imu < all_muons.size(); ++imu)
    {
      ap_uint<mu_w_size> the_mu = all_muons.at(imu);
      // printf(" >> idx = %2i, pt = %10u, theta = %10u, phi = %10u . Full word = %x\n",
      printf(" >> idx = %2i, pt = %10u, theta = %10u, phi = %10u . Full word = %s\n",
          imu,
          get_mu_pt(the_mu).to_uint(),
          get_mu_theta(the_mu).to_uint(),
          get_mu_phi(the_mu).to_uint(),
          ap_uint_to_string<mu_w_size>(the_mu).c_str()
          // the_mu.to_ulong()
      );
    }

    printf("Number of tracks unpacked : %i\n", all_tracks.size());
    for (uint itrk = 0; itrk < all_tracks.size(); ++itrk)
    {
      ap_uint<trk_w_size> the_trk = all_tracks.at(itrk);
      // printf(" >> idx = %2i, pt = %10u, theta = %10u, phi = %10u . Full word = %x\n",
      printf(" >> idx = %2i, pt = %10u, theta = %10u, phi = %10u . Full word = %s\n",
          itrk,
          get_trk_pt(the_trk).to_uint(),
          get_trk_theta(the_trk).to_uint(),
          get_trk_phi(the_trk).to_uint(),
          ap_uint_to_string<trk_w_size>(the_trk).c_str()
          // the_trk.to_ulong()
      );
    }

    printf("Number of tkmu unpacked : %i\n", all_tkmus.size());
    for (uint itrk = 0; itrk < all_tkmus.size(); ++itrk)
    {
      ap_uint<tkmu_w_size> the_tkmu = all_tkmus.at(itrk);
      // printf(" >> idx = %2i, pt = %10u, theta = %10u, phi = %10u . Full word = %x\n",
      printf(" >> idx = %2i, pt = %10u, theta = %10u, phi = %10u . Full word = %s\n",
          itrk,
          get_tkmu_pt(the_tkmu).to_uint(),
          get_tkmu_theta(the_tkmu).to_uint(),
          get_tkmu_phi(the_tkmu).to_uint(),
          ap_uint_to_string<tkmu_w_size>(the_tkmu).c_str()
          // the_tkmu.to_ulong()
      );
    }    


  }

  // printf(">>>>> Data written in\n");
  // debug_print_buffer(write_buf, 30, 0, 1);

  // printf(">>>>> Data read out\n");
  // debug_print_buffer(read_buf, 30, 0, 1);

  // printf(">>>>> Data written for mu2 (32 LSBs)\n");
  // debug_print_buffer(write_buf, 30, 100, 2048);

  // printf(">>>>> Data read out for mutrk2 (32 LSBs) - 0 latency\n");
  // debug_print_buffer(read_buf, 30, 100, 2048);

  // printf(">>>>> Data read out for mutrk2 (32 LSBs) - 64*2 latency\n");
  // debug_print_buffer(read_buf, 30, 100 + 64*2, 2048);

  // printf(">>>>> Data read out for mutrk2 (32 LSBs) - 64*3 latency\n");
  // debug_print_buffer(read_buf, 30, 100 + 64*3, 2048);

  /// some debug
  
  // read the second muon in the event:
  // bit 79:40 in the 2048 size word
  // meaning 31:0 (byte 0) + 63:32 (byte 1) + 95:64 (byte 2)
  //         --- skip        --- >> 8         mask w/ 1111111 <- but I don't care about those HSB for now


  /*
  size_t the_bx = 2;
  size_t bxoffset = 64*the_bx; // 64 x bx number 

  uint32_t mu2 = write_buf[1 + bxoffset] >> 8;
  printf("mu2 in the event, bx %i : %08x\n", the_bx, mu2);

  // read the second tkmu in the event:
  // bit 199:100 in the 2048 size word
  // meaning 31:0 (byte 0) + 63:32 (byte 1) + 95:64 (byte 2) + 127:96 (byte 3)
  //         --- skip        --- skip         --- skip         --- >> 4


  uint32_t tkmu2_l0 = read_buf[3 + bxoffset] >> 4;
  uint32_t tkmu2_l1 = read_buf[3 + bxoffset + 64*1] >> 4;
  uint32_t tkmu2_l2 = read_buf[3 + bxoffset + 64*2] >> 4;
  uint32_t tkmu2_l3 = read_buf[3 + bxoffset + 64*3] >> 4;

  printf("tkmu2 in the event, bx %i , L0 : %08x\n", the_bx, tkmu2_l0);
  printf("tkmu2 in the event, bx %i , L1 : %08x\n", the_bx, tkmu2_l1);
  printf("tkmu2 in the event, bx %i , L2 : %08x\n", the_bx, tkmu2_l2);
  printf("tkmu2 in the event, bx %i , L3 : %08x\n", the_bx, tkmu2_l3);
  */

  // out_word.range((ipart+1)*32-1, (ipart)*32) = read_buf[mem_addr+latency];
  // ap_uint<tkmu_w_size> tkmu1 = out_word.range(tkmu_w_size-1, 0);
  // ap_uint<tkmu_w_size> tkmu2 = out_word.range(tkmu_w_size*2-1, tkmu_w_size);


 //  for (int i = 0; i < 20*64; i+=64)
	// printf ("ADDR %i: mu1: %08x mu2: %08x trk1: %08x trk2: %08x tkmu1: %08x tkmu2: %08x \n",
 //    i/64,
 //    write_buf[i],
 //    read_buf[i+latency]
 //  );

/*
  printf("NOW COMPARING FULL BUFFER\n");

  // compare entire buffer
  uint32_t xorr;
  int mismatch_cnt = 0;
  for (size_t i = 0; i < mem_size/4 - latency; i++)
	{
	  // invert read word to cancel firmware inversion
	  // shift read data by latency so it matches write data
	  xorr = write_buf[i] ^ (read_buf[i+latency]); 
	  if (xorr != 0)
		{
		  printf ("i: %08lx w: %08x r: %08x e: %08x\n",
					  i, write_buf[i], read_buf[i+latency], xorr);
		  mismatch_cnt++;
		  if (mismatch_cnt > 200) goto eject; // too many mismatches, get out
		}
	}

 eject:
  printf("Mismatched dwords: %d\n", mismatch_cnt);
  return 0;
*/
}
