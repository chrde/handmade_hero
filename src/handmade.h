#if !defined(HANDMADE_H)
#if HANDMADE_INTERNAL
struct debug_read_file_result{
  uint32_t contentsSize;
  void *contents;
};
internal debug_read_file_result DEBUGPlatformReadEntireFile(char *filename);
internal void DEBUGPlatformFreeFileMemory(void *memory);
internal bool32 DEBUGPlatformWriteEntireFile(char *filename, uint32_t memorySize, void *memory);
#endif
#if HANDMADE_SLOW
#define assert(exp) \
  if (!(exp)) {     \
    *(int *)0 = 0;  \
  }
#else
#define assert(exp)
#endif

#define arrayCount(array) (sizeof(array) / sizeof((array)[0]))
#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value) * 1024LL)
#define Terabytes(value) (Gigabytes(value) * 1024LL)
#define Gigabytes(value) (Megabytes(value) * 1024LL)

inline uint32_t safeTruncateUInt64(uint64_t value) {
  assert(value < 0xFFFFFFFF);
  uint32_t fileSize32 = (uint32_t)value;
  return fileSize32;
}

struct game_offscreen_buffer {
  // NOTE pixels are always 32bits wide, Memory order BB GG RR XX
  void *memory;
  int width;
  int height;
  int pitch;
};
struct game_sound_output_buffer {
  int samplesPerSecond;
  int sampleCount;
  int16_t *samples;
};

struct game_button_state {
  int halfTransitionCount;
  bool32 endedDown;
};
struct game_controller_input {
  bool32 isConnected;
  bool32 isAnalog;
  float_t stickAverageX;
  float_t stickAverageY;
  union {
    game_button_state buttons[12];
    struct {
      game_button_state moveUp;
      game_button_state moveDown;
      game_button_state moveLeft;
      game_button_state moveRight;
      game_button_state actionUp;
      game_button_state actionDown;
      game_button_state actionLeft;
      game_button_state actionRight;
      game_button_state leftShoulder;
      game_button_state rightShoulder;
      game_button_state start;
      game_button_state back;
    };
  };
};
struct game_input {
  game_controller_input controllers[5];
};
inline game_controller_input *getController(game_input *input, int unsigned controllerIndex){
  assert(controllerIndex < arrayCount(input->controllers));
  game_controller_input *controller = &input->controllers[controllerIndex];
  return controller;
}
struct game_state {
  int toneHz;
  int greenOffset;
  int blueOffset;
};
struct game_memory {
  bool32 isInitialized;
  uint64_t permanentStorageSize;
  void *permanentStorage;
  uint64_t transientStorageSize;
  void *transientStorage;
};
internal void gameUpdateAndRender(game_memory *memory, game_offscreen_buffer *buffer, game_input *input);
internal void gameOutputSound(game_sound_output_buffer *soundBuffer, int toneHz);
internal void gameGetSoundSamples(game_memory *memory, game_sound_output_buffer *soundBuffer);

#define HANDMADE_H
#endif