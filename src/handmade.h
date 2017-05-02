#if !defined(HANDMADE_H)
struct game_offscreen_buffer {
  // NOTE pixels are always 32bits wide, Memory order BB GG RR XX
  void *memory;
  int width;
  int height;
  int pitch;
};
struct game_sound_output_buffer{
  int samplesPerSecond;
  int sampleCount;
  int16_t *samples;
};
internal void GameUpdateAndRender(game_offscreen_buffer *buffer, game_sound_output_buffer *soundBuffer, int blueOffset, int greenOffset);
internal void gameOutputSound(game_sound_output_buffer *soundBuffer, int toneHz);

#define HANDMADE_H
#endif