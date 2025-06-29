
########################################################################
# VGA 1111 on breadboard with PS/2 keyboard
########################################################################

set(picomputer_vga_defines
  PICO_CORE1_STACK_SIZE=0x800
  # Tell FATFS we want to find files
  FF_USE_FIND=1
  # Configure the SPI sd card to use SPI0
  #SDCARD_PIO=pio1
  #SDCARD_PIO_SM=1
  SDCARD_PIN_SPI0_CS=5
  SDCARD_PIN_SPI0_SCK=2
  SDCARD_PIN_SPI0_MOSI=3
  SDCARD_PIN_SPI0_MISO=4
  # Use stdio
  USE_STDIO
)

set(zxspectrum_vga_640x480x60Hz_defines
  # Set the resolution of the menu screen
  PCS_COLS=80
  PCS_ROWS=30
  # Set the VGA timing
  VGA_MODE=vga_mode_640x240_60
  # The display resolution
  DISPLAY_WIDTH_PIXELS=640
  DISPLAY_HEIGHT_PIXELS=480
  # The default interrupt source
  ZX_SPECTRUM_INT_SRC=SyncToCpu
  # TODO work what this is
  PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=700
)

set(zxspectrum_vga_720x576x50Hz_defines
  # Set the resolution of the menu screen
  PCS_COLS=90
  PCS_ROWS=30
  SZ_FRAME_X=5
  # Set the VGA timing
  VGA_MODE=vga_mode_720x288_50
  # The display resolution
  DISPLAY_WIDTH_PIXELS=720
  DISPLAY_HEIGHT_PIXELS=576
  # The default interrupt source
  ZX_SPECTRUM_INT_SRC=SyncToDisplay
  # TODO work what this is
  PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=800
)

set(zxspectrum_vga_CVBSx50Hz_defines
  # Set the resolution of the menu screen
  PCS_COLS=80
  PCS_ROWS=30
  SZ_FRAME_X=5
  # Set the VGA timing
  VGA_MODE=cvbs_mode_50
  # different pixel clock variants, pick one
  #CVBS_13_5MHZ=1
  CVBS_12MHZ=1
  # The display resolution
  DISPLAY_WIDTH_PIXELS=640
  DISPLAY_HEIGHT_PIXELS=576 # halved to 288
  # The default interrupt source
  ZX_SPECTRUM_INT_SRC=SyncToDisplay
  # TODO work what this is
  PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=800
)


foreach(target
  ZxSpectrum4PinAudioVga1111Ps2_640x480x60Hz
  ZxSpectrum4PinAudioVga1111Ps2_720x576x50Hz
  ZxSpectrum4PinAudioVga222Ps2_640x480x60Hz
  ZxSpectrum4PinAudioVga222Ps2_720x576x50Hz
  ZxSpectrumI2SAudioVga1111Ps2_CVBSx50Hz
)
  add_executable(${target}
    ${zxspectrum_common_src}
    ${zxspectrum_scanvideo_src}
    main.cpp
  )

  target_link_libraries(${target}
    ${zxspectrum_common_libs}
    pico_emu_utils
    pico_scanvideo_dpi
    tinyusb_host
    tinyusb_board
    hardware_pio
    hardware_pwm
    pico_scanvideo_menu
  )

  zxspectrum_generate_pio_headers(${target})

  pico_enable_stdio_usb(${target} 0)
  pico_enable_stdio_uart(${target} 1)

  # create map/bin/hex file etc.
  pico_add_extra_outputs(${target})

endforeach()

set(ZxSpectrum4PinAudioVga1111Ps2_defines
  # VGA 1111
  VGA_ENC_RGBY_1111
  PICO_SCANVIDEO_COLOR_PIN_COUNT=4u
  PICO_SCANVIDEO_DPI_PIXEL_RCOUNT=2u
  PICO_SCANVIDEO_DPI_PIXEL_GCOUNT=1u
  PICO_SCANVIDEO_DPI_PIXEL_BCOUNT=1u
  PICO_SCANVIDEO_DPI_PIXEL_RSHIFT=2u
  PICO_SCANVIDEO_DPI_PIXEL_GSHIFT=1u
  PICO_SCANVIDEO_DPI_PIXEL_BSHIFT=0u
  PICO_SCANVIDEO_COLOR_PIN_BASE=12u

  # 4 pin audio
  BZR_PIN=21
  EAR_PIN=11
  AY8912_A_PIN=20
  AY8912_B_PIN=26
  AY8912_C_PIN=27
  # Enable the PS/2 keyboard
  USE_PS2_KBD
)

