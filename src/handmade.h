#if !defined(HANDMADE_H)
#define arrayCount(array) (sizeof(array) / sizeof((array)[0]))
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
  bool32 isAnalog;
  float_t startX;
  float_t startY;
  float_t minX;
  float_t minY;
  float_t maxX;
  float_t maxY;
  float_t endX;
  float_t endY;
  union {
    game_button_state buttons[6];
    struct {
      game_button_state up;
      game_button_state down;
      game_button_state left;
      game_button_state right;
      game_button_state leftShoulder;
      game_button_state rightShoulder;
    };
  };
};
struct game_input {
  game_controller_input controllers[4];
};
internal void GameUpdateAndRender(game_offscreen_buffer *buffer, game_sound_output_buffer *soundBuffer, game_input *input);
internal void gameOutputSound(game_sound_output_buffer *soundBuffer, int toneHz);

#define HANDMADE_H
#endif