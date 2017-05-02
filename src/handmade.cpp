#include "handmade.h"

internal void renderSomething(game_offscreen_buffer *buffer, int xOffset,
                              int yOffset) {
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

internal void GameUpdateAndRender(game_offscreen_buffer *buffer, int blueOffset,
                                  int greenOffset) {
  renderSomething(buffer, blueOffset, greenOffset);
}