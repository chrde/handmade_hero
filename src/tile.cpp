inline void recanonicalizeCoord(tile_map *tileMap, uint32_t *tile, float_t *tileRel) {
  int32_t offset = RoundFloatToInt32(*tileRel / tileMap->tileSideInMeters);
  *tile += offset;
  *tileRel -= offset * tileMap->tileSideInMeters;

  assert(*tileRel >= -0.5f * tileMap->tileSideInMeters);
  assert(*tileRel <= 0.5f * tileMap->tileSideInMeters);
}

inline tile_map_position recanonicalizePosition(tile_map *tileMap, tile_map_position pos) {
  tile_map_position result = pos;

  recanonicalizeCoord(tileMap, &result.absTileX, &result.tileRelX);
  recanonicalizeCoord(tileMap, &result.absTileY, &result.tileRelY);

  return result;
}

inline tile_chunk *getTileChunk(tile_map *tileMap, uint32_t tileChunkX, uint32_t tileChunkY, uint32_t tileChunkZ) {
  tile_chunk *tileChunk = 0;

  if ((tileChunkX >= 0) && (tileChunkX < tileMap->tileChunkCountX) && (tileChunkY >= 0) &&
      (tileChunkY < tileMap->tileChunkCountY) && (tileChunkZ >= 0) && (tileChunkZ < tileMap->tileChunkCountZ)) {
    tileChunk = &tileMap->tileChunks[tileChunkZ * tileMap->tileChunkCountY * tileMap->tileChunkCountX +
                                     tileChunkY * tileMap->tileChunkCountX + tileChunkX];
  }

  return tileChunk;
}

inline uint32_t getTileValueUnchecked(tile_map *tileMap, tile_chunk *tileChunk, uint32_t tileX, uint32_t tileY) {
  assert(tileChunk);
  assert(tileX < tileMap->chunkDim);
  assert(tileY < tileMap->chunkDim);

  uint32_t tileChunkValue = tileChunk->tiles[tileY * tileMap->chunkDim + tileX];
  return tileChunkValue;
}

inline void setTileValueUnchecked(tile_map *tileMap, tile_chunk *tileChunk, uint32_t tileX, uint32_t tileY,
                                  uint32_t TileValue) {
  assert(tileChunk);
  assert(tileX < tileMap->chunkDim);
  assert(tileY < tileMap->chunkDim);

  tileChunk->tiles[tileY * tileMap->chunkDim + tileX] = TileValue;
}

inline uint32_t getTileValue(tile_map *tileMap, tile_chunk *tileChunk, uint32_t testTileX, uint32_t testTileY) {
  uint32_t tileChunkValue = 0;

  if (tileChunk && tileChunk->tiles) {
    tileChunkValue = getTileValueUnchecked(tileMap, tileChunk, testTileX, testTileY);
  }

  return tileChunkValue;
}

inline void setTileValue(tile_map *tileMap, tile_chunk *tileChunk, uint32_t testTileX, uint32_t testTileY,
                         uint32_t TileValue) {
  if (tileChunk && tileChunk->tiles) {
    setTileValueUnchecked(tileMap, tileChunk, testTileX, testTileY, TileValue);
  }
}

inline tile_chunk_position getChunkPositionFor(tile_map *tileMap, uint32_t absTileX, uint32_t absTileY,
                                               uint32_t absTileZ) {
  tile_chunk_position result;

  result.tileChunkX = absTileX >> tileMap->chunkShift;
  result.tileChunkY = absTileY >> tileMap->chunkShift;
  result.tileChunkZ = absTileZ;
  result.relTileX = absTileX & tileMap->chunkMask;
  result.relTileY = absTileY & tileMap->chunkMask;

  return result;
}

internal uint32_t getTileValue(tile_map *tileMap, uint32_t absTileX, uint32_t absTileY, uint32_t absTileZ) {
  bool32 Empty = false;

  tile_chunk_position chunkPos = getChunkPositionFor(tileMap, absTileX, absTileY, absTileZ);
  tile_chunk *tileChunk = getTileChunk(tileMap, chunkPos.tileChunkX, chunkPos.tileChunkY, chunkPos.tileChunkZ);
  uint32_t tileChunkValue = getTileValue(tileMap, tileChunk, chunkPos.relTileX, chunkPos.relTileY);

  return tileChunkValue;
}

internal bool32 isTileMapPointEmpty(tile_map *tileMap, tile_map_position CanPos) {
  uint32_t tileChunkValue = getTileValue(tileMap, CanPos.absTileX, CanPos.absTileY, CanPos.absTileZ);
  bool32 Empty = (tileChunkValue == 1);

  return Empty;
}

internal void setTileValue(memory_arena *arena, tile_map *tileMap, uint32_t absTileX, uint32_t absTileY,
                           uint32_t absTileZ, uint32_t TileValue) {
  tile_chunk_position chunkPos = getChunkPositionFor(tileMap, absTileX, absTileY, absTileZ);
  tile_chunk *tileChunk = getTileChunk(tileMap, chunkPos.tileChunkX, chunkPos.tileChunkY, chunkPos.tileChunkZ);

  assert(tileChunk);
  if (!tileChunk->tiles) {
    uint32_t tileCount = tileMap->chunkDim * tileMap->chunkDim;
    tileChunk->tiles = pushArray(arena, tileCount, uint32_t);
    for (uint32_t tileIndex = 0; tileIndex < tileCount; ++tileIndex) {
      tileChunk->tiles[tileIndex] = 1;
    }
  }

  setTileValue(tileMap, tileChunk, chunkPos.relTileX, chunkPos.relTileY, TileValue);
}
