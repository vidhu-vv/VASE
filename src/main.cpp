#define MINIAUDIO_IMPLEMENTATION
#include "../deps/miniaudio.h"
#include "fft.h"

#include "fft_convolver.h"
#include "hrtf_decoder.h"
#include <stdio.h>
void data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                   ma_uint32 frameCount) {
  ma_decoder *pDecoder = (ma_decoder *)pDevice->pUserData;
  if (pDecoder == NULL) {
    return;
  }
  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

  (void)pInput;
}

int main(int argc, char **argv) {
  fft_precompute(1024);

  // dirac delta IR — output should equal input
  float ir[256] = {};
  ir[0] = 1.0f;

  Convolver conv(ir, 256, 512);

  // make a test input block with some recognisable values
  float input[512] = {};
  float output[512] = {};
  input[0] = 1.0f;
  input[1] = 0.5f;
  input[2] = -0.5f;
  input[100] = 0.75f;

  conv.process(input, output);

  // output should match input exactly
  printf("Dirac test:\n");
  for (int i = 0; i < 10; i++) {
    printf("  in=%.4f  out=%.4f  diff=%.6f\n", input[i], output[i],
           fabsf(input[i] - output[i]));
  }
  printf("  in[100]=%.4f  out[100]=%.4f\n", input[100], output[100]);
  fft_precompute(1024);
  Decoder h_decoder(512);

  // make some dry audio
  float dry[512] = {};
  for (int i = 0; i < 512; i++)
    dry[i] = sinf(2.0f * M_PI * 440.0f * i / 48000.0f); // 440hz tone

  // manually encode at azimuth=0, elevation=0
  // W=0.707, X=cos(0)cos(0)=1, Y=cos(0)sin(0)=0, Z=sin(0)=0
  float W[512], X[512], Y[512], Z[512];
  for (int i = 0; i < 512; i++) {
    W[i] = dry[i] * 0.7071f;
    X[i] = dry[i] * 0.0f;
    Y[i] = dry[i] * 1.0f;
    Z[i] = dry[i] * 0.0f;
  }

  const float *foa[4] = {W, X, Y, Z};
  float out_L[512] = {};
  float out_R[512] = {};

  h_decoder.process(foa, out_L, out_R);

  // at 0° azimuth L and R should be nearly identical
  printf("90 right test (R should be louder than L):\n");
  for (int i = 100; i < 108; i++) {
    printf("  L=%.4f  R=%.4f\n", out_L[i], out_R[i]);
  }
  ma_result result;
  ma_decoder decoder;
  ma_device_config deviceConfig;
  ma_device device;

  if (argc < 2) {
    printf("No input file.\n");
    return -1;
  }

  result = ma_decoder_init_file(argv[1], NULL, &decoder);
  if (result != MA_SUCCESS) {
    printf("Could not load file: %s\n", argv[1]);
    return -2;
  }

  deviceConfig = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format = decoder.outputFormat;
  deviceConfig.playback.channels = decoder.outputChannels;
  deviceConfig.sampleRate = decoder.outputSampleRate;
  deviceConfig.dataCallback = data_callback;
  deviceConfig.pUserData = &decoder;

  if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
    printf("Failed to open playback device.\n");
    ma_decoder_uninit(&decoder);
    return -3;
  }
  if (ma_device_start(&device) != MA_SUCCESS) {
    printf("Failed to open playback device.\n");
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
    return -4;
  }

  printf("Press Enter to quit...");
  getchar();
  ma_device_uninit(&device);
  ma_decoder_uninit(&decoder);

  return 0;
}
