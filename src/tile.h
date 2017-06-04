#if !defined(HANDMADE_TILE_H)

struct tile_map_position {
  // These are fixed point tile locations.  The high
  // bits are the tile chunk index, and the low bits are the tile
  // index in the chunk.
  uint32_t absTileX;
  uint32_t absTileY;
  uint32_t absTileZ;

  // Should these be from the center of a tile?
  // Rename to offset X and Y
  float_t tileRelX;
  float_t tileRelY;
};

struct tile_chunk_position {
  uint32_t tileChunkX;
  uint32_t tileChunkY;
  uint32_t tileChunkZ;

  uint32_t relTileX;
  uint32_t relTileY;
};

struct tile_chunk {
  uint32_t *tiles;
};

struct tile_map {
  uint32_t chunkShift;
  uint32_t chunkMask;
  uint32_t chunkDim;

  float_t tileSideInMeters;

  uint32_t tileChunkCountX;
  uint32_t tileChunkCountY;
  uint32_t tileChunkCountZ;
  tile_chunk *tileChunks;
};

#define HANDMADE_TILE_H
#endif
