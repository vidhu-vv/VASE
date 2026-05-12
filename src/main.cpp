#define MINIAUDIO_IMPLEMENTATION
#include "../deps/miniaudio.h"
#include "fft.h"
#include "rotation.h"

#include "VASE.h"
#include "fft_convolver.h"
#include "hrtf_decoder.h"
#include "sh_encoder.h"
#include <stdio.h>

struct CallbackData {
  ma_decoder *decoder;
  VASE *engine;
  int block_size;
  float *mono_buf;
  float *out_L;
  float *out_R;
};

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                   ma_uint32 frameCount) {

  CallbackData *cb = (CallbackData *)pDevice->pUserData;
  if (!cb)
    return;

  ma_uint64 frames_read = 0;
  ma_decoder_read_pcm_frames(cb->decoder, cb->mono_buf, frameCount,
                             &frames_read);

  if (frames_read < frameCount) {
    memset(cb->mono_buf + frames_read, 0,
           (frameCount - frames_read) * sizeof(float));
  }

  cb->engine->process(cb->mono_buf, cb->out_L, cb->out_R);

  float *output = (float *)pOutput;

  for (ma_uint32 i = 0; i < frameCount; ++i) {
    output[2 * i + 0] = cb->out_L[i];
    output[2 * i + 1] = cb->out_R[i];
  }

  (void)pInput;
}

int main(int argc, char **argv) {
  fft_precompute(1024);
  const int BLOCK_SIZE = 512;
  const float SAMPLE_RATE = 48000.0f;

  VASE engine(BLOCK_SIZE, SAMPLE_RATE);

  float *mono_buf = new float[BLOCK_SIZE]();
  float *out_L = new float[BLOCK_SIZE]();
  float *out_R = new float[BLOCK_SIZE]();

  ma_decoder_config deviceConfig =
      ma_decoder_config_init(ma_format_f32, 1, (ma_uint32)SAMPLE_RATE);
  ma_decoder file_decoder;
  if (ma_decoder_init_file(argv[1], &deviceConfig, &file_decoder) !=
      MA_SUCCESS) {
    printf("could not open file\n");
    return -1;
  }
  CallbackData cb = {&file_decoder, &engine, BLOCK_SIZE,
                     mono_buf,      out_L,   out_R};
  ma_device_config dev_cfg = ma_device_config_init(ma_device_type_playback);
  dev_cfg.playback.format = ma_format_f32;
  dev_cfg.playback.channels = 2;
  dev_cfg.sampleRate = (ma_uint32)SAMPLE_RATE;
  dev_cfg.dataCallback = data_callback;
  dev_cfg.pUserData = &cb;
  dev_cfg.periodSizeInFrames = 512;

  ma_device device;
  if (ma_device_init(NULL, &dev_cfg, &device) != MA_SUCCESS) {
    printf("failed to open device\n");
    return -2;
  }

  ma_device_start(&device);
  printf("playing — press enter to quit\n");
  getchar();

  ma_device_uninit(&device);
  ma_decoder_uninit(&file_decoder);
  delete[] mono_buf;
  delete[] out_L;
  delete[] out_R;

  return 0;
}