set(ZxSpectrumI2SAudioVga1111Ps2_defines
  # VGA 1111
  VGA_ENC_RGBY_1111
  PICO_SCANVIDEO_COLOR_PIN_COUNT=4u
  PICO_SCANVIDEO_DPI_PIXEL_RCOUNT=2u
  PICO_SCANVIDEO_DPI_PIXEL_GCOUNT=1u
  PICO_SCANVIDEO_DPI_PIXEL_BCOUNT=1u
  PICO_SCANVIDEO_DPI_PIXEL_RSHIFT=2u
  PICO_SCANVIDEO_DPI_PIXEL_GSHIFT=1u
  PICO_SCANVIDEO_DPI_PIXEL_BSHIFT=0u
  PICO_SCANVIDEO_COLOR_PIN_BASE=12u

  # Configure the I2S audio, pio1 SM ?
  PICO_AUDIO_I2S=1
  PICO_AUDIO_I2S_PIO=pio1
  PICO_AUDIO_I2S_PIO_FUNC=GPIO_FUNC_PIO1
  PICO_AUDIO_I2S_DATA=26
  PICO_AUDIO_I2S_BCLK=27

  # Enable the PS/2 keyboard
  USE_PS2_KBD
)

target_compile_definitions(ZxSpectrum4PinAudioVga1111Ps2_720x576x50Hz PRIVATE
  ${picomputer_vga_defines}
  ${ZxSpectrum4PinAudioVga1111Ps2_defines}
  ${zxspectrum_vga_720x576x50Hz_defines}
)

target_compile_definitions(ZxSpectrum4PinAudioVga1111Ps2_640x480x60Hz PRIVATE
  ${picomputer_vga_defines}
  ${ZxSpectrum4PinAudioVga1111Ps2_defines}
  ${zxspectrum_vga_640x480x60Hz_defines}
)

target_compile_definitions(ZxSpectrumI2SAudioVga1111Ps2_CVBSx50Hz PRIVATE
  ${picomputer_vga_defines}
  ${ZxSpectrumI2SAudioVga1111Ps2_defines}
  ${zxspectrum_vga_CVBSx50Hz_defines}
)

########################################################################
# VGA 222 on breadboard with PS/2 keyboard
########################################################################

set(ZxSpectrum4PinAudioVga222Ps2_defines
  ${picomputer_vga_defines}
  #VGA 222
  VGA_ENC_RGB_222
  VGA_GPIO_SYNC=18
  VGA_GPIO_OUTNUM=6
  # 4 pin audio
  BZR_PIN=21
  AY8912_A_PIN=20
  AY8912_B_PIN=26
  AY8912_C_PIN=27
  # Enable the PS/2 keyboard
  USE_PS2_KBD
)

target_compile_definitions(ZxSpectrum4PinAudioVga222Ps2_720x576x50Hz PRIVATE
  ${ZxSpectrum4PinAudioVga222Ps2_defines}
  ${zxspectrum_vga_720x576x50Hz_defines}
)

target_compile_definitions(ZxSpectrum4PinAudioVga222Ps2_640x480x60Hz PRIVATE
  ${ZxSpectrum4PinAudioVga222Ps2_defines}
  ${zxspectrum_vga_640x480x60Hz_defines}
)

