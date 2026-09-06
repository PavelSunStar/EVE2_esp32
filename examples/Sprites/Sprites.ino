#include <Arduino.h>
#include <EVE_esp32.h>
#include "sprites_assets.h"

EVE_esp32 GD;
static uint8_t t = 0;

void setup(){
    Serial.begin(115200);
    delay(500);

    if (!GD.init()) {
        Serial.println("EVE init FAILED");
        while (true) delay(100);
    }
    if (!GD.setSPIMode(true)) {
        Serial.println("QUAD SPI enable FAILED");
    }

    Serial.println("Loading sprite assets...");
    if (!LOAD_ASSETS()) {
        Serial.println("LOAD_ASSETS FAILED");

        while (true) {
            delay(100);
        }
    }
    Serial.println("Sprite assets loaded OK");
}

void loop(){
    /*
     * Исходные координаты рассчитаны на 480x272.
     * Центрируем эту область на экране 640x480.
     */
    const int offsetX = (GD.w - 480) / 2;
    const int offsetY = (GD.h - 272) / 2;

    if (!GD.beginFrame()) {
        Serial.println("beginFrame FAILED");
        delay(100);
        return;
    }

    /*
     * Настройка изображения спрайтов.
     * В RAM_G оно загружено с адреса 0.
     */
    GD.BitmapHandle(PICKUPS_HANDLE);
    GD.BitmapSource(0);
    GD.BitmapLayout(ARGB1555, PICKUPS_WIDTH * 2, PICKUPS_HEIGHT);

    /*
     * Исходная картинка имеет размер 16x16.
     * Выводим её размером 24x24.
     *
     * Для оригинального размера замените 24, 24
     * на PICKUPS_WIDTH, PICKUPS_HEIGHT.
     */
    GD.BitmapSize(NEAREST, BORDER, BORDER, 24, 24);
    GD.ColorRGB(255, 255, 255);
    GD.ColorA(255);

    /*
     * Сохраняем графический контекст.
     * Перемещение будет действовать только на спрайты
     * и чёрную полосу.
     */
    GD.SaveContext();
    GD.VertexTranslateX(offsetX * 16);
    GD.VertexTranslateY(offsetY * 16);

    GD.Begin(BITMAPS);
    uint8_t j = t;
    int nspr = min(2001, max(256, 19 * int(t)));
    const uint32_t* pv = sprites;

    for (int i = 0; i < nspr; i++) {
        uint32_t vertex = pgm_read_dword(pv++);
        uint32_t offset = pgm_read_dword(circle + j++);
        GD.cmd32(vertex + offset);

        /*
         * Отправляем команды частями, поскольку
         * локальный CMD-буфер равен 4092 байтам.
         */
        if ((i + 1) % 700 == 0) {
            if (!GD.sendCMD()) {
                Serial.println("Sprite block send FAILED");
                delay(100);
                return;
            }
        }
    }

    /*
     * Чёрная полупрозрачная полоса.
     * VertexTranslate также автоматически переместит её.
     */
    GD.ColorRGB(0x000000);
    GD.ColorA(140);
    GD.LineWidth(28 * 16);
    GD.Begin(LINES);
    GD.Vertex2ii(240 - 110, 136, 0, 0);
    GD.Vertex2ii(240 + 110, 136, 0, 0);

    /*
     * Убираем VertexTranslate и возвращаем
     * белый цвет с полной непрозрачностью.
     */
    GD.RestoreContext();

    /*
     * cmd_number и cmd_text получают уже окончательные
     * координаты на экране 640x480.
     */
    GD.cmd_number(offsetX + 215, offsetY + 110, 31, OPT_RIGHTX, nspr);
    GD.cmd_text(offsetX + 229, offsetY + 110, 31, 0, "sprites");

    GD.swap();
    t++;
}
