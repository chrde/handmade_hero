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

internal uint32_t RoundReal32ToUInt32(float_t n) {
  uint32_t result = (uint32_t)(n + 0.5f);
  return result;
}

internal void DrawRectangle(game_offscreen_buffer *buffer, float_t realminX, float_t realMinY, float_t realMaxX,
                            float_t realMaxY, float_t R, float_t G, float_t B) {
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

  uint32_t color = ((RoundReal32ToUInt32(R * 255.0f) << 16) | (RoundReal32ToUInt32(G * 255.0f) << 8) |
                    (RoundReal32ToUInt32(B * 255.0f) << 0));

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
      float_t dPlayerX = 0.0f;
      float_t dPlayerY = 0.0f;

      if (controller->moveUp.endedDown) {
        dPlayerY = -1.0f;
      }
      if (controller->moveDown.endedDown) {
        dPlayerY = 1.0f;
      }
      if (controller->moveLeft.endedDown) {
        dPlayerX = -1.0f;
      }
      if (controller->moveRight.endedDown) {
        dPlayerX = 1.0f;
      }
      dPlayerX *= 64.0f;
      dPlayerY *= 64.0f;

      gameState->playerX += input->dtForFrame * dPlayerX;
      gameState->playerY += input->dtForFrame * dPlayerY;
    }
  }
  uint32_t tileMap[9][17] =
  {
      {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
      {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
      {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
      {0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0},
      {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
      {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
      {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
      {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
  };

  float_t upperLeftX = -30;
  float_t upperLeftY = 0;
  float_t tileWidth = 60;
  float_t tileHeight = 60;
  
  DrawRectangle(buffer, 0.0f, 0.0f, (float_t)buffer->width, (float_t)buffer->height,
                1.0f, 0.0f, 0.1f);
  for(int row = 0;
      row < 9;
      ++row)
  {
      for(int column = 0;
          column < 17;
          ++column)
      {
          uint32_t tileID = tileMap[row][column];
          float_t gray = 0.5f;
          if(tileID == 1)
          {
              gray = 1.0f;
          }

          float_t minX = upperLeftX + ((float_t)column)*tileWidth;
          float_t minY = upperLeftY + ((float_t)row)*tileHeight;
          float_t maxX = minX + tileWidth;
          float_t maxY = minY + tileHeight;
          DrawRectangle(buffer, minX, minY, maxX, maxY, gray, gray, gray);
      }
  }
  
  float_t playerR = 1.0f;
  float_t playerG = 1.0f;
  float_t playerB = 0.0f;
  float_t playerWidth = 0.75f*tileWidth;
  float_t playerHeight = tileHeight;
  float_t playerLeft = gameState->playerX - 0.5f*playerWidth;
  float_t playerTop = gameState->playerY - playerHeight;
  DrawRectangle(buffer,
                playerLeft, playerTop,
                playerLeft + playerWidth,
                playerTop + playerHeight,
                playerR, playerG, playerB);

}

extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples) {
  game_state *gameState = (game_state *)memory->permanentStorage;
  gameOutputSound(soundBuffer, gameState, 400);
}