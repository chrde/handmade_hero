#include "handmade.h"

internal void gameOutputSound(game_sound_output_buffer *soundBuffer, int toneHz){
  local_persist float_t tSine;
  int16_t toneVolume = 3000;
  int wavePeriod = soundBuffer->samplesPerSecond / toneHz;

  int16_t * sample = soundBuffer->samples;
      for (int sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; ++sampleIndex) {
      float_t sineValue = sinf(tSine);
      int16_t sampleValue = (int16_t)(sineValue * toneVolume);
      *sample++ = sampleValue;
      *sample++ = sampleValue;
      tSine += 2.0f * PI * 1.0f / (float_t)wavePeriod;
    }
}

internal void renderSomething(game_offscreen_buffer *buffer, int xOffset, int yOffset) {
  uint8_t *row = (uint8_t *)buffer->memory;
  for (int y = 0; y < buffer->height; ++y) {
    uint32_t *pixel = (uint32_t *)row;
    for (int x = 0; x < buffer->width; ++x) {
      uint8_t green = y + yOffset;
      uint8_t blue = x + xOffset;
      *pixel = ((green << 8) | blue);
      ++pixel;
    }
    row += buffer->pitch;
  }
}

internal void GameUpdateAndRender(game_offscreen_buffer *buffer, game_sound_output_buffer *soundBuffer, int blueOffset, int greenOffset, int toneHz) {
  gameOutputSound(soundBuffer, toneHz);
  renderSomething(buffer, blueOffset, greenOffset);
}