#
# Inital attempt at PicoMiteVGA support:-
#
# Stuff to do:
#
# 1) May need to move out from here
# 2) May need code modifications for joystick handling
# 3) PIN definitions not updated
# 6) RGBY is probably the wrong order
# 7) c-sync vs h-sync and v-sync
# 8) May be better off basing this on the Murulator build
#
set(ZxSpectrumPicoMiteVGA_defines
  # Configure the SPI sd card to use SPI0
  SDCARD_PIN_SPI0_CS=10
  SDCARD_PIN_SPI0_SCK=12
  SDCARD_PIN_SPI0_MOSI=11
  SDCARD_PIN_SPI0_MISO=13
  # VGA 1111
  VGA_ENC_BGYR_1111
  PICO_SCANVIDEO_COLOR_PIN_COUNT=4u
  PICO_SCANVIDEO_DPI_PIXEL_RCOUNT=2u
  PICO_SCANVIDEO_DPI_PIXEL_GCOUNT=1u
  PICO_SCANVIDEO_DPI_PIXEL_BCOUNT=1u
  PICO_SCANVIDEO_DPI_PIXEL_RSHIFT=2u
  PICO_SCANVIDEO_DPI_PIXEL_GSHIFT=1u
  PICO_SCANVIDEO_DPI_PIXEL_BSHIFT=0u
  PICO_SCANVIDEO_COLOR_PIN_BASE=18u
  # 4 pin audio
  BZR_PIN=6
  SPK_PIN=7
  # Enable the PS/2 keyboard
  USE_PS2_KBD
  PS2KBD_CLK0_DAT
  # Use stdio
  USE_STDIO
)

foreach(target
  ZxSpectrumPicoMiteVGA_640x480x60Hz
  ZxSpectrumPicoMiteVGA_720x576x50Hz
  ZX-MURMULATOR_VGA_PWM_AUDIO_640x480x60Hz
  ZX-MURMULATOR_VGA_PWM_AUDIO_720x576x50Hz
  ZX-MURMULATOR_VGA_I2S_AUDIO_640x480x60Hz
  ZX-MURMULATOR_VGA_I2S_AUDIO_720x576x50Hz
  ZX-MURMULATOR2_VGA_PWM_AUDIO_640x480x60Hz
  ZX-MURMULATOR2_VGA_PWM_AUDIO_720x576x50Hz
  ZX-MURMULATOR2_VGA_I2S_AUDIO_640x480x60Hz
  ZX-MURMULATOR2_VGA_I2S_AUDIO_720x576x50Hz
)

  add_executable(${target}
    ${zxspectrum_common_src}
    ${zxspectrum_rgb_src}
    ${zxspectrum_scanvideo_src}
    main.cpp
  )

  target_link_libraries(${target}
    ${zxspectrum_common_libs}
    pico_emu_utils
    pico_scanvideo_dpi
    pico_scanvideo_menu
    pico_nespad
    tinyusb_host
    tinyusb_board
    hardware_pio
    hardware_pwm
  )

  zxspectrum_generate_pio_headers(${target})

  pico_enable_stdio_usb(${target} 0)

  # create map/bin/hex file etc.
  pico_add_extra_outputs(${target})

endforeach()

foreach(target
  ZxSpectrumPicoMiteVGA_640x480x60Hz
  ZxSpectrumPicoMiteVGA_720x576x50Hz
)

  pico_enable_stdio_uart(${target} 1)

endforeach()

target_compile_definitions(ZxSpectrumPicoMiteVGA_720x576x50Hz PRIVATE
  ${ZxSpectrumPicoMiteVGA_defines}
  ${zxspectrum_vga_720x576x50Hz_defines}
)

target_compile_definitions(ZxSpectrumPicoMiteVGA_640x480x60Hz PRIVATE
  ${ZxSpectrumPicoMiteVGA_defines}
  ${zxspectrum_vga_640x480x60Hz_defines}
)

