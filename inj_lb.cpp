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

// demo program for MTF7 skeleton firmware design

// compile command: 
// g++ -o inj_lb inj_lb.cpp

// give device name as parameter, like this:
// ./inj_lb /dev/utca_sp121

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
  for (size_t i = 0; i < mem_size/4; i++)
	{
	  write_buf[i] = i;
	}

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
  
  uint32_t latency = 2*64; // memory latency = 2 clocks by 64 32-bit words

  // print a few words, inverting read word to cancel firmware inversion
  // shift read data by latency so it matches write data
  for (int i = 0; i < 20*64; i+=64)
	printf ("i: %08x w: %08x r: %08x\n", i, write_buf[i], read_buf[i+latency]);

  

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
}
