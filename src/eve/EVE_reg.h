#pragma once

#include "reg_cmd.h"
#include "host_cmd.h"
#include "reg_dl.h"
#include "reg_defs.h"

#define _8BIT               8UL
#define _16BIT              16UL
#define _24BIT              24UL

#define ALIGN_UP(size, align)(((size) + ((align) - 1)) & ~((align) - 1))
#define DEGREES(n) ((65536UL * (n)) / 360)

static constexpr uint32_t BVS_RAM_ADDR = 0;

#define RAM_G               0x000000    // 0x0FFFFFh 1024 kB  General purpose graphics RAM
#define ROM_FONT            0x1E0000    // 0x2FFFFBh 1152 kB Font table and bitmap
#define ROM_FONT_ADDR       0x2FFFFC    // 0x2FFFFFh 4 B  Font table pointer address
#define RAM_DL              0x300000    // 0x301FFFh 8 kB Display List RAM
#define RAM_REG             0x302000    // 0x302FFFh 4 kB Registers
#define RAM_CMD             0x308000    // 0x308FFFh 4 kB Command buffer

#define RAM_G_SIZE          0x100000    // 1024 kB
#define ROM_FONT_SIZE       0x120000    // 1152 kB
#define ROM_FONT_ADDR_SIZE  0x000004    // 4 B
#define RAM_DL_SIZE         0x002000    // 8 kB
#define RAM_REG_SIZE        0x001000    // 4 kB
#define RAM_CMD_SIZE        0x001000    // 4 kB

static constexpr size_t     CMD_BUF_SIZE = 4092;

// ============================================================================
// EVE command coprocessor
// ============================================================================

#define REG_CMD_READ        0x3020F8UL // Позиция чтения кольцевого RAM_CMD
#define REG_CMD_WRITE       0x3020FCUL // Позиция записи кольцевого RAM_CMD
#define REG_CMD_DL          0x302100UL // Размер создаваемого Display List

#define REG_CMDB_SPACE      0x302574UL // Свободное место FIFO, максимум 4092
#define REG_CMDB_WRITE      0x302578UL // Порт записи команд сопроцессора

// [ --== Основные адреса памяти и регистров BT817/BT818 ==-- ]
#define RAM_G               0x000000UL  // Графическая память BT818, 1 Мбайт.
#define RAM_DL              0x300000UL  // Display List, 8 Кбайт.
#define REG_ID              0x302000UL  // После успешного запуска всегда содержит 0x7C.
#define REG_CLOCK           0x302008UL  // Системный счётчик тактов EVE.
#define REG_FREQUENCY       0x30200CUL  // Текущая системная частота EVE в герцах.
#define REG_CPURESET        0x302020UL  // Биты reset движков; после запуска должно быть 0.
#define REG_HCYCLE          0x30202CUL  // Полное число pixel clocks в строке.
#define REG_HOFFSET         0x302030UL  // Начало видимой горизонтальной области.
#define REG_HSIZE           0x302034UL  // Ширина видимой области.
#define REG_HSYNC0          0x302038UL  // Начало горизонтального sync.
#define REG_HSYNC1          0x30203CUL  // Конец горизонтального sync.
#define REG_VCYCLE          0x302040UL  // Полное число строк в кадре.
#define REG_VOFFSET         0x302044UL  // Начало видимой вертикальной области.
#define REG_VSIZE           0x302048UL  // Высота видимой области.
#define REG_VSYNC0          0x30204CUL  // Начало вертикального sync.
#define REG_VSYNC1          0x302050UL  // Конец вертикального sync.
#define REG_DLSWAP          0x302054UL  // Управление сменой RAM_DL.
#define REG_ROTATE          0x302058UL  // Аппаратный поворот scanout.
#define REG_OUTBITS         0x30205CUL  // Число активных бит каналов RGB.
#define REG_DITHER          0x302060UL  // Dithering RGB-выхода.
#define REG_SWIZZLE         0x302064UL  // Перестановка каналов/бит RGB.
#define REG_CSPREAD         0x302068UL  // Spread-spectrum для PCLK.
#define REG_PCLK_POL        0x30206CUL  // Полярность выборки pixel clock.
#define REG_PCLK            0x302070UL  // Делитель pixel clock; 0 выключает scanout.
#define REG_SPI_WIDTH       0x302188UL  // Ширина SPI: 0=Single, 1=Dual, 2=Quad; бит 2=extra dummy.
#define REG_CMDB_SPACE      0x302574UL // Свободное место командного FIFO; пустой FIFO = 4092.
#define REG_CMDB_WRITE      0x302578UL // Write-only порт для блочной записи команд в FIFO.

// [ --== SPI ==--]
// Биты 1:0 — ширина шины SPI
#define SPI_WIDTH_SINGLE       0x00  // Single SPI: MOSI передаёт, MISO принимает
#define SPI_WIDTH_DUAL         0x01  // Dual SPI: IO0 и IO1
#define SPI_WIDTH_QUAD         0x02  // Quad SPI: IO0, IO1, IO2 и IO3

// Бит 2 — дополнительный dummy-байт при чтении
#define SPI_WIDTH_DUMMY        0x04  // Добавить второй dummy-байт


// Готовые режимы без дополнительного dummy-байта
#define SPI_SINGLE             0x00  // Single SPI, 1 dummy-байт
#define SPI_DUAL               0x01  // Dual SPI,   1 dummy-байт
#define SPI_QUAD               0x02  // Quad SPI,   1 dummy-байт

// Готовые режимы с дополнительным dummy-байтом
#define SPI_SINGLE_DUMMY       0x04  // Single SPI, 2 dummy-байта
#define SPI_DUAL_DUMMY         0x05  // Dual SPI,   2 dummy-байта
#define SPI_QUAD_DUMMY         0x06  // Quad SPI,   2 dummy-байта
//----------------------------------------------------------------------------------

#define CHIP_RESET  0x7C       // Expected REG_ID value after a successful EVE boot.
#define CHIPID_ADDR 0x0C0000UL // to 0x0C0003 (4 bytes)
    //- 0C0000h: 08h
    //- 0C0001h: 15h (BT815), 16h(BT816)
    //- 0C0002h: 01h
    //- 0C0003h: 00h

  
