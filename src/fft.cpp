#include "fft.h"
#include <cassert>
#include <cmath>
#include <utility>
#include <vector>

Complex::Complex(float r, float i) : r(r), i(i) {};
Complex Complex::operator+(const Complex &other) const {
  return {r + other.r, i + other.i};
}

Complex Complex::operator-(const Complex &other) const {
  return {r - other.r, i - other.i};
}

Complex Complex::operator*(const Complex &other) const {
  return {r * other.r - i * other.i, r * other.i + i * other.r};
}

void Complex::operator*=(const Complex &other) {
  float next_r = r * other.r - i * other.i;
  float next_i = r * other.i + i * other.r;

  r = next_r;
  i = next_i;
}

void Complex::operator/=(float n) {
  r /= n;
  i /= n;
}

static std::vector<Complex> twiddle_table;
static std::vector<int> bitrev_table;
static int table_N = 0;
static int table_m = 0;

static int bit_reverse(int x, int m) {
  int result = 0;
  for (int i = 0; i < m; ++i) {
    result = (result << 1) | (x & 1);
    x >>= 1;
  }
  return result;
}

void fft_precompute(int N) {
  assert(N > 0 && (N & (N - 1)) == 0);

  int m = 0;
  int tmp = N;
  while (tmp > 1) {
    tmp >>= 1;
    ++m;
  }
  twiddle_table.resize(N);
  table_N = N;
  table_m = m;

  for (int k = 0; k < N; k++) {
    float angle = -2.0 * M_PI * k / N;
    twiddle_table[k] = {cosf(angle), sinf(angle)};
  }

  bitrev_table.resize(N);
  for (int i = 0; i < N; ++i) {
    bitrev_table[i] = bit_reverse(i, m);
  }
}

void fft(Complex *data, int N) {
  assert(N == table_N && "fft_precompute not called with correct size");
  for (int i = 0; i < N; ++i) {
    int j = bitrev_table[i];
    if (j > i) {
      std::swap(data[i], data[j]);
    }
  }

  for (int l = 2; l <= N; l <<= 1) {
    int half = l >> 1;
    int stride = table_N / l;

    for (int k = 0; k < N; k += l) {
      for (int j = 0; j < half; j++) {
        Complex w = twiddle_table[j * stride];
        Complex u = data[k + j];
        Complex t = w * data[k + j + half];

        data[k + j] = u + t;
        data[k + j + half] = u - t;
      }
    }
  }
}

void ifft(Complex *data, int N) {
  for (int i = 0; i < N; ++i) {
    int j = bitrev_table[i];
    if (j > i) {
      std::swap(data[i], data[j]);
    }
  }

  for (int l = 2; l <= N; l <<= 1) {
    int half = l >> 1;
    int stride = table_N / l;

    for (int k = 0; k < N; k += l) {
      for (int j = 0; j < half; j++) {
        Complex w = twiddle_table[(table_N - (j * stride)) % table_N];
        Complex u = data[k + j];
        Complex t = w * data[k + j + half];

        data[k + j] = u + t;
        data[k + j + half] = u - t;
      }
    }
  }

  for (int i = 0; i < N; ++i)
    data[i] /= (float)N;
}
