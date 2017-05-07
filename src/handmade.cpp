#include "handmade.h"

internal void gameOutputSound(game_sound_output_buffer *soundBuffer, int toneHz) {
  local_persist float_t tSine;
  int16_t toneVolume = 3000;
  int wavePeriod = soundBuffer->samplesPerSecond / toneHz;

  int16_t *sample = soundBuffer->samples;
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
      uint8_t green = (uint8_t)(y + yOffset);
      uint8_t blue = (uint8_t)(x + xOffset);
      *pixel = ((green << 8) | blue);
      ++pixel;
    }
    row += buffer->pitch;
  }
}

internal void GameUpdateAndRender(game_memory *memory, game_offscreen_buffer *buffer,
                                  game_sound_output_buffer *soundBuffer, game_input *input) {
  assert(sizeof(game_state) <= memory->permanentStorageSize);
  game_state *gameState = (game_state *)memory->permanentStorage;
  if (!memory->isInitialized) {
    char *filename = __FILE__;
    debug_read_file_result file = DEBUGPlatformReadEntireFile(filename);
    if (file.contents) {
      DEBUGPlatformWriteEntireFile("C:/Users/chrde/github/handmade_hero/src/handmade_copy.cpp", file.contentsSize, file.contents);
      DEBUGPlatformFreeFileMemory(file.contents);
    }
    gameState->toneHz = 256 ;
    memory->isInitialized = true;
  }

for(int controllerIndex= 0; controllerIndex < arrayCount(input->controllers); ++controllerIndex){

  game_controller_input *controller = getController(input, controllerIndex);
  if (controller->isAnalog) {
    gameState->toneHz = 256 + (int)(128.0f * controller->stickAverageY);
    gameState->blueOffset += (int)(4.0f * controller->stickAverageX);
  } else {
    if(controller->moveLeft.endedDown){
      gameState->blueOffset +=1;
    }
    if(controller->moveRight.endedDown){
      gameState->blueOffset -=1;
    }
    if(controller->moveUp.endedDown){
      gameState->greenOffset +=1;
    }
    if(controller->moveDown.endedDown){
      gameState->greenOffset -=1;
    }
  }
  if (controller->actionDown.endedDown) {
    gameState->greenOffset += 1;
  }
}
  gameOutputSound(soundBuffer, gameState->toneHz);
  renderSomething(buffer, gameState->blueOffset, gameState->greenOffset);
}