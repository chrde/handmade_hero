#if !defined(HANDMADE_H)
#include "platform.h"
#define internal static
#define local_persist static
#define global_variable static
#define PI 3.14159265359f

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

inline game_controller_input *getController(game_input *input, int unsigned controllerIndex) {
  assert(controllerIndex < arrayCount(input->controllers));
  game_controller_input *result = &input->controllers[controllerIndex];
  return result;
}

struct memory_arena {
  memory_index size;
  uint8_t *base;
  memory_index used;
};

inline void initializeArena(memory_arena *arena, memory_index size, uint8_t *base) {
  arena->size = size;
  arena->base = base;
  arena->used = 0;
}

#define pushStruct(arena, type) (type *)pushSize_(arena, sizeof(type))
#define pushArray(arena, count, type) (type *)pushSize_(arena, (count) * sizeof(type))
void *pushSize_(memory_arena *arena, memory_index size) {
  assert((arena->used + size) <= arena->size);
  void *result = arena->base + arena->used;
  arena->used += size;
  return result;
}
#include "intrinsics.h"
#include "tile.h"

struct world {
  tile_map *tileMap;
};

struct game_state {
  memory_arena worldArena;
  world *world;
  tile_map_position playerP;
};

#define HANDMADE_H
#endif
