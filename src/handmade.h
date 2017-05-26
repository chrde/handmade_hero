#if !defined(HANDMADE_H)
#include <math.h>
#include <stdint.h>
#define internal static
#define local_persist static
#define global_variable static
#define PI 3.14159265359f
typedef int32_t bool32;

struct thread_context {
  int Placeholder;
};

#if HANDMADE_INTERNAL
struct debug_read_file_result {
  uint32_t contentsSize;
  void *contents;
};

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *thread, void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);
#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *thread, char *filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);
#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) \
  bool32 name(thread_context *thread, char *filename, uint32_t memorySize, void *memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

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
  int bytesPerPixel;
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
      game_button_state terminator;
    };
  };
};
struct game_input {
  game_button_state mouseButtons[5];
  int32_t mouseX, mouseY, mouseZ;
  float_t secondsToAdvanceOverUpdate;
  game_controller_input controllers[5];
};

inline game_controller_input *getController(game_input *input, int unsigned controllerIndex) {
  assert(controllerIndex < arrayCount(input->controllers));
  game_controller_input *controller = &input->controllers[controllerIndex];
  return controller;
}

struct game_state {
};

struct game_memory {
  bool32 isInitialized;
  uint64_t permanentStorageSize;
  void *permanentStorage;
  uint64_t transientStorageSize;
  void *transientStorage;
  debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
  debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
  debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
};

#define GAME_UPDATE_AND_RENDER(name) \
  void name(thread_context *thread, game_memory *memory, game_offscreen_buffer *buffer, game_input *input)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
#define GAME_GET_SOUND_SAMPLES(name) \
  void name(thread_context *thread, game_memory *memory, game_sound_output_buffer *soundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

#define HANDMADE_H
#endif
