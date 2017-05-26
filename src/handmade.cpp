#include "handmade.h"

internal void gameOutputSound(game_sound_output_buffer *soundBuffer, game_state *gameState, int toneHz) {
  int16_t toneVolume = 3000;
  int wavePeriod = soundBuffer->samplesPerSecond / toneHz;

  int16_t *sample = soundBuffer->samples;
  for (int sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; ++sampleIndex) {
    int16_t sampleValue = 0;
    *sample++ = sampleValue;
    *sample++ = sampleValue;
  }
}

internal int32_t RoundReal32ToInt32(float_t n) {
  int32_t result = (int32_t)(n + 0.5f);
  return result;
}

internal void DrawRectangle(game_offscreen_buffer *buffer, float_t realminX, float_t realMinY, float_t realMaxX,
                            float_t realMaxY, uint32_t color) {

  int32_t minX = RoundReal32ToInt32(realminX);
  int32_t minY = RoundReal32ToInt32(realMinY);
  int32_t maxX = RoundReal32ToInt32(realMaxX);
  int32_t maxY = RoundReal32ToInt32(realMaxY);

  if (minX < 0) {
    minX = 0;
  }

  if (minY < 0) {
    minY = 0;
  }

  if (maxX > buffer->width) {
    maxX = buffer->width;
  }

  if (maxY > buffer->height) {
    maxY = buffer->height;
  }

  uint8_t *row = ((uint8_t *)buffer->memory + minX * buffer->bytesPerPixel + minY * buffer->pitch);
  for (int y = minY; y < maxY; ++y) {
    uint32_t *pixel = (uint32_t *)row;
    for (int x = minX; x < maxX; ++x) {
      *pixel++ = color;
    }

    row += buffer->pitch;
  }
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender) {
  assert(sizeof(game_state) <= memory->permanentStorageSize);
  game_state *gameState = (game_state *)memory->permanentStorage;
  if (!memory->isInitialized) {
    memory->isInitialized = true;
  }

  for (int controllerIndex = 0; controllerIndex < arrayCount(input->controllers); ++controllerIndex) {
    game_controller_input *controller = getController(input, controllerIndex);
    if (controller->isAnalog) {
    } else {
    }
  }

    DrawRectangle(buffer, 0.0f, 0.0f, (float_t)buffer->width, (float_t)buffer->height, 0x00FF00FF);
    DrawRectangle(buffer, 10.0f, 10.0f, 40.0f, 40.0f, 0x0000FFFF);
}

extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples) {
  game_state *gameState = (game_state *)memory->permanentStorage;
  gameOutputSound(soundBuffer, gameState, 400);
}