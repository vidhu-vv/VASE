#include "sh_encoder.h"
#include <cmath>

SHEncoder::SHEncoder() { set_direction(0.0f, 0.0f); }

void SHEncoder::set_direction(float az_deg, float el_deg) {
  float az = az_deg * M_PI / 180.0f;
  float el = el_deg * M_PI / 180.0f;

  m_W = 0.7071f;
  m_X = cosf(el) * cosf(az);
  m_Y = cosf(el) * sinf(az);
  m_Z = sinf(el);
}

void SHEncoder::process(const float *input, float *out_W, float *out_X,
                        float *out_Y, float *out_Z, int num_samples) {
  for (int i = 0; i < num_samples; ++i) {
    out_W[i] = input[i] * m_W;
    out_X[i] = input[i] * m_X;
    out_Y[i] = input[i] * m_Y;
    out_Z[i] = input[i] * m_Z;
  }
}
