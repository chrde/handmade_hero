#if !defined(HANDMADE_INTRINSICS_H)
#include <math.h>

inline int32_t RoundFloatToInt32(float_t x) {
  int32_t result = (int32_t)roundf(x);
  return result;
}

inline uint32_t RoundFloatToUInt32(float_t x) {
  uint32_t result = (uint32_t)roundf(x);
  return result;
}

inline int32_t FloorFloatToInt32(float_t x) {
  int32_t result = (int32_t)floorf(x);
  return result;
}

inline int32_t TruncateFloatToInt32(float_t x) {
  int32_t result = (int32_t)x;
  return result;
}

inline float_t Sin(float_t angle) {
  float_t result = sinf(angle);
  return result;
}

inline float_t Cos(float_t angle) {
  float_t result = cosf(angle);
  return result;
}

inline float_t ATan2(float_t y, float_t x) {
  float_t result = atan2f(y, x);
  return result;
}
#define HANDMADE_INTRINSICS_H
#endif
