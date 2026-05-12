#include "fft_convolver.h"

Convolver::Convolver(const float *ir, int ir_len, int block_size)
    : m_ir_len(ir_len), m_block_size(block_size) {

  int min_size = block_size + ir_len - 1;
  m_fft_size = 1;
  while (m_fft_size < min_size) {
    m_fft_size <<= 1;
  }

  m_H.resize(m_fft_size, {0.0, 0.0});
  m_history.resize(ir_len - 1, 0.0f);
  m_work_buf.resize(m_fft_size, {0.0, 0.0});

  for (int i = 0; i < m_fft_size; ++i) {
    m_work_buf[i] = {i < m_ir_len ? ir[i] : 0.0f, 0.0f};
  }

  fft(m_work_buf.data(), m_fft_size);

  m_H = m_work_buf;
  std::fill(m_work_buf.begin(), m_work_buf.end(), Complex{0.0f, 0.0f});
}

Convolver::Convolver() : m_fft_size(0), m_ir_len(0), m_block_size(0) {}
void Convolver::process(const float *input, float *output) {
  for (int i = 0; i < m_ir_len - 1; ++i) {
    m_work_buf[i] = {m_history[i], 0.0f};
  }

  for (int i = m_ir_len - 1; i < m_fft_size; ++i) {
    m_work_buf[i] = {
        i < m_ir_len - 1 + m_block_size ? input[i - m_ir_len + 1] : 0.0f, 0.0f};
  }

  fft(m_work_buf.data(), m_fft_size);

  for (int i = 0; i < m_fft_size; ++i) {
    m_work_buf[i] *= m_H[i];
  }

  ifft(m_work_buf.data(), m_fft_size);

  for (int i = 0; i < m_block_size; ++i) {
    output[i] = m_work_buf[i + m_ir_len - 1].r;
  }

  for (int i = 0; i < m_ir_len - 1; ++i) {
    m_history[i] = input[m_block_size - m_ir_len + 1 + i];
  }
}
