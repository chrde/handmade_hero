#include "handmade.h"

internal void gameOutputSound(game_sound_output_buffer *soundBuffer, game_state *gameState) {
  int16_t toneVolume = 3000;
  int wavePeriod = soundBuffer->samplesPerSecond / gameState->toneHz;

  int16_t *sample = soundBuffer->samples;
  for (int sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; ++sampleIndex) {
    float_t sineValue = sinf(gameState->tSine);
    int16_t sampleValue = (int16_t)(sineValue * toneVolume);
    *sample++ = sampleValue;
    *sample++ = sampleValue;
    gameState->tSine += 2.0f * PI * 1.0f / (float_t)wavePeriod;
    if (gameState->tSine > 2.0f * PI) {
      gameState->tSine -= 2.0f * PI;
    }
  }
}

internal void renderSomething(game_offscreen_buffer *buffer, int xOffset, int yOffset) {
  uint8_t *row = (uint8_t *)buffer->memory;
  for (int y = 0; y < buffer->height; ++y) {
    uint32_t *pixel = (uint32_t *)row;
    for (int x = 0; x < buffer->width; ++x) {
      uint8_t green = (uint8_t)(y + yOffset);
      uint8_t blue = (uint8_t)(x + xOffset);
      *pixel = ((green << 8) | blue);
      ++pixel;
    }
    row += buffer->pitch;
  }
}

internal void renderPlayer(game_offscreen_buffer *buffer, int playerX, int playerY) {
  uint8_t *endOfBuffer = (uint8_t *)buffer->memory + buffer->pitch * buffer->height;

  uint32_t color = 0xFFFFFFFF;
  int top = playerY;
  int bottom = playerY + 10;
  for (int x = playerX; x < playerX + 10; ++x) {
    uint8_t *pixel = ((uint8_t *)buffer->memory + x * buffer->bytesPerPixel + top * buffer->pitch);
    for (int y = top; y < bottom; ++y) {
      if ((pixel >= buffer->memory) && ((pixel + 4) <= endOfBuffer)) {
        *(uint32_t *)pixel = color;
      }

      pixel += buffer->pitch;
    }
  }
}

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender) {
  assert(sizeof(game_state) <= memory->permanentStorageSize);
  game_state *gameState = (game_state *)memory->permanentStorage;
  if (!memory->isInitialized) {
    char *filename = __FILE__;
    debug_read_file_result file = memory->DEBUGPlatformReadEntireFile(thread, filename);
    if (file.contents) {
      memory->DEBUGPlatformWriteEntireFile(thread, "C:/Users/chrde/github/handmade_hero/src/test.out",
                                           file.contentsSize, file.contents);
      memory->DEBUGPlatformFreeFileMemory(thread, file.contents);
    }
    gameState->toneHz = 256;
    gameState->tSine = 0.0f;
    gameState->playerX = 100;
    gameState->playerY = 100;
    memory->isInitialized = true;
  }

  for (int controllerIndex = 0; controllerIndex < arrayCount(input->controllers); ++controllerIndex) {
    game_controller_input *controller = getController(input, controllerIndex);
    if (controller->isAnalog) {
      gameState->toneHz = 256 + (int)(128.0f * controller->stickAverageY);
      gameState->blueOffset += (int)(4.0f * controller->stickAverageX);
    } else {
      if (controller->moveLeft.endedDown) {
        gameState->blueOffset += 1;
      }
      if (controller->moveRight.endedDown) {
        gameState->blueOffset -= 1;
      }
      if (controller->moveUp.endedDown) {
        gameState->greenOffset += 1;
      }
      if (controller->moveDown.endedDown) {
        gameState->greenOffset -= 1;
      }
    }
    gameState->playerX += (int)(4.0f * controller->stickAverageX);
    gameState->playerY -= (int)(4.0f * controller->stickAverageY);
    if (gameState->tJump > 0) {
      gameState->playerY += (int)(5.0f * sinf(0.5f * PI * gameState->tJump));
    }
    if (controller->actionDown.endedDown) {
      gameState->tJump = 4.0;
    }
    gameState->tJump -= 0.033f;
    if (controller->actionDown.endedDown) {
      gameState->greenOffset += 1;
    }
  }
  renderSomething(buffer, gameState->blueOffset, gameState->greenOffset);
  renderPlayer(buffer, gameState->playerX, gameState->playerY);

  renderPlayer(buffer, input->mouseX, input->mouseY);
  for (int buttonIndex = 0; buttonIndex < arrayCount(input->mouseButtons); ++buttonIndex) {
    if (input->mouseButtons[buttonIndex].endedDown) {
      renderPlayer(buffer, 10 + 20 * buttonIndex, 10);
    }
  }
}

extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples) {
  game_state *gameState = (game_state *)memory->permanentStorage;
  gameOutputSound(soundBuffer, gameState);
}