#========================================================
# ZX MURMULATOR
#========================================================
set(ZX-MURMULATOR_VGA_defines
  ZX_PLATFORM="MURMULATOR"
  # Configure the SPI sd card to use SPI0
  SDCARD_PIN_SPI0_CS=5
  SDCARD_PIN_SPI0_SCK=2
  SDCARD_PIN_SPI0_MOSI=3
  SDCARD_PIN_SPI0_MISO=4
  #VGA 222 vith VSYNC
  VGA_ENC_RGB_222
  VGA_VSYNC
  PICO_SCANVIDEO_COLOR_PIN_COUNT=6u
  PICO_SCANVIDEO_DPI_PIXEL_RCOUNT=2u
  PICO_SCANVIDEO_DPI_PIXEL_GCOUNT=2u
  PICO_SCANVIDEO_DPI_PIXEL_BCOUNT=3u
  PICO_SCANVIDEO_DPI_PIXEL_RSHIFT=4u
  PICO_SCANVIDEO_DPI_PIXEL_GSHIFT=2u
  PICO_SCANVIDEO_DPI_PIXEL_BSHIFT=0u
  PICO_SCANVIDEO_COLOR_PIN_BASE=6u
  # Use the alternate volume map
  AY8912_ALT_VOL_MAP
  # Tape audio in
  EAR_PIN=22
  # Enable the PS/2 keyboard
  USE_PS2_KBD
  PS2KBD_CLK0_DAT
  # Pins for dual NES joypads, pio1 SM ?
  NESPAD_DATA_PIN_BASE=16
  NESPAD_CLK_PIN_BASE=14
  NESPAD_PIO=pio1
  NESPAD_IRQ=PIO1_IRQ_0
  NESPAD_ENABLE
)

set(ZX-MURMULATOR_VGA_PWM_Audio_defines
  # Speaker pin audio
  SPK_PIN=28
  # Buzzer audio out
  # BZR_PIN=28
  # STEREO AY audio
  AY8912_ABC_STEREO
  # Left chanel audio out
  AY8912_A_PIN=27
  # Right chanel audio out
  AY8912_C_PIN=26
)

set (ZX-MURMULATOR_VGA_I2S_Audio_defines
  # Configure the I2S audio, pio1 SM ?
  PICO_AUDIO_I2S=1
  PICO_AUDIO_I2S_PIO=pio1
  PICO_AUDIO_I2S_PIO_FUNC=GPIO_FUNC_PIO1
  PICO_AUDIO_I2S_DATA=26
  PICO_AUDIO_I2S_BCLK=27
)

foreach(target
  ZX-MURMULATOR_VGA_PWM_AUDIO_640x480x60Hz
  ZX-MURMULATOR_VGA_PWM_AUDIO_720x576x50Hz
  ZX-MURMULATOR_VGA_I2S_AUDIO_640x480x60Hz
  ZX-MURMULATOR_VGA_I2S_AUDIO_720x576x50Hz
)

  pico_enable_stdio_uart(${target} 0)

endforeach()

target_compile_definitions(ZX-MURMULATOR_VGA_PWM_AUDIO_640x480x60Hz PRIVATE
  ${ZX-MURMULATOR_VGA_defines}
  ${ZX-MURMULATOR_VGA_PWM_Audio_defines}
  ${zxspectrum_vga_640x480x60Hz_defines}
)

target_compile_definitions(ZX-MURMULATOR_VGA_PWM_AUDIO_720x576x50Hz PRIVATE
  ${ZX-MURMULATOR_VGA_defines}
  ${ZX-MURMULATOR_VGA_PWM_Audio_defines}
  ${zxspectrum_vga_720x576x50Hz_defines}
)

target_compile_definitions(ZX-MURMULATOR_VGA_I2S_AUDIO_640x480x60Hz PRIVATE
  ${ZX-MURMULATOR_VGA_defines}
  ${ZX-MURMULATOR_VGA_I2S_Audio_defines}
  ${zxspectrum_vga_640x480x60Hz_defines}
)

target_compile_definitions(ZX-MURMULATOR_VGA_I2S_AUDIO_720x576x50Hz PRIVATE
  ${ZX-MURMULATOR_VGA_defines}
  ${ZX-MURMULATOR_VGA_I2S_Audio_defines}
  ${zxspectrum_vga_720x576x50Hz_defines}
)

