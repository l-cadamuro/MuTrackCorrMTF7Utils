#ifndef PATTERN_GENERATION_H
#define PATTERN_GENERATION_H

#include "dataformat.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <stdexcept>

#define START_BIT 1980 
#define RESET_BIT 1981

void SetBit (uint32_t& number, int bitpos, bool val)
{
    if (val)
       number |= (1 << bitpos);
    else
       number &= ~(1 << bitpos);
}

void set_start_reset_bits(ap_uint<2048>& mem_word)
{
  mem_word.range(START_BIT,START_BIT) = 1; // this is the start bit
  mem_word.range(RESET_BIT,RESET_BIT) = 0; // this is the reset bit
}

// same as above, but write direcly in the input buffer for the word number iword
// (1 word = 64 chunks of 32 bits each, that gets sent to the board)
void set_start_reset_bits_tobuffer (uint32_t *write_buf, size_t iword)
{
  size_t tot_offset = 64*iword;
  size_t nchunk_start = START_BIT/32;
  size_t nchunk_reset = RESET_BIT/32;
  size_t ibit_start   = START_BIT%32;
  size_t ibit_reset   = RESET_BIT%32;

  SetBit(write_buf[tot_offset + nchunk_start], ibit_start, true);
  SetBit(write_buf[tot_offset + nchunk_reset], ibit_reset, false);

  // printf("tot_offset = %i, nchunk_start = %i, ibit_start = %i, the_val = %i\n",
  //   tot_offset, nchunk_start, ibit_start, write_buf[tot_offset + nchunk_start]);

}

void wpatt_simple_idx (uint32_t *write_buf, uint32_t mem_size)
{
  for (size_t i = 0; i < mem_size/4; i++)
  {
    write_buf[i] = i;
  }
}

void wpatt_null (uint32_t *write_buf, uint32_t mem_size)
{
  for (size_t i = 0; i < mem_size/4; i++)
  {
    write_buf[i] = 0;
  }
}

void wpatt_allones (uint32_t *write_buf, uint32_t mem_size)
{
  for (size_t i = 0; i < mem_size/4; i++)
  {
    write_buf[i] = 0xffffffff;
  }
}

void wpatt_idx_every_2048 (uint32_t *write_buf, uint32_t mem_size)
{
  size_t wmax = (mem_size/4)/64; // one writing every 2048 bit word (64 chunks)
  for (size_t ibx = 0; ibx < wmax; ibx++)
  {
    ap_uint<2048> mem_word = ibx;

    // logic  [15:0] in_info_V;
    // assign {ap_rst, ap_start, in_info_V} = inject_data;
    // do I need to setup these ?
    // if (ibx == 0){
      // mem_word.range(16,16) = 1; // this is the start bit
      // mem_word.range(17,17) = 0; // this is the reset bit
    // }

    // mem_word.range(1980,1980) = 1; // this is the start bit
    // mem_word.range(1981,1981) = 0; // this is the reset bit
    set_start_reset_bits(mem_word);

    // copy the 2048 parts into the buffer
    unsigned int offset = ibx*64;
    for (uint ipart = 0; ipart < 64; ++ipart)
    {
        write_buf[ipart + offset] = mem_word((ipart+1)*32-1, ipart*32);
    }
  }
}


void wpatt_debugdata_correlator (uint32_t *write_buf, uint32_t mem_size, bool verbose=false)
{
    // crudely copied from the correlator testbench code
    // A.M is using mem_size/4, so let's use the same as the place to fill up
     // it takes 64 addresses to build a bx
     // so further scale down this max by 64
    const int nbx = (mem_size/4)/64;

    for (unsigned int ibx = 0; ibx < nbx; ++ibx)
    {
        ap_uint<trk_w_size*n_trk + mu_w_size*n_mu> input_word;

        // define muons
        for (uint imu = 0; imu < n_mu; ++imu)
        {
            ap_uint<mu_w_size> my_mu = 0xf00*ibx + imu;
            input_word.range((imu+1)*mu_w_size-1, imu*mu_w_size) = my_mu;
            // ap_uint<mu_w_size> xcheck = input_word.range((imu+1)*mu_w_size-1, imu*mu_w_size);
            // printf(">> Muon idx %i --> %i (vs %i)\n", imu, my_mu.to_int(), xcheck.to_int());
        }

        const int goffset = mu_w_size*n_mu;
        
        // define tracks
        for (uint itrk = 0; itrk < n_trk; ++itrk)
        {
            ap_uint<trk_w_size> my_trk = 0xf00*ibx + itrk;
            input_word.range((itrk+1)*trk_w_size-1 + goffset, itrk*trk_w_size + goffset) = my_trk;
            // ap_uint<trk_w_size> xcheck = input_word.range((itrk+1)*trk_w_size-1 + goffset, itrk*trk_w_size + goffset);
            // printf(">> Track idx %i --> %i (vs %i)\n", itrk, my_trk.to_int(), xcheck.to_int());
        }

        // convert this into a 2048 bit word
        ap_uint<2048> mem_word;
        mem_word.range(trk_w_size*n_trk + mu_w_size*n_mu-1, 0) = input_word;

        // chunkify and move to the memory buffer
        unsigned int offset = ibx*64;
        for (uint ipart = 0; ipart < 64; ++ipart)
        {
            write_buf[ipart + offset] = mem_word((ipart+1)*32-1, ipart*32);
        }
    }
}


