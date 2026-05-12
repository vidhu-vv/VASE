#ifndef __VASE_CONVOLVER__
#define __VASE_CONVOLVER__

#include "fft.h"
#include <vector>

class Convolver {
private:
  int m_ir_len;
  int m_block_size;
  int m_fft_size;

  std::vector<Complex> m_H;
  std::vector<float> m_history;
  std::vector<Complex> m_work_buf;

public:
  Convolver(const float *ir, int ir_len, int block_size);
  Convolver();

  void process(const float *input, float *output);
};

#endif
