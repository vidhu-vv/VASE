#ifndef __VASE_ENGINE__
#define __VASE_ENGINE__
#include "hrtf_decoder.h"
#include "rotation.h"
#include "sh_encoder.h"
#include <vector>

class VASE {
public:
  VASE(int block_size, float sample_rate);
  void process(const float *input, float *out_L, float *out_R);

private:
  int m_sample_rate;
  int m_block_size;

  SHEncoder m_encoder;
  Decoder m_decoder;
  Rotation m_rotation;

  std::vector<float> m_W;
  std::vector<float> m_X;
  std::vector<float> m_Y;
  std::vector<float> m_Z;
};
#endif // !__VASE_ENGINE__
