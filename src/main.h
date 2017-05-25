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
  uint32_t runningSampleIndex;
  int bytesPerSample;
  int audioBufferSize;
  int safetyBytes;
  float_t tSine;
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

struct win32_game_code {
  HMODULE gameCodeDll;
  FILETIME dllLastWriteTime;
  game_update_and_render *updateAndRender;
  game_get_sound_samples *getSoundSamples;
  bool32 isValid;
};

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_replay_buffer {
  HANDLE fileHandle;
  HANDLE memoryMap;
  char fileName[WIN32_STATE_FILE_NAME_COUNT];
  void *memoryBlock;
};

struct win32_state {
  uint64_t totalSize;
  void *gameMemoryBlock;
  win32_replay_buffer replayBuffers[4];
  HANDLE recordingHandle;
  int inputRecordingIndex;
  HANDLE playbackHandle;
  int inputPlayingIndex;
  char exeFileName[WIN32_STATE_FILE_NAME_COUNT];
  char *onePastLastExeFileNameSlash;
};

#define MAIN_H
#endif