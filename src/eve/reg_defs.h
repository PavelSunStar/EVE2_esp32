#pragma once

/*
 * Константы и параметры BT817/BT818 (EVE4).
 *
 * Здесь находятся не самостоятельные команды, а значения параметров для:
 *   VERTEX_FORMAT, BEGIN, BLEND_FUNC, ALPHA_FUNC, STENCIL_FUNC,
 *   STENCIL_OP, BITMAP_LAYOUT, BITMAP_SIZE и команд сопроцессора CMD_*.
 *
 * Один и тот же бит OPT_* может иметь разный смысл в разных CMD_* — это
 * предусмотрено EVE и не является конфликтом.
 */

// =============================================================================
// VERTEX_FORMAT — точность координат VERTEX2F
// =============================================================================

#define PX_PREC_1                  0UL  // Шаг 1 пиксель;    диапазон целой части −16384…16383.
#define PX_PREC_1_2                1UL  // Шаг 1/2 пикселя;  диапазон −8192…8191.
#define PX_PREC_1_4                2UL  // Шаг 1/4 пикселя;  диапазон −4096…4095.
#define PX_PREC_1_8                3UL  // Шаг 1/8 пикселя;  диапазон −2048…2047.
#define PX_PREC_1_16               4UL  // Шаг 1/16 пикселя; диапазон −1024…1023; значение после сброса.
#define PX_PREC_1_32               5UL  // Шаг 1/32 пикселя; диапазон −512…511.
#define PX_PREC_1_64               6UL  // Шаг 1/64 пикселя; диапазон −256…255.
#define PX_PREC_1_128              7UL  // Шаг 1/128 пикселя; диапазон −128…127.

// Короткие совместимые алиасы. Новому коду лучше использовать PX_PREC_*.
#define _1_2                       PX_PREC_1_2
#define _1_4                       PX_PREC_1_4
#define _1_8                       PX_PREC_1_8
#define _1_16                      PX_PREC_1_16
#define _1_32                      PX_PREC_1_32
#define _1_64                      PX_PREC_1_64
#define _1_128                     PX_PREC_1_128

// =============================================================================
// BEGIN — графические примитивы
// =============================================================================

#define BITMAPS                    1UL  // Рисование bitmap через VERTEX2F/VERTEX2II.
#define POINTS                     2UL  // Каждая вершина рисует круглую точку размера POINT_SIZE.
#define LINES                      3UL  // Каждая пара вершин образует отдельный отрезок.
#define LINE_STRIP                 4UL  // Последовательные вершины образуют связанную ломаную.
#define EDGE_STRIP_R               5UL  // Полоса от ломаной до правой границы clipping-области.
#define EDGE_STRIP_L               6UL  // Полоса от ломаной до левой границы clipping-области.
#define EDGE_STRIP_A               7UL  // Полоса от ломаной до верхней границы clipping-области.
#define EDGE_STRIP_B               8UL  // Полоса от ломаной до нижней границы clipping-области.
#define RECTS                      9UL  // Каждая пара вершин задаёт противоположные углы прямоугольника.

// =============================================================================
// ALPHA_FUNC и STENCIL_FUNC — функции сравнения
// =============================================================================

#define NEVER                      0UL  // Проверка никогда не проходит.
#define LESS                       1UL  // Новое значение меньше reference.
#define LEQUAL                     2UL  // Новое значение меньше либо равно reference.
#define GREATER                    3UL  // Новое значение больше reference.
#define GEQUAL                     4UL  // Новое значение больше либо равно reference.
#define EQUAL                      5UL  // Новое значение равно reference.
#define NOTEQUAL                   6UL  // Новое значение не равно reference.
#define ALWAYS                     7UL  // Проверка всегда проходит.

// =============================================================================
// BLEND_FUNC — коэффициенты смешивания цветов
// =============================================================================

