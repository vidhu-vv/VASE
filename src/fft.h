#ifndef __VASE_FFT__
#define __VASE_FFT__

struct Complex {
  float r, i;

  Complex(float r = 0, float i = 0);

  Complex operator+(const Complex &other) const;
  Complex operator-(const Complex &other) const;
  Complex operator*(const Complex &other) const;
  void operator*=(const Complex &other);
  void operator/=(float n);
};

void fft_precompute(int N);
static int bit_reverse(int x, int m);

void fft(Complex *data, int N);
void ifft(Complex *data, int N);

#endif
