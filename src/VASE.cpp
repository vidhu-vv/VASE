#include "VASE.h"

VASE::VASE(int block_size, float sample_rate)
    : m_block_size(block_size), m_sample_rate(sample_rate),
      m_decoder(block_size), m_rotation(0.5f, sample_rate, block_size) {
  m_W.resize(block_size, 0.0f);
  m_X.resize(block_size, 0.0f);
  m_Y.resize(block_size, 0.0f);
  m_Z.resize(block_size, 0.0f);
}

void VASE::process(const float *input, float *out_L, float *out_R) {
  float next_az = m_rotation.next_az();
  m_encoder.set_direction(next_az, 0.0f);
  m_encoder.process(input, m_W.data(), m_X.data(), m_Y.data(), m_Z.data(),
                    m_block_size);

  const float *foa[4] = {m_W.data(), m_X.data(), m_Y.data(), m_Z.data()};

  m_decoder.process(foa, out_L, out_R);
}