// reads a line of 64 chunks of 32 bits from a file and converts them to the write buffer
// each chunk must be in hex format
// Assuming that the word (1 line in the txt file) is written as: [chunk63] [chunk62] [chunk61] ... [chunk1] [chunk0], then
//
// reverse = false: ==> the word is written to the buffer in the same way you can "read" it from the txt file
// buffer [0] = [chunk0]
// buffer [1] = [chunk1]
// ...
// buffer [63] = [chunk63]
//
// reverse = true: ==> the word chunks order is inverted w.r.t the original one in the txt file
// buffer [0] = [chunk63]
// buffer [1] = [chunk62]
// ...
// buffer [63] = [chunk0]
//
// NB!! the input txt file is read from left to right, so the incoming arrival order is [chunk63] > [chunk62] > ... > [chunk1] [chunk0]
void wpatt_load_from_file (uint32_t *write_buf, uint32_t mem_size, std::string input_file_name, bool reverse=false)
{
  std::fstream input_file(input_file_name.c_str());

  // each line contains 64 buckets of 32 bits, encoded in hex format
  std::string line;
  int nlines = 0;
  while (std::getline(input_file, line))
  {
      std::istringstream iss(line);
      int nwords = 0;
      uint32_t wtemp;
      uint32_t words[64];

      while (iss >> std::hex >> wtemp)
      {
        // buffer size is limited to 64 -> if this is already exceeded, throw error message
        if (nwords >= 64){
          std::ostringstream strnlines;
          strnlines << nlines;
          throw std::runtime_error(std::string("Error in parsing the patter input file at line: ") + strnlines.str());
        }

        if (reverse)
          words[nwords] = wtemp;
        else
          words[63-nwords] = wtemp;
        
        ++nwords;
      }
      std::cout << "........ wpatt_load_from_file : from line " << nlines << " read " << nwords << " words" << std::endl;
      
      // now copy the read line to the write buffer
      unsigned int offset = nlines*64;
      for (size_t i = 0; i < 64; ++i)
        write_buf[i + offset] = words[i];

      // and set the start and reset bits for this word
      set_start_reset_bits_tobuffer (write_buf, nlines);

      // update the line count
      ++nlines;
  }

  std::cout << "... wpatt_load_from_file : read " << nlines << " input patterns" << std::endl;
}


/*
void wpatt_debugdata_correlator (uint32_t *write_buf, uint32_t mem_size, bool verbose=false)
{
  // NOTE: %08x -> print 8 digits in hex form
  const unsigned int mu_w_size   = 40;
  const unsigned int trk_w_size  = 100;
  const unsigned int tkmu_w_size = 100;
  const unsigned int n_mu        = 12;
  const unsigned int n_trk       = 15;
  const unsigned int trkoffset   = mu_w_size*n_mu;

  // A.M is using mem_size/4, so let's use the same as the place to fill up
  // it takes 64 addresses to build a bx
  // so further scale down this max by 64
  const int maxbx = (mem_size/4)/64;

    // loop on the first 20 events - NOTE: each one is built of 64 values
    // for (int i = 0; i < 20*64; i+=64)
    for (int ibx = 0; ibx < maxbx; ++ibx)
    {
        //// build a full in and out word into an ap_uint
        ap_uint<2048> in_word;
        // ap_uint<2048> out_word;
        for (uint ipart = 0; ipart < 64; ++ipart)
        {
          // if (ipart == 0)
          //   printf("WORD %i - part 0 - val in %i, val out %i\n", i/64, write_buf[i], read_buf[i+latency]);
          uint64_t mem_addr = ibx*64+ipart; // get to the chunk with the 64 bits offset, and add offset from the specific piece

          // if (ipart < 2)
          //   printf("WORD @ bx %i - part %i - val in %i, val out %i\n", ibx/64, ipart, write_buf[mem_addr], read_buf[mem_addr+latency]);

          in_word.range((ipart+1)*32-1, (ipart)*32) = write_buf[mem_addr];
          // out_word.range((ipart+1)*32-1, (ipart)*32) = read_buf[mem_addr+latency];
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
        // ap_uint<tkmu_w_size> tkmu1 = out_word.range(tkmu_w_size-1, 0);
        // ap_uint<tkmu_w_size> tkmu2 = out_word.range(tkmu_w_size*2-1, tkmu_w_size);

    }

    if (verbose) // just print the first 20 in case
    {
        printf("Printing the values filled in the first 20 bx of the inptu buffer (20 x 64 addresses)\n");
        for (int ibx = 0; ibx < maxbx; ++ibx)
        {
            ap_uint<2048> in_word = write_buf[ibx];
            ap_uint<trk_w_size>  trk1  = in_word.range(trk_w_size-1 + mu_w_size*n_mu, mu_w_size*n_mu);
            ap_uint<trk_w_size>  trk2  = in_word.range(trk_w_size*2-1 + mu_w_size*n_mu, trk_w_size + mu_w_size*n_mu);
            ap_uint<mu_w_size>   mu1   = in_word.range(mu_w_size-1, 0);;
            ap_uint<mu_w_size>   mu2   = in_word.range(mu_w_size*2-1, mu_w_size);;

            // printf ("WORD %i: mu1: %08x mu2: %08x trk1: %08x trk2: %08x tkmu1: %08x tkmu2: %08x \n",
            // printf ("BX %i: mu1: %i mu2: %i trk1: %i trk2: %i tkmu1: %i tkmu2: %i \n",
            //   ibx,
            //   mu1.to_uint(),
            //   mu2.to_uint(),
            //   trk1.to_uint(),
            //   trk2.to_uint(),
            //   tkmu1.to_uint(),
            //   tkmu2.to_uint()
            // );
            printf ("BX %i: mu1: %i mu2: %i trk1: %i trk2: %i \n",
              ibx,
              mu1.to_uint(),
              mu2.to_uint(),
              trk1.to_uint(),
              trk2.to_uint()
            );
        }
    }
}
*/

#endif