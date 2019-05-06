#ifndef DATAFORMAT_H
#define DATAFORMAT_H

#define AP_INT_MAX_W 2048
#include "ap_int.h"
#include <vector>
#include <string>
#include <iostream>

// the definitions of the data format
const unsigned int mu_w_size   = 40;
const unsigned int trk_w_size  = 100;
const unsigned int tkmu_w_size = 100;
const unsigned int n_mu        = 12;
const unsigned int n_trk       = 15;
const unsigned int n_tkmu      = 12;
const unsigned int trkoffset   = mu_w_size*n_mu;

const unsigned int mu_theta_w_size     = 11; // includes sign bit
const unsigned int mu_phi_w_size       = 11;
const unsigned int mu_pt_w_size        = 15;

const unsigned int trk_theta_w_size    = 15; // includes sign bit
const unsigned int trk_phi_w_size      = 15;
const unsigned int trk_pt_w_size       = 15;

const unsigned int tkmu_theta_w_size   = 15; // includes sign bit
const unsigned int tkmu_phi_w_size     = 15;
const unsigned int tkmu_pt_w_size      = 15;

// FIXME: to be made uniform
const unsigned int theta_w_size        = 6;
const unsigned int phi_w_size          = 6;


//////////////////////////////

// combine the 64 contiguous addresses at a certain ibx into an ap_uint<2048>
ap_uint<2048> buffer_to_2048 (uint32_t *buf, size_t ibx, uint32_t latency = 0)
{
    //// build a full in and out word into an ap_uint
    ap_uint<2048> word;
    for (uint ipart = 0; ipart < 64; ++ipart)
    {
      uint32_t mem_addr = ibx*64+ipart; // get to the chunk with the 64 bits offset, and add offset from the specific piece
      word.range((ipart+1)*32-1, (ipart)*32) = buf[mem_addr+latency];
      // printf("%i [%i] => %i\n", ipart, (mem_addr+latency), buf[mem_addr+latency]);
    }
    return word;
}

// converts the input buffer of the write memory into the injected word @ bx = ibx + the spy values + a vector for the values
// returns by reference
void readout_input_buf (uint32_t *write_buf, size_t ibx,
    ap_uint<2048>& word,
    ap_uint<trk_w_size> &trk1, ap_uint<trk_w_size> &trk2,
    ap_uint<mu_w_size>  &mu1,  ap_uint<mu_w_size>  &mu2,
    std::vector <ap_uint<trk_w_size> > &all_tracks,
    std::vector <ap_uint<mu_w_size> >  &all_muons)
{
    word = buffer_to_2048 (write_buf, ibx, 0);

    // debug the input buffer readout
    //// >>>> range!! (high, low) (32, 63) or (low, high) (63,32) ???
    // printf("MY DEBUG   buf0=%8x   buf1=%8x   word-p0=%8x   word-p1=%8x\n", write_buf[0], write_buf[1], (uint32_t) word.range(31, 0).to_uint(), (uint32_t) word.range(63, 32).to_uint());
    // for (size_t ic = 0; ic < 64; ++ic)
    // {
    //     uint32_t from_write_buf = write_buf[ic];
    //     int imin = ic*32;
    //     int imax = (ic+1)*32 -1;
    //     uint32_t from_word = (uint32_t) (word.range(imax, imin).to_uint());
    //     std::string msg = (from_write_buf == from_word ? "" : "<<<<");
    //     printf("MY DEBUG   idx=%3i   buf=%8x   word=%8x   imin=%4i, imax=%4i %s\n", ic, from_write_buf, from_word, imin, imax, msg.c_str());
    // }

    trk1  = word.range(trk_w_size-1 + mu_w_size*n_mu, mu_w_size*n_mu);
    trk2  = word.range(trk_w_size*2-1 + mu_w_size*n_mu, trk_w_size + mu_w_size*n_mu);
    mu1   = word.range(mu_w_size-1, 0);
    mu2   = word.range(mu_w_size*2-1, mu_w_size);

    all_tracks.resize(n_trk);
    for (unsigned int i = 0; i < n_trk; ++i){
        all_tracks.at(i) = word.range(trk_w_size*(i+1)-1 + mu_w_size*n_mu, trk_w_size*i + mu_w_size*n_mu);
    }
    all_muons.resize(n_mu);
    for (unsigned int i = 0; i < n_mu; ++i){
        all_muons.at(i) = word.range(mu_w_size*(i+1)-1, mu_w_size*i);
    }

}

