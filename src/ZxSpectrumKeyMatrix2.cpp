#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "class/hid/hid.h"
#include "ZxSpectrumKeyMatrix.h"

#define SAMPLES 2

#define ROW_PIN_1 9
#define ROW_PIN_2 1
#define ROW_PIN_3 8
#define ROW_PIN_4 5
#define ROW_PIN_5 4
#define ROW_PIN_6 15
#define ROW_PINS ROW_PIN_5,ROW_PIN_4,ROW_PIN_3,ROW_PIN_2,ROW_PIN_1,ROW_PIN_6
#define COL_PIN_DAT 17
#define COL_PIN_CLK 14
#define ROW_PIN_BIT_R(a,p,b) ((a >> (p - b)) & (1 << b))
#define ROW_PIN_BIT_L(a,p,b) ((a << (b - p)) & (1 << b))
#define ROW_PINS_JOIN(a) ( \
ROW_PIN_BIT_R(a, ROW_PIN_5,  0) | \
ROW_PIN_BIT_R(a, ROW_PIN_4,  1) | \
ROW_PIN_BIT_R(a, ROW_PIN_3,  2) | \
ROW_PIN_BIT_L(a, ROW_PIN_2,  3) | \
ROW_PIN_BIT_R(a, ROW_PIN_1,  4) | \
ROW_PIN_BIT_R(a, ROW_PIN_6,  5) \
)

#define COL_PIN_COUNT 8
#define ROW_PIN_COUNT 6
#define JOYSTICK_OFFSET 2

static uint8_t row_pins[] = {ROW_PINS};          // Row pins
static uint8_t rs[COL_PIN_COUNT][SAMPLES];       // Oversampled pins
static uint8_t rdb[COL_PIN_COUNT];               // Debounced pins
static hid_keyboard_report_t hr[2];              // Current and previous hid report
static uint8_t hri = 0;                          // Currenct hid report inde
static uint8_t kbi = 0;
static uint8_t menu = 0;

void __not_in_flash_func(zx_menu_mode)(bool m) {
  menu = m ? 1 : 0; 
}

bool zx_menu_mode() {
  return menu != 0;
}