#define ZERO                       0UL  // Коэффициент 0.
#define ONE                        1UL  // Коэффициент 1.
#define SRC_ALPHA                  2UL  // Alpha рисуемого источника.
#define DST_ALPHA                  3UL  // Alpha уже находящегося на экране цвета.
#define ONE_MINUS_SRC_ALPHA        4UL  // 1 − alpha источника.
#define ONE_MINUS_DST_ALPHA        5UL  // 1 − alpha назначения.

// Стандартное alpha-смешивание:
//   BLEND_FUNC(SRC_ALPHA, ONE_MINUS_SRC_ALPHA)

// =============================================================================
// STENCIL_OP — операции над stencil-буфером
// =============================================================================

#define STENCIL_ZERO               0UL  // Записать 0.
#define STENCIL_KEEP               1UL  // Оставить текущее значение без изменения.
#define STENCIL_REPLACE            2UL  // Записать reference из STENCIL_FUNC.
#define STENCIL_INCR               3UL  // Увеличить с ограничением на 255.
#define STENCIL_DECR               4UL  // Уменьшить с ограничением на 0.
#define STENCIL_INVERT             5UL  // Инвертировать все 8 бит.

// Совместимые короткие имена, используемые в некоторых EVE-библиотеках.
#define KEEP                       STENCIL_KEEP
#define REPLACE                    STENCIL_REPLACE
#define INCR                       STENCIL_INCR
#define DECR                       STENCIL_DECR
#define INVERT                     STENCIL_INVERT

// =============================================================================
// BITMAP_SIZE — фильтрация и поведение координат за границами bitmap
// =============================================================================

#define NEAREST                    0UL  // Выбирать ближайший texel; резкое масштабирование.
#define BILINEAR                   1UL  // Интерполировать четыре соседних texel; сглаживание.
#define BORDER                     0UL  // За границей bitmap использовать прозрачную границу.
#define REPEAT                     1UL  // Повторять bitmap по соответствующей оси.

// =============================================================================
// BITMAP_LAYOUT — обычные bitmap-форматы
// Число в комментарии — количество бит на пиксель.
// =============================================================================

#define ARGB1555                   0UL   // 16 bpp: 1 alpha + 5R + 5G + 5B.
#define L1                         1UL   // 1 bpp: монохромная яркость.
#define L4                         2UL   // 4 bpp: яркость 0–15.
#define L8                         3UL   // 8 bpp: яркость 0–255.
#define RGB332                     4UL   // 8 bpp: 3R + 3G + 2B.
#define ARGB2                      5UL   // 8 bpp: 2A + 2R + 2G + 2B.
#define ARGB4                      6UL   // 16 bpp: 4A + 4R + 4G + 4B.
#define RGB565                     7UL   // 16 bpp: 5R + 6G + 5B.
#define PALETTED                   8UL   // 8 bpp: старый индексный формат; палитра ARGB4.
#define TEXT8X8                    9UL   // 8 bpp: аппаратный текстовый формат 8×8.
#define TEXTVGA                   10UL   // 8 bpp: аппаратный VGA-текст.
#define BARGRAPH                  11UL   // 8 bpp: специальный формат горизонтальной диаграммы.
#define PALETTED565               14UL   // 8 bpp: индекс, палитра RGB565.
#define PALETTED4444              15UL   // 8 bpp: индекс, палитра ARGB4444.
#define PALETTED8                 16UL   // 8 bpp: индекс, палитра ARGB8888.
#define L2                        17UL   // 2 bpp: яркость 0–3.
#define GLFORMAT                  31UL   // Маркер: реальный формат задан BITMAP_EXT_FORMAT.

// =============================================================================
// BITMAP_EXT_FORMAT — ASTC-сжатие
// Значение в комментарии — эффективное количество бит на пиксель.
// =============================================================================

