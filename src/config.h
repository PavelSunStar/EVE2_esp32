#pragma once

#include "structures.h"

#define PIN_PD      1
#define PIN_CS      3
#define PIN_SCK     20
#define PIN_MOSI    5
#define PIN_MISO    4
#define PIN_IO2     32
#define PIN_IO3     33

inline constexpr Mode MODE640x480_60 = {
    24000000,  // реальный PCLK
    640,       // HSIZE
    768,       // HCYCLE
    112,       // HOFFSET
    0,         // HSYNC0
    64,        // HSYNC1
    480,       // VSIZE
    521,       // VCYCLE
    35,        // VOFFSET
    0,         // VSYNC0
    2,         // VSYNC1
    3          // PCLK divider: 72 / 3 = 24 MHz
};

inline constexpr Mode MODE640x480_57 = {24000000UL, 640, 800, 144, 0, 96, 480, 525, 35, 0, 2, 3};
inline constexpr Mode MODE640x400_70 = {24000000UL, 640, 763, 128, 0, 80, 400, 449, 35, 0, 2, 3};
inline constexpr Mode MODE640x350_70 = {24000000UL, 640, 763, 128, 0, 80, 350, 449, 60, 0, 2, 3};
inline constexpr Mode MODE720x400_70 = {36000000UL, 720, 1144, 216, 0, 108, 400, 449, 35, 0, 2, 2};
inline constexpr Mode MODE800x600_60 = {36000000UL, 800, 950, 130, 0, 80, 600, 632, 27, 0, 4, 2};
inline constexpr Mode MODE800x600_56 = {36000000UL, 800, 1024, 176, 0, 96, 600, 628, 23, 0, 4, 2};