static uint8_t kbits[6][COL_PIN_COUNT][ROW_PIN_COUNT] = {
  {
// Normal mappings + cursor SymShift = HID_KEY_ALT_RIGHT
/* 0 */ { HID_KEY_1,          HID_KEY_2,         HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_F10          /* 128 K */ },
/* 1 */ { HID_KEY_Q,          HID_KEY_W,         HID_KEY_E, HID_KEY_R, HID_KEY_T, HID_KEY_F9           /* 48 K  */ },
/* 2 */ { HID_KEY_A,          HID_KEY_S,         HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_F1           /* Menu  */ },
/* 3 */ { HID_KEY_0,          HID_KEY_9,         HID_KEY_8, HID_KEY_7, HID_KEY_6, HID_KEY_ARROW_DOWN   /* Down  */ },
/* 4 */ { HID_KEY_P,          HID_KEY_O,         HID_KEY_I, HID_KEY_U, HID_KEY_Y, HID_KEY_ARROW_RIGHT  /* Right */ },
/* 5 */ { HID_KEY_SHIFT_LEFT, HID_KEY_Z,         HID_KEY_X, HID_KEY_C, HID_KEY_V, HID_KEY_ARROW_UP     /* Up    */ },
/* 6 */ { HID_KEY_ENTER,      HID_KEY_L,         HID_KEY_K, HID_KEY_J, HID_KEY_H, HID_KEY_ARROW_LEFT   /* Left  */ },
/* 7 */ { HID_KEY_SPACE,      HID_KEY_ALT_RIGHT, HID_KEY_M, HID_KEY_N, HID_KEY_B, HID_KEY_0            /* Fire  */ }
  },
  {
// Shifted normal mappings + cursor
/* 0 */ { HID_KEY_1,          HID_KEY_2,         HID_KEY_3, HID_KEY_4, HID_KEY_5, 0                    /* 128 K */ },
/* 1 */ { HID_KEY_Q,          HID_KEY_W,         HID_KEY_E, HID_KEY_R, HID_KEY_T, 0                    /* 48 K  */ },
/* 2 */ { HID_KEY_A,          HID_KEY_S,         HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_F1           /* Menu  */ },
/* 3 */ { HID_KEY_0,          HID_KEY_9,         HID_KEY_8, HID_KEY_7, HID_KEY_6, HID_KEY_ARROW_DOWN   /* Down  */ },
/* 4 */ { HID_KEY_P,          HID_KEY_O,         HID_KEY_I, HID_KEY_U, HID_KEY_Y, HID_KEY_ARROW_RIGHT  /* Right */ },
/* 5 */ { HID_KEY_SHIFT_LEFT, HID_KEY_Z,         HID_KEY_X, HID_KEY_C, HID_KEY_V, HID_KEY_ARROW_UP     /* Up    */ },
/* 6 */ { HID_KEY_ENTER,      HID_KEY_L,         HID_KEY_K, HID_KEY_J, HID_KEY_H, HID_KEY_ARROW_LEFT   /* Left  */ },
/* 7 */ { HID_KEY_SPACE,      HID_KEY_ALT_RIGHT, HID_KEY_M, HID_KEY_N, HID_KEY_B, HID_KEY_ENTER        /* Fire  */ }
  },
  {
// Normal mappings + joystick
/* 0 */ { HID_KEY_1,          HID_KEY_2,         HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_F10          /* 128 K */ },
/* 1 */ { HID_KEY_Q,          HID_KEY_W,         HID_KEY_E, HID_KEY_R, HID_KEY_T, HID_KEY_F9           /* 48 K  */ },
/* 2 */ { HID_KEY_A,          HID_KEY_S,         HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_F1           /* Menu  */ },
/* 3 */ { HID_KEY_0,          HID_KEY_9,         HID_KEY_8, HID_KEY_7, HID_KEY_6, 0                    /* Down  */ },
/* 4 */ { HID_KEY_P,          HID_KEY_O,         HID_KEY_I, HID_KEY_U, HID_KEY_Y, 0                    /* Right */ },
/* 5 */ { HID_KEY_SHIFT_LEFT, HID_KEY_Z,         HID_KEY_X, HID_KEY_C, HID_KEY_V, 0                    /* Up    */ },
/* 6 */ { HID_KEY_ENTER,      HID_KEY_L,         HID_KEY_K, HID_KEY_J, HID_KEY_H, 0                    /* Left  */ },
/* 7 */ { HID_KEY_SPACE,      HID_KEY_ALT_RIGHT, HID_KEY_M, HID_KEY_N, HID_KEY_B, 0                    /* Fire  */ }
  },
  {
// Shifted normal mappings + joystick
/* 0 */ { HID_KEY_1,          HID_KEY_2,         HID_KEY_3, HID_KEY_4, HID_KEY_5, 0                    /* 128 K */ },
/* 1 */ { HID_KEY_Q,          HID_KEY_W,         HID_KEY_E, HID_KEY_R, HID_KEY_T, 0                    /* 48 K  */ },
/* 2 */ { HID_KEY_A,          HID_KEY_S,         HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_F1           /* Menu  */ },
/* 3 */ { HID_KEY_0,          HID_KEY_9,         HID_KEY_8, HID_KEY_7, HID_KEY_6, 0                    /* Down  */ },
/* 4 */ { HID_KEY_P,          HID_KEY_O,         HID_KEY_I, HID_KEY_U, HID_KEY_Y, 0                    /* Right */ },
/* 5 */ { HID_KEY_SHIFT_LEFT, HID_KEY_Z,         HID_KEY_X, HID_KEY_C, HID_KEY_V, 0                    /* Up    */ },
/* 6 */ { HID_KEY_ENTER,      HID_KEY_L,         HID_KEY_K, HID_KEY_J, HID_KEY_H, 0                    /* Left  */ },
/* 7 */ { HID_KEY_SPACE,      HID_KEY_ALT_RIGHT, HID_KEY_M, HID_KEY_N, HID_KEY_B, 0                    /* Fire  */ }
  },
  {
// Normal menu mappings
/* 0 */ { HID_KEY_1,          HID_KEY_2,         HID_KEY_3, HID_KEY_4, HID_KEY_5, 0                    /* 128 K */ },
/* 1 */ { HID_KEY_Q,          HID_KEY_W,         HID_KEY_E, HID_KEY_R, HID_KEY_T, 0                    /* 48 K  */ },
/* 2 */ { HID_KEY_A,          HID_KEY_S,         HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_F1           /* Menu  */ },
/* 3 */ { HID_KEY_0,          HID_KEY_9,         HID_KEY_8, HID_KEY_7, HID_KEY_6, HID_KEY_ARROW_DOWN   /* Down  */ },
/* 4 */ { HID_KEY_P,          HID_KEY_O,         HID_KEY_I, HID_KEY_U, HID_KEY_Y, HID_KEY_ARROW_RIGHT  /* Right */ },
/* 5 */ { HID_KEY_SHIFT_LEFT, HID_KEY_Z,         HID_KEY_X, HID_KEY_C, HID_KEY_V, HID_KEY_ARROW_UP     /* Up    */ },
/* 6 */ { HID_KEY_ENTER,      HID_KEY_L,         HID_KEY_K, HID_KEY_J, HID_KEY_H, HID_KEY_ARROW_LEFT   /* Left  */ },
/* 7 */ { HID_KEY_SPACE,      HID_KEY_ALT_RIGHT, HID_KEY_M, HID_KEY_N, HID_KEY_B, HID_KEY_ENTER        /* Fire  */ }
  },
  {
// Shifted menu mappings
/* 0 */ { HID_KEY_1,          HID_KEY_2,         HID_KEY_3, HID_KEY_4, HID_KEY_5, 0                    /* 128 K */ },
/* 1 */ { HID_KEY_Q,          HID_KEY_W,         HID_KEY_E, HID_KEY_R, HID_KEY_T, 0                    /* 48 K  */ },
/* 2 */ { HID_KEY_A,          HID_KEY_S,         HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_ESCAPE       /* Menu  */ },
/* 3 */ { HID_KEY_0,          HID_KEY_9,         HID_KEY_8, HID_KEY_7, HID_KEY_6, HID_KEY_ARROW_DOWN   /* Down  */ },
/* 4 */ { HID_KEY_P,          HID_KEY_O,         HID_KEY_I, HID_KEY_U, HID_KEY_Y, HID_KEY_ARROW_RIGHT  /* Right */ },
/* 5 */ { HID_KEY_SHIFT_LEFT, HID_KEY_Z,         HID_KEY_X, HID_KEY_C, HID_KEY_V, HID_KEY_ARROW_UP     /* Up    */ },
/* 6 */ { HID_KEY_ESCAPE,     HID_KEY_L,         HID_KEY_K, HID_KEY_J, HID_KEY_H, HID_KEY_ARROW_LEFT   /* Left  */ },
/* 7 */ { HID_KEY_SPACE,      HID_KEY_ALT_RIGHT, HID_KEY_M, HID_KEY_N, HID_KEY_B, HID_KEY_ESCAPE       /* Fire  */ }
  }
};

