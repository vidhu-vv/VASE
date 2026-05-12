#ifndef __VASE_ROTATION__
#define __VASE_ROTATION__
class Rotation {
public:
  Rotation(float spin_rate_hz, float sample_rate, int block_size);
  float next_az();

private:
  float m_phase;
  float m_inc;
};
#endif // !__VASE_ROTATION__