#define COMPRESSED_RGBA_ASTC_4x4_KHR     0x93B0UL  // 8.00 bpp; лучшее качество, наибольший размер.
#define COMPRESSED_RGBA_ASTC_5x4_KHR     0x93B1UL  // 6.40 bpp.
#define COMPRESSED_RGBA_ASTC_5x5_KHR     0x93B2UL  // 5.12 bpp.
#define COMPRESSED_RGBA_ASTC_6x5_KHR     0x93B3UL  // 4.27 bpp.
#define COMPRESSED_RGBA_ASTC_6x6_KHR     0x93B4UL  // 3.56 bpp.
#define COMPRESSED_RGBA_ASTC_8x5_KHR     0x93B5UL  // 3.20 bpp.
#define COMPRESSED_RGBA_ASTC_8x6_KHR     0x93B6UL  // 2.67 bpp.
#define COMPRESSED_RGBA_ASTC_8x8_KHR     0x93B7UL  // 2.00 bpp; хороший общий компромисс.
#define COMPRESSED_RGBA_ASTC_10x5_KHR    0x93B8UL  // 2.56 bpp.
#define COMPRESSED_RGBA_ASTC_10x6_KHR    0x93B9UL  // 2.13 bpp.
#define COMPRESSED_RGBA_ASTC_10x8_KHR    0x93BAUL  // 1.60 bpp.
#define COMPRESSED_RGBA_ASTC_10x10_KHR   0x93BBUL  // 1.28 bpp.
#define COMPRESSED_RGBA_ASTC_12x10_KHR   0x93BCUL  // 1.07 bpp.
#define COMPRESSED_RGBA_ASTC_12x12_KHR   0x93BDUL  // 0.89 bpp; максимальное сжатие.

// =============================================================================
// BITMAP_SWIZZLE — источники выходных каналов
// ZERO и ONE также используются как коэффициенты BLEND_FUNC.
// =============================================================================

#define SWIZZLE_ZERO               ZERO  // Постоянное значение 0.
#define SWIZZLE_ONE                ONE   // Постоянное значение 1.
#define SWIZZLE_RED                2UL   // Красный канал исходного bitmap.
#define SWIZZLE_GREEN              3UL   // Зелёный канал исходного bitmap.
#define SWIZZLE_BLUE               4UL   // Синий канал исходного bitmap.
#define SWIZZLE_ALPHA              5UL   // Alpha-канал исходного bitmap.

// =============================================================================
// Общие options для CMD_* сопроцессора
// Одинаковые значения намеренно используются разными командами.
// =============================================================================

#define OPT_3D                     0x0000UL  // Виджет с обычным объёмным оформлением; противоположность OPT_FLAT.
#define OPT_MONO                   0x0001UL  // CMD_LOADIMAGE: декодировать изображение в монохромный L8.
#define OPT_NODL                   0x0002UL  // Не добавлять автоматически bitmap-команды в Display List.
#define OPT_NOTEAR                 0x0004UL  // Видео: синхронизировать обновление, уменьшая tearing.
#define OPT_FULLSCREEN            0x0008UL  // Масштабировать изображение/видео максимально на весь экран.
#define OPT_MEDIAFIFO             0x0010UL  // Читать входные данные из Media FIFO.
#define OPT_SOUND                 0x0020UL  // CMD_PLAYVIDEO: декодировать звуковую дорожку.
#define OPT_FLASH                 0x0040UL  // Читать входные данные из подключённой внешней Flash.
#define OPT_OVERLAY               0x0080UL  // Видео: наложить кадр на существующий Display List.
#define OPT_FLAT                  0x0100UL  // Виджет без объёмной рамки и бликов.
#define OPT_SIGNED                0x0100UL  // CMD_NUMBER: интерпретировать число как знаковое.
#define OPT_DITHER                0x0100UL  // CMD_LOADIMAGE: включить dithering при декодировании PNG.
#define OPT_CENTERX               0x0200UL  // Центрировать объект/текст относительно X.
#define OPT_CENTERY               0x0400UL  // Центрировать объект/текст относительно Y.
#define OPT_CENTER                (OPT_CENTERX | OPT_CENTERY)  // Центрировать одновременно по X и Y.
#define OPT_RIGHTX                0x0800UL  // Координату X считать правой границей текста/числа.
#define OPT_FORMAT                0x1000UL  // CMD_CALIBRATE: использовать расширенное форматирование.
#define OPT_NOBACK                0x1000UL  // CMD_CLOCK/GAUGE: не рисовать фон виджета.
#define OPT_FILL                  0x2000UL  // CMD_TEXT: переносить и заполнять текст в пределах CMD_FILLWIDTH.
#define OPT_NOTICKS               0x2000UL  // CMD_CLOCK/GAUGE: не рисовать деления.
#define OPT_NOHM                  0x4000UL  // CMD_CLOCK: скрыть часовую и минутную стрелки.
#define OPT_NOPOINTER             0x4000UL  // CMD_GAUGE: скрыть стрелку.
#define OPT_NOSECS                0x8000UL  // CMD_CLOCK: скрыть секундную стрелку.
#define OPT_NOHANDS               0xC000UL  // CMD_CLOCK: скрыть все стрелки; OPT_NOHM | OPT_NOSECS.
#define OPT_RGB565                0x0000UL  // Формат результата RGB565 там, где формат выбирается options.