/**
 * Special key locations we need to keep track of
 */
// TODO these say _ROW but they should be _COL
#define KEY_UP_ROW 5
#define KEY_UP_BIT 0x20
#define KEY_DOWN_ROW 3
#define KEY_DOWN_BIT 0x20
#define KEY_LEFT_ROW 6
#define KEY_LEFT_BIT 0x20
#define KEY_RIGHT_ROW 4
#define KEY_RIGHT_BIT 0x20
#define KEY_FIRE_ROW 7
#define KEY_FIRE_BIT 0x20
#define KEY_CURSOR_ROW 1
#define KEY_CURSOR_BIT 0x20
#define KEY_KEMPSTON_ROW 0
#define KEY_KEMPSTON_BIT 0x20 
#define KEY_SHIFT_ROW 5
#define KEY_SHIFT_BIT 0x1

// TODO Check this and move to make
// #define LED_PIN 25

static uint8_t kempstonJoystick = 0;

static void zx_mode_joystick() {
  // Joystick mode
  kempstonJoystick = JOYSTICK_OFFSET;
  // gpio_put(LED_PIN, 0);
}

static void zx_mode_cursor() {
  // Cursor mode
  kempstonJoystick = 0;
  // gpio_put(LED_PIN, 1);
}

void zx_keyscan_init() {

   gpio_init(COL_PIN_DAT);
   gpio_set_dir(COL_PIN_DAT, GPIO_OUT);
   gpio_init(COL_PIN_CLK);
   gpio_set_dir(COL_PIN_CLK, GPIO_OUT);
   gpio_put(COL_PIN_DAT, 1);
   gpio_put(COL_PIN_CLK, 0);
   sleep_ms(1);

   for(int i = 0; i < COL_PIN_COUNT; ++i) {
     gpio_put(COL_PIN_CLK, 1);
     sleep_ms(1);
     gpio_put(COL_PIN_CLK, 0);
     sleep_ms(1);
   }

   gpio_put(COL_PIN_DAT, 0);
   sleep_ms(1);
   gpio_put(COL_PIN_CLK, 1);
   sleep_ms(1);

   for(int i = 0; i < ROW_PIN_COUNT; ++i) {
      gpio_init(row_pins[i]);
      gpio_set_dir(row_pins[i], GPIO_IN);
      gpio_pull_up(row_pins[i]);
   }
}

