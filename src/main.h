#if !defined(MAIN_H)

struct win32_offscreen_buffer {
  // NOTE pixels are always 32bits wide, Memory order BB GG RR XX
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int pitch;
  int bytesPerPixel;
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
  int safetyBytes;
  float_t tSine;
  int latencySampleCount;
};

struct win32_debug_time_marker {
  DWORD outputPlayCursor;
  DWORD outputWriteCursor;
  DWORD byteToLock;
  DWORD bytesToWrite;
  DWORD expectedFlipPlayCursor;
  DWORD flipPlayCursor;
  DWORD flipWriteCursor;
};

#define MAIN_H
#endif