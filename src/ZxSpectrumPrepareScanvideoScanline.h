#pragma once

#include "pico/stdlib.h"
#include "pico/scanvideo.h"
#include "ZxSpectrumAudioDriverEnum.h"

#ifdef __cplusplus
extern "C" {
#endif

void __not_in_flash_func(zx_prepare_scanvideo_scanline)(
  struct scanvideo_scanline_buffer *scanline_buffer, 
  uint32_t y, 
  uint32_t frame,
  uint8_t* screenPtr,
  uint8_t* attrPtr,
  uint8_t borderColor
);

void __not_in_flash_func(zx_prepare_scanvideo_blankline)(
  struct scanvideo_scanline_buffer *scanline_buffer
);

#ifdef __cplusplus
}
#endif
