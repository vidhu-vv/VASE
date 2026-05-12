#include "hrtf_decoder.h"
#include "fft_convolver.h"
#include "hrtf_data.h"
#include <cmath>
#include <cstring>

Decoder::Decoder(int block_size) : m_block_size(block_size) {
  const float spk_pos[N_SPEAKERS][2] = {
    {45.0f, 35.0f},   // front-left-above
    {315.0f, 35.0f},  // front-right-above
    {135.0f, 35.0f},  // rear-left-above
    {225.0f, 35.0f},  // rear-right-above
    {45.0f, -35.0f},  // front-left-below
    {315.0f, -35.0f}, // front-right-below
    {135.0f, -35.0f}, // rear-left-below
    {225.0f, -35.0f}, // rear-right-below
  }
}
