#define MINIAUDIO_IMPLEMENTATION
#include "../deps/miniaudio.h"
#include "fft.h"

#include "fft_convolver.h"
#include "hrtf_decoder.h"
#include "sh_encoder.h"
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
  SHEncoder encoder;

  float dry[512] = {};
  for (int i = 0; i < 512; i++)
    dry[i] = sinf(2.0f * M_PI * 440.0f * i / 48000.0f);

  float W[512], X[512], Y[512], Z[512];
  const float *foa[4] = {W, X, Y, Z};

  // test 1 — front centre
  {
    Decoder decoder(512);
    float out_L[512] = {}, out_R[512] = {};
    encoder.set_direction(0.0f, 0.0f);
    encoder.process(dry, W, X, Y, Z, 512);
    decoder.process(foa, out_L, out_R);
    printf("front centre (L should ≈ R):\n");
    for (int i = 100; i < 106; i++)
      printf("  L=%.4f  R=%.4f  diff=%.6f\n", out_L[i], out_R[i],
             fabsf(out_L[i] - out_R[i]));
  }

  // test 2 — hard right
  {
    Decoder decoder(512);
    float out_L[512] = {}, out_R[512] = {};
    encoder.set_direction(90.0f, 0.0f);
    encoder.process(dry, W, X, Y, Z, 512);
    decoder.process(foa, out_L, out_R);
    printf("hard right (R should be louder):\n");
    for (int i = 100; i < 106; i++)
      printf("  L=%.4f  R=%.4f\n", out_L[i], out_R[i]);
  }

  // test 3 — hard left
  {
    Decoder decoder(512);
    float out_L[512] = {}, out_R[512] = {};
    encoder.set_direction(270.0f, 0.0f);
    encoder.process(dry, W, X, Y, Z, 512);
    decoder.process(foa, out_L, out_R);
    printf("hard left (L should be louder):\n");
    for (int i = 100; i < 106; i++)
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
