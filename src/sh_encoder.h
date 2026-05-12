#ifndef __VASE_SHENCODER__
#define __VASE_SHENCODER__

class SHEncoder {
public:
  SHEncoder();
  void set_direction(float az_deg, float el_deg);
  void process(const float *input, float *out_W, float *out_X, float *out_Y,
               float *out_Z, int num_samples);

private:
  float m_W;
  float m_X;
  float m_Y;
  float m_Z;
};
#endif // !__VASE_SHENCODER__