void zx_scan_matrix() {
  for( int i = 0; i < COL_PIN_COUNT; ++i) {
    sleep_ms(2);
    zx_keyscan_row();
  }
}

bool zx_fire_raw() {
  zx_scan_matrix();
  for(int si = 0; si < SAMPLES; ++si) {
    uint8_t s = rs[KEY_FIRE_ROW][si];
    if (s & KEY_FIRE_BIT) return true;  
  }
  return false;
}

uint8_t __not_in_flash_func(zx_kempston)() {
  // 000FUDLR
  return kempstonJoystick && !menu
  ? ((rdb[KEY_FIRE_ROW] & KEY_FIRE_BIT) >> 1)    // Kempston fire
  | ((rdb[KEY_LEFT_ROW] & KEY_LEFT_BIT) >> 4)    // Kempston left
  | ((rdb[KEY_UP_ROW] & KEY_UP_BIT) >> 2)        // Kempston up
  | ((rdb[KEY_RIGHT_ROW] & KEY_RIGHT_BIT) >> 5)  // Kempston right
  | ((rdb[KEY_DOWN_ROW] & KEY_DOWN_BIT) >> 3)    // Kempston down
  : 0;
}

void __not_in_flash_func(zx_keyscan_row)() {
  static uint32_t ri = 0; /* The column index */
  static uint32_t si = 0; /* The sample index */

  gpio_put(COL_PIN_CLK, 0);

  uint32_t a = ~gpio_get_all();

  uint32_t r = ROW_PINS_JOIN(a);
  rs[ri][si] = (uint8_t)r;
  
  
  if (++ri >= COL_PIN_COUNT) {
    ri = 0;
    if (++si >= SAMPLES) si = 0;
  }

  gpio_put(COL_PIN_DAT, ri == 0 ? 0 : 1);
  
  uint32_t om = 0;
  uint32_t am = (1 << ROW_PIN_COUNT) - 1;
  for(int si = 0; si < SAMPLES; ++si) {
    uint8_t s = rs[ri][si];
    om |= s; // bits 0 if no samples have the button pressed
    am &= s; // bits 1 if all samples have the button pressed
  }
  // only change key state if all samples on or off
  rdb[ri] = (am | rdb[ri]) & om; 
  
  gpio_put(COL_PIN_CLK, 1);
}

void __not_in_flash_func(zx_keyscan_get_hid_reports)(hid_keyboard_report_t const **curr, hid_keyboard_report_t const **prev) {

  // Key modifier (shift, alt, ctrl, etc.)
  static uint8_t modifier = 0;
  
  bool shift = rdb[KEY_SHIFT_ROW] & KEY_SHIFT_BIT;

  // Cursor mode 
  if (shift) {
    if (rdb[KEY_CURSOR_ROW] & KEY_CURSOR_BIT) { 
      zx_mode_cursor();
    }
    // Joystick mode
    if (rdb[KEY_KEMPSTON_ROW] & KEY_KEMPSTON_BIT) {
      zx_mode_joystick();
    }
  }

  kbi = (menu ? 4 : kempstonJoystick) + (shift ? 1 : 0 );

  // Build the current hid report
  uint32_t ki = 0;
  hid_keyboard_report_t *chr = &hr[hri & 1];
  chr->modifier = modifier;
  if (shift) chr->modifier |= 2;
  //if (ctrl) chr->modifier |= 1;
  for(int ri = 0; ri < COL_PIN_COUNT; ++ri) {
    uint8_t r = rdb[ri];
    uint32_t ci = 0;
    while(r) {
      if (r & 1) {
        if (ki >= sizeof(chr->keycode)) {
          // overflow
          ki = 0;
          while(ki < sizeof(chr->keycode)) chr->keycode[ki++] = 1;
          break;  
        }
        uint8_t kc = kbits[kbi][ri][ci];
        if (kc) chr->keycode[ki++] = kc;
      }
      ++ci;
      r >>= 1;
    }
  }
  while(ki < sizeof(chr->keycode)) chr->keycode[ki++] = 0;
  *curr = chr;
  hri++;
  *prev = &hr[hri & 1];
}