// =============================================================================
// Touch mode — значения REG_CTOUCH_MODE/REG_TOUCH_MODE
// =============================================================================

#define TOUCHMODE_OFF             0UL  // Touch-измерения выключены.
#define TOUCHMODE_ONESHOT         1UL  // Выполнить одно измерение и перейти в OFF.
#define TOUCHMODE_FRAME           2UL  // Выполнять одно измерение на каждый кадр.
#define TOUCHMODE_CONTINUOUS      3UL  // Выполнять измерения непрерывно.

// =============================================================================
// REG_DLSWAP — режим смены Display List
// =============================================================================

#define DLSWAP_DONE               0UL  // Смена завершена; нового запроса нет.
#define DLSWAP_LINE               1UL  // Выполнить смену после текущей строки сканирования.
#define DLSWAP_FRAME              2UL  // Выполнить смену после полного текущего кадра.

// =============================================================================
// REG_INT_FLAGS / REG_INT_MASK — основные флаги прерываний
// =============================================================================

#define INT_SWAP                  0x001UL  // Завершилась смена Display List.
#define INT_TOUCH                 0x002UL  // Обнаружено касание.
#define INT_TAG                   0x004UL  // Изменился touch tag.
#define INT_SOUND                 0x008UL  // Завершился встроенный звуковой эффект.
#define INT_PLAYBACK              0x010UL  // Завершилось воспроизведение аудиоданных.
#define INT_CMDEMPTY              0x020UL  // FIFO сопроцессора стал пустым.
#define INT_CMDFLAG               0x040UL  // Сработала команда CMD_INTERRUPT.
#define INT_CONVCOMPLETE          0x080UL  // Завершилось touch-преобразование.
#define INT_UNDERRUN              0x100UL  // Graphics pipeline не успел подготовить данные строки.

// =============================================================================
// Формат воспроизводимых аудиосэмплов
// =============================================================================

#define LINEAR_SAMPLES            0UL  // 8-битные линейные PCM-сэмплы.
#define ULAW_SAMPLES              1UL  // 8-битные µ-law-сэмплы.
#define ADPCM_SAMPLES             2UL  // 4-битные IMA ADPCM-сэмплы.

// =============================================================================
// Параметры анимации
// =============================================================================

#define ANIM_ONCE                 0UL  // Проиграть один раз.
#define ANIM_LOOP                 1UL  // Повторять циклически.
#define ANIM_HOLD                 2UL  // После завершения удерживать последний кадр.

// =============================================================================
// REG_FLASH_STATUS — состояние внешней Flash
// =============================================================================

#define FLASH_STATUS_INIT         0UL  // Начальное состояние после сброса.
#define FLASH_STATUS_DETACHED     1UL  // Flash отключена.
#define FLASH_STATUS_BASIC        2UL  // Flash подключена в базовом SPI-режиме.
#define FLASH_STATUS_FULL         3UL  // Flash работает в полном быстром режиме.

