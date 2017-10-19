#pragma once
#include "mp4errors.h"

#ifdef __cplusplus
namespace libmp4operations
{
extern "C"
{
#endif // __cplusplus

#define ROTATION_MATRIX_ELEMENTS  (9)

typedef enum 
{
  VIDEO_ROTATION_MODE_NO_ROTATION = 0,
  VIDEO_ROTATION_MODE_90_DEGREES_RIGHT,
  VIDEO_ROTATION_MODE_180_DEGREES,
  VIDEO_ROTATION_MODE_90_DEGREES_LEFT,
  VIDEO_ROTATION_MODE_ADOPT_ISO_FILE_SETTINGS,
  NUMBER_OF_SUPPORTED_VIDEO_ROTATION_MODES
} eVideoRotationMode;

#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

