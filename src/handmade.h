#if !defined(HANDMADE_H)
struct game_offscreen_buffer {
  // NOTE pixels are always 32bits wide, Memory order BB GG RR XX
  void *memory;
  int width;
  int height;
  int pitch;
};
internal void GameUpdateAndRender(game_offscreen_buffer *buffer, int blueOffset, int greenOffset);

#define HANDMADE_H
#endif