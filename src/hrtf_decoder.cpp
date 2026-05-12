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
  };

  for (int i = 0; i < N_SPEAKERS; ++i) {
    m_speaker_az[i] = spk_pos[i][0];
    m_speaker_el[i] = spk_pos[i][1];
  }

  const float norm = (float)N_FOA_CHANNELS / (float)N_SPEAKERS;
  for (int i = 0; i < N_SPEAKERS; ++i) {
    float az = m_speaker_az[i] * M_PI / 180.0f;
    float el = m_speaker_el[i] * M_PI / 180.0f;

    m_decode_matrix[i][0] = 0.7071f;
    m_decode_matrix[i][1] = cosf(el) * cosf(az);
    m_decode_matrix[i][2] = cosf(el) * sinf(az);
    m_decode_matrix[i][3] = sinf(el);

    for (int j = 0; j < N_FOA_CHANNELS; ++j) {
      m_decode_matrix[i][j] *= norm;
    }
  }

  for (int i = 0; i < N_SPEAKERS; ++i) {
    m_speaker_buf[i].resize(block_size, 0.0f);
    m_conv_out_left[i].resize(block_size, 0.0f);
    m_conv_out_right[i].resize(block_size, 0.0f);
  }

  for (int i = 0; i < N_SPEAKERS; ++i) {
    int dir = find_closest_hrtf(m_speaker_az[i], m_speaker_el[i]);

    m_conv_right[i] = Convolver(hrtf_irs[dir][1], HRTF_IR_LENGTH, block_size);
    m_conv_left[i] = Convolver(hrtf_irs[dir][0], HRTF_IR_LENGTH, block_size);
  }
}

void Decoder::process(const float *foa[4], float *out_L, float *out_R) {
  for (int i = 0; i < N_SPEAKERS; ++i) {
    for (int j = 0; j < m_block_size; ++j) {
      m_speaker_buf[i][j] = m_decode_matrix[i][0] * foa[0][j] +
                            m_decode_matrix[i][1] * foa[1][j] +
                            m_decode_matrix[i][2] * foa[2][j] +
                            m_decode_matrix[i][3] * foa[3][j];
    }
  }

  for (int i = 0; i < N_SPEAKERS; ++i) {
    m_conv_left[i].process(m_speaker_buf[i].data(), m_conv_out_left[i].data());
    m_conv_right[i].process(m_speaker_buf[i].data(),
                            m_conv_out_right[i].data());
  }

  memset(out_L, 0, m_block_size * sizeof(float));
  memset(out_R, 0, m_block_size * sizeof(float));

  for (int i = 0; i < N_SPEAKERS; ++i) {
    for (int j = 0; j < m_block_size; ++j) {
      out_L[j] += m_conv_out_left[i][j];
      out_R[j] += m_conv_out_right[i][j];
    }
  }
}
