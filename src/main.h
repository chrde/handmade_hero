#if !defined(MAIN_H)


struct win32_offscreen_buffer {
  // NOTE pixels are always 32bits wide, Memory order BB GG RR XX
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int pitch;
};
struct win32_window_dimension {
  int width;
  int height;
};

struct win32_sound_output {
  int samplesPerSecond;
  // int toneHz;
  uint32_t runningSampleIndex;
  int bytesPerSample;
  int audioBufferSize;
  float_t tSine;
  int latencySampleCount;
};

#define MAIN_H
#endif