// converts the out buffer of the read memory into the injected word @ bx = ibx + the spy values
void readout_output_buf (uint32_t *read_buf, size_t ibx, uint32_t latency,
    ap_uint<2048>& word,
    ap_uint<tkmu_w_size> &tkmu1, ap_uint<tkmu_w_size> &tkmu2,
    std::vector <ap_uint<tkmu_w_size> >  &all_tkmus)
{
    word = buffer_to_2048 (read_buf, ibx, latency);
    tkmu1 = word.range(tkmu_w_size-1, 0);
    tkmu2 = word.range(tkmu_w_size*2-1, tkmu_w_size);
    all_tkmus.resize(n_tkmu);
    for (unsigned int i = 0; i < n_tkmu; ++i){
        all_tkmus.at(i) = word.range(tkmu_w_size*(i+1)-1, tkmu_w_size*i);
    }

}


ap_uint<mu_pt_w_size> get_mu_pt(ap_uint<mu_w_size> word);
ap_uint<mu_phi_w_size> get_mu_phi(ap_uint<mu_w_size> word);
ap_uint<mu_theta_w_size> get_mu_theta(ap_uint<mu_w_size> word);
////////
ap_uint<trk_pt_w_size> get_trk_pt(ap_uint<trk_w_size> word);
ap_uint<trk_phi_w_size> get_trk_phi(ap_uint<trk_w_size> word);
ap_uint<trk_theta_w_size> get_trk_theta(ap_uint<trk_w_size> word);
////////
ap_uint<tkmu_pt_w_size> get_tkmu_pt(ap_uint<tkmu_w_size> word);
ap_uint<tkmu_phi_w_size> get_tkmu_phi(ap_uint<tkmu_w_size> word);
ap_uint<tkmu_theta_w_size> get_tkmu_theta(ap_uint<tkmu_w_size> word);

// unpackers
ap_uint<mu_pt_w_size> get_mu_pt(ap_uint<mu_w_size> word){
    return word.range(mu_pt_w_size-1, 0);
}
ap_uint<mu_phi_w_size> get_mu_phi(ap_uint<mu_w_size> word){
    return word.range(mu_pt_w_size + mu_phi_w_size -1, mu_pt_w_size);
}
ap_uint<mu_theta_w_size> get_mu_theta(ap_uint<mu_w_size> word){
    return word.range(mu_pt_w_size + mu_phi_w_size + mu_theta_w_size -1, mu_pt_w_size + mu_phi_w_size);
}
////////
ap_uint<trk_pt_w_size> get_trk_pt(ap_uint<trk_w_size> word){
    return word.range(trk_pt_w_size-1, 0);
}
ap_uint<trk_phi_w_size> get_trk_phi(ap_uint<trk_w_size> word){
    return word.range(trk_pt_w_size + trk_phi_w_size -1, trk_pt_w_size);
}
ap_uint<trk_theta_w_size> get_trk_theta(ap_uint<trk_w_size> word){
    return word.range(trk_pt_w_size + trk_phi_w_size + trk_theta_w_size -1, trk_pt_w_size + trk_phi_w_size);
}
////////
ap_uint<tkmu_pt_w_size> get_tkmu_pt(ap_uint<tkmu_w_size> word){
    return word.range(tkmu_pt_w_size-1, 0);
}
ap_uint<tkmu_phi_w_size> get_tkmu_phi(ap_uint<tkmu_w_size> word){
    return word.range(tkmu_pt_w_size + tkmu_phi_w_size -1, tkmu_pt_w_size);
}
ap_uint<tkmu_theta_w_size> get_tkmu_theta(ap_uint<tkmu_w_size> word){
    return word.range(tkmu_pt_w_size + tkmu_phi_w_size + tkmu_theta_w_size -1, tkmu_pt_w_size + tkmu_phi_w_size);
}

/////// ancillary functions to print to screen an ap::uint value
template <size_t N>
std::string ap_uint_to_string (ap_uint<N> val, bool pad_space = true)
{
    size_t left = N;
    size_t offset = 0;
    std::string result = "";
    std::string sep = pad_space ? " " : "";
    // the ones I cannot fit in uint64
    while (left > 64)
    {

        std::string out_string;
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(64/4) << val.range(63+offset, 0+offset).to_uint64();
        out_string = ss.str();
        result = out_string + sep + result;
        // std::cout << " ..... " << 63+offset << " " << 0+offset << " ,,, " << out_string << std::endl;
        left -= 64;
        offset += 64;
    }

    // the final chiunk
    std::string out_string;
    std::stringstream ss;
    size_t tot_s = N - offset;
    ss << std::hex << std::setfill('0') << std::setw(tot_s/4) << val.range(N-1, 0+offset).to_uint64();
    out_string = ss.str();
    result = out_string + sep + result;

    return result;
}


#endif