#========================================================
# ZX MURMULATOR2
#========================================================
set(ZX-MURMULATOR2_VGA_defines
  ZX_PLATFORM="MURMULATOR2"
  # Configure the SPI sd card to use SPI0
  SDCARD_PIN_SPI0_CS=5
  SDCARD_PIN_SPI0_SCK=6
  SDCARD_PIN_SPI0_MOSI=7
  SDCARD_PIN_SPI0_MISO=4
  #VGA 222 vith VSYNC
  VGA_ENC_RGB_222
  VGA_VSYNC
  PICO_SCANVIDEO_COLOR_PIN_COUNT=6u
  PICO_SCANVIDEO_DPI_PIXEL_RCOUNT=2u
  PICO_SCANVIDEO_DPI_PIXEL_GCOUNT=2u
  PICO_SCANVIDEO_DPI_PIXEL_BCOUNT=3u
  PICO_SCANVIDEO_DPI_PIXEL_RSHIFT=4u
  PICO_SCANVIDEO_DPI_PIXEL_GSHIFT=2u
  PICO_SCANVIDEO_DPI_PIXEL_BSHIFT=0u
  PICO_SCANVIDEO_COLOR_PIN_BASE=12u
  # Use the alternate volume map
  AY8912_ALT_VOL_MAP
  # Tape audio in
  EAR_PIN=22
  # Enable the PS/2 keyboard
  USE_PS2_KBD
  PS2KBD_CLK2_DAT
  # Pins for dual NES joypads, pio1 SM ?
  NESPAD_DATA_PIN_BASE=26
  NESPAD_CLK_PIN_BASE=20
  NESPAD_PIO=pio1
  NESPAD_IRQ=PIO1_IRQ_0
  NESPAD_ENABLE
)

set(ZX-MURMULATOR2_VGA_PWM_Audio_defines
  #
  # This could be ABC output on GPIO 10, 9, 11 but the sofware FIFO is not working on the RP2350
  # For now I will output mono PWM using a PIO block on GPIO 10
  #
  PICO_AUDIO_PWM_PIO=pio1
  PICO_PIO_PWM_AUDIO
  SPK_PIN=9
)

set (ZX-MURMULATOR2_VGA_I2S_Audio_defines
  # Configure the I2S audio, pio1 SM ?
  PICO_AUDIO_I2S=1
  PICO_AUDIO_I2S_PIO=pio1
  PICO_AUDIO_I2S_PIO_FUNC=GPIO_FUNC_PIO1
  PICO_AUDIO_I2S_DATA=9
  PICO_AUDIO_I2S_BCLK=10
)

foreach(target
  ZX-MURMULATOR2_VGA_PWM_AUDIO_640x480x60Hz
  ZX-MURMULATOR2_VGA_PWM_AUDIO_720x576x50Hz
  ZX-MURMULATOR2_VGA_I2S_AUDIO_640x480x60Hz
  ZX-MURMULATOR2_VGA_I2S_AUDIO_720x576x50Hz
)
  pico_enable_stdio_uart(${target} 1)

endforeach()

target_compile_definitions(ZX-MURMULATOR2_VGA_PWM_AUDIO_640x480x60Hz PRIVATE
  ${ZX-MURMULATOR2_VGA_defines}
  ${ZX-MURMULATOR2_VGA_PWM_Audio_defines}
  ${zxspectrum_vga_640x480x60Hz_defines}
  # Speaker pin audio (@250MHz 250000000 / (1000 * 3 * 2))
  PICO_PWM_AUDIO_FREQ=41666
)

target_compile_definitions(ZX-MURMULATOR2_VGA_PWM_AUDIO_720x576x50Hz PRIVATE
  ${ZX-MURMULATOR2_VGA_defines}
  ${ZX-MURMULATOR2_VGA_PWM_Audio_defines}
  ${zxspectrum_vga_720x576x50Hz_defines}
  # Speaker pin audio (@270MHz 270000000 / (1000 * 3 * 2))
  PICO_PWM_AUDIO_FREQ=45000
)

target_compile_definitions(ZX-MURMULATOR2_VGA_I2S_AUDIO_640x480x60Hz PRIVATE
  ${ZX-MURMULATOR2_VGA_defines}
  ${ZX-MURMULATOR2_VGA_I2S_Audio_defines}
  ${zxspectrum_vga_640x480x60Hz_defines}
)

target_compile_definitions(ZX-MURMULATOR2_VGA_I2S_AUDIO_720x576x50Hz PRIVATE
  ${ZX-MURMULATOR2_VGA_defines}
  ${ZX-MURMULATOR2_VGA_I2S_Audio_defines}
  ${zxspectrum_vga_720x576x50Hz_defines}
)

