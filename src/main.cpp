#define MINIAUDIO_IMPLEMENTATION
#include "../deps/miniaudio.h"
#include "fft.h"

#include "fft_convolver.h"
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
