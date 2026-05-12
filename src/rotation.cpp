#include "rotation.h"
#include <cmath>

Rotation::Rotation(float spin_rate_hz, float sample_rate, int block_size) {
  m_phase = 0.0f;
  m_inc = 2.0f * M_PI * spin_rate_hz * block_size / sample_rate;
}

float Rotation::next_az() {
  m_phase += m_inc;

  if (m_phase >= 2.0f * M_PI) {
    m_phase -= 2.0f * M_PI;
  }

  return m_phase * 180.0f / M_PI;
}
