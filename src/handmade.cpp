#include "handmade.h"
#include "random.h"
#include "tile.cpp"

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

internal void DrawRectangle(game_offscreen_buffer *buffer, float_t realminX, float_t realMinY, float_t realMaxX,
                            float_t realMaxY, float_t R, float_t G, float_t B) {
  int32_t minX = RoundFloatToInt32(realminX);
  int32_t minY = RoundFloatToInt32(realMinY);
  int32_t maxX = RoundFloatToInt32(realMaxX);
  int32_t maxY = RoundFloatToInt32(realMaxY);

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

  uint32_t color = ((RoundFloatToUInt32(R * 255.0f) << 16) | (RoundFloatToUInt32(G * 255.0f) << 8) |
                    (RoundFloatToUInt32(B * 255.0f) << 0));

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

  float_t playerHeight = 1.4f;
  float_t playerWidth = 0.75f * playerHeight;
  game_state *gameState = (game_state *)memory->permanentStorage;
  if (!memory->isInitialized) {
    gameState->playerP.absTileX = 1;
    gameState->playerP.absTileY = 3;
    gameState->playerP.tileRelX = 5.0f;
    gameState->playerP.tileRelY = 5.0f;

    initializeArena(&gameState->worldArena, memory->permanentStorageSize - sizeof(game_state),
                    (uint8_t *)memory->permanentStorage + sizeof(game_state));

    gameState->world = pushStruct(&gameState->worldArena, world);
    world *world = gameState->world;
    world->tileMap = pushStruct(&gameState->worldArena, tile_map);

    tile_map *tileMap = world->tileMap;

    tileMap->chunkShift = 4;
    tileMap->chunkMask = (1 << tileMap->chunkShift) - 1;
    tileMap->chunkDim = (1 << tileMap->chunkShift);

    tileMap->tileChunkCountX = 128;
    tileMap->tileChunkCountY = 128;
    tileMap->tileChunkCountZ = 2;
    tileMap->tileChunks =
        pushArray(&gameState->worldArena,
                  tileMap->tileChunkCountX * tileMap->tileChunkCountY * tileMap->tileChunkCountZ, tile_chunk);

    tileMap->tileSideInMeters = 1.4f;

    uint32_t randomNumberIndex = 0;
    uint32_t tilesPerWidth = 17;
    uint32_t tilesPerHeight = 9;
    uint32_t screenX = 0;
    uint32_t screenY = 0;
    uint32_t absTileZ = 0;

    bool32 doorLeft = false;
    bool32 doorRight = false;
    bool32 doorTop = false;
    bool32 doorBottom = false;
    bool32 doorUp = false;
    bool32 doorDown = false;
    for (uint32_t screenIndex = 0; screenIndex < 100; ++screenIndex) {
      assert(randomNumberIndex < arrayCount(randomNumberTable));

      uint32_t randomChoice;
      if (doorUp || doorDown) {
        randomChoice = randomNumberTable[randomNumberIndex++] % 2;
      } else {
        randomChoice = randomNumberTable[randomNumberIndex++] % 3;
      }

      if (randomChoice == 2) {
        if (absTileZ == 0) {
          doorUp = true;
        } else {
          doorDown = true;
        }
      } else if (randomChoice == 1) {
        doorRight = true;
      } else {
        doorTop = true;
      }

      for (uint32_t TileY = 0; TileY < tilesPerHeight; ++TileY) {
        for (uint32_t TileX = 0; TileX < tilesPerWidth; ++TileX) {
          uint32_t absTileX = screenX * tilesPerWidth + TileX;
          uint32_t absTileY = screenY * tilesPerHeight + TileY;

          uint32_t TileValue = 1;
          if ((TileX == 0) && (!doorLeft || (TileY != (tilesPerHeight / 2)))) {
            TileValue = 2;
          }

          if ((TileX == (tilesPerWidth - 1)) && (!doorRight || (TileY != (tilesPerHeight / 2)))) {
            TileValue = 2;
          }

          if ((TileY == 0) && (!doorBottom || (TileX != (tilesPerWidth / 2)))) {
            TileValue = 2;
          }

          if ((TileY == (tilesPerHeight - 1)) && (!doorTop || (TileX != (tilesPerWidth / 2)))) {
            TileValue = 2;
          }

          if ((TileX == 10) && (TileY == 6)) {
            if (doorUp) {
              TileValue = 3;
            }

            if (doorDown) {
              TileValue = 4;
            }
          }

          setTileValue(&gameState->worldArena, world->tileMap, absTileX, absTileY, absTileZ, TileValue);
        }
      }

      doorLeft = doorRight;
      doorBottom = doorTop;

      if (doorUp) {
        doorDown = true;
        doorUp = false;
      } else if (doorDown) {
        doorUp = true;
        doorDown = false;
      } else {
        doorUp = false;
        doorDown = false;
      }

      doorRight = false;
      doorTop = false;

      if (randomChoice == 2) {
        if (absTileZ == 0) {
          absTileZ = 1;
        } else {
          absTileZ = 0;
        }
      } else if (randomChoice == 1) {
        screenX += 1;
      } else {
        screenY += 1;
      }
    }

    memory->isInitialized = true;
  }

  world *world = gameState->world;
  tile_map *tileMap = world->tileMap;

  int32_t tileSideInPixels = 60;
  float_t metersToPixels = (float_t)tileSideInPixels / (float_t)tileMap->tileSideInMeters;

  float_t lowerLeftX = -(float_t)tileSideInPixels / 2;
  float_t lowerLeftY = (float_t)buffer->height;

  for (int controllerIndex = 0; controllerIndex < arrayCount(input->controllers); ++controllerIndex) {
    game_controller_input *controller = getController(input, controllerIndex);
    if (controller->isAnalog) {
    } else {
      float_t dPlayerX = 0.0f;
      float_t dPlayerY = 0.0f;

      if (controller->moveUp.endedDown) {
        dPlayerY = 1.0f;
      }
      if (controller->moveDown.endedDown) {
        dPlayerY = -1.0f;
      }
      if (controller->moveLeft.endedDown) {
        dPlayerX = -1.0f;
      }
      if (controller->moveRight.endedDown) {
        dPlayerX = 1.0f;
      }
      float_t playerSpeed = 2.0f;
      if (controller->actionUp.endedDown) {
        playerSpeed = 10.0f;
      }
      dPlayerX *= playerSpeed;
      dPlayerY *= playerSpeed;

      tile_map_position newPlayerP = gameState->playerP;
      newPlayerP.tileRelX += input->dtForFrame * dPlayerX;
      newPlayerP.tileRelY += input->dtForFrame * dPlayerY;
      newPlayerP = recanonicalizePosition(tileMap, newPlayerP);

      tile_map_position playerLeft = newPlayerP;
      playerLeft.tileRelX -= 0.5f * playerWidth;
      playerLeft = recanonicalizePosition(tileMap, playerLeft);

      tile_map_position playerRight = newPlayerP;
      playerRight.tileRelX += 0.5f * playerWidth;
      playerRight = recanonicalizePosition(tileMap, playerRight);

      if (isTileMapPointEmpty(tileMap, newPlayerP) && isTileMapPointEmpty(tileMap, playerRight) &&
          isTileMapPointEmpty(tileMap, playerLeft)) {
        gameState->playerP = newPlayerP;
      }
    }
  }

  float_t upperLeftX = -30;
  float_t upperLeftY = 0;
  float_t tileWidth = 60;
  float_t tileHeight = 60;

  DrawRectangle(buffer, 0.0f, 0.0f, (float_t)buffer->width, (float_t)buffer->height, 1.0f, 0.0f, 0.1f);

  float_t screenCenterX = 0.5f * (float_t)buffer->width;
  float_t screenCenterY = 0.5f * (float_t)buffer->height;
  for (int relativeRow = -10; relativeRow < 10; ++relativeRow) {
    for (int relativeColumn = -20; relativeColumn < 20; ++relativeColumn) {
      uint32_t column = gameState->playerP.absTileX + relativeColumn;
      uint32_t row = gameState->playerP.absTileY + relativeRow;
      uint32_t tileId = getTileValue(tileMap, column, row, gameState->playerP.absTileZ);
      if (tileId > 0) {
        float_t gray = 0.5f;
        if (tileId > 2) {
          gray = 0.25f;
        }
        if (tileId == 2) {
          gray = 1.0f;
        }
        if ((column == gameState->playerP.absTileX) && (row == gameState->playerP.absTileY)) {
          gray = 0.0f;
        }

        float_t cenX =
            screenCenterX - metersToPixels * gameState->playerP.tileRelX + ((float_t)relativeColumn) * tileSideInPixels;
        float_t cenY =
            screenCenterY + metersToPixels * gameState->playerP.tileRelY - ((float_t)relativeRow) * tileSideInPixels;
        float_t minX = cenX - 0.5f * tileSideInPixels;
        float_t minY = cenY - 0.5f * tileSideInPixels;
        float_t maxX = cenX + 0.5f * tileSideInPixels;
        float_t maxY = cenY + 0.5f * tileSideInPixels;
        DrawRectangle(buffer, minX, minY, maxX, maxY, gray, gray, gray);
      }
    }
  }

  float_t playerR = 1.0f;
  float_t playerG = 1.0f;
  float_t playerB = 0.0f;
  float_t playerLeft = screenCenterX - 0.5f * metersToPixels * playerWidth;
  float_t playerTop = screenCenterY - metersToPixels * playerHeight;
  DrawRectangle(buffer, playerLeft, playerTop, playerLeft + metersToPixels * playerWidth,
                playerTop + metersToPixels * playerHeight, playerR, playerG, playerB);
}

extern "C" GAME_GET_SOUND_SAMPLES(gameGetSoundSamples) {
  game_state *gameState = (game_state *)memory->permanentStorage;
  gameOutputSound(soundBuffer, gameState, 400);
}