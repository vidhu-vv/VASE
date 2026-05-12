#ifndef __VASE_HRTF_DECODER__
#define __VASE_HRTF_DECODER__

#include "fft_convolver.h"
static const int N_SPEAKERS = 8;
static const int N_FOA_CHANNELS = 4;

class Decoder {
private:
  int m_block_size;
  float m_speaker_az[8];
  float m_speaker_el[8];

  float m_decode_matrix[8][4];

  Convolver m_conv_left[8];
  Convolver m_conv_right[8];

  std::vector<float> m_speaker_buf[8];
  std::vector<float> m_conv_out_left[8];
  std::vector<float> m_conv_out_right[8];

public:
  Decoder(int block_size);
  void process(const float *foa[4], float *out_L, float *out_R);
};

#endif
