#include <Arduino.h>
#include <EVE_esp32.h>
#include "ESP32_Gamepad.h"

EVE_esp32 GD;
ESP32_Gamepad pad;


// ============================================================
// SETTINGS
// ============================================================

// Отдельное имя, чтобы не конфликтовать с DEADZONE
// внутри ESP32_Gamepad.h
constexpr int STICK_DEADZONE = 10;

// Trail
constexpr int TRAIL_SIZE = 64;

// Частота записи положения стика
constexpr uint32_t TRAIL_INTERVAL_MS = 15;

// Полное время жизни следа
constexpr uint32_t TRAIL_LIFETIME_MS = 1200;

// Минимальное изменение координат для записи новой точки
constexpr int TRAIL_MIN_MOVE = 2;


// ============================================================
// TRAIL
// ============================================================

struct TrailPoint {
    int8_t x;
    int8_t y;
    uint32_t time;
    bool valid;
};

TrailPoint leftTrail[TRAIL_SIZE]  = {};
TrailPoint rightTrail[TRAIL_SIZE] = {};

int leftTrailPos  = 0;
int rightTrailPos = 0;

uint32_t lastTrailMs = 0;


// ============================================================
// D-PAD NAME
// ============================================================

static const char* dpadName(ESP32_Gamepad& p)
{
    if (p.Center())    return "CENTER";
    if (p.Up())        return "UP";
    if (p.UpRight())   return "UP RIGHT";
    if (p.Right())     return "RIGHT";
    if (p.DownRight()) return "DOWN RIGHT";
    if (p.Down())      return "DOWN";
    if (p.DownLeft())  return "DOWN LEFT";
    if (p.Left())      return "LEFT";
    if (p.UpLeft())    return "UP LEFT";

    return "?";
}


// ============================================================
// ADD TRAIL POINT
// ============================================================

static void addTrailPoint(
    TrailPoint* trail,
    int& writePos,
    int x,
    int y)
{
    x = constrain(x, -128, 127);
    y = constrain(y, -128, 127);

    int previous = writePos - 1;

    if (previous < 0)
        previous = TRAIL_SIZE - 1;


    // --------------------------------------------------------
    // Не записываем практически одинаковые точки
    // --------------------------------------------------------

    if (trail[previous].valid) {

        int dx = x - trail[previous].x;
        int dy = y - trail[previous].y;

        if (
            abs(dx) < TRAIL_MIN_MOVE &&
            abs(dy) < TRAIL_MIN_MOVE
        ) {
            return;
        }
    }


    // --------------------------------------------------------
    // Запись
    // --------------------------------------------------------

    trail[writePos].x =
        (int8_t)x;

    trail[writePos].y =
        (int8_t)y;

    trail[writePos].time =
        millis();

    trail[writePos].valid =
        true;


    writePos++;

    if (writePos >= TRAIL_SIZE)
        writePos = 0;
}


// ============================================================
// UPDATE BOTH TRAILS
// ============================================================

static void updateTrails(
    int lx,
    int ly,
    int rx,
    int ry)
{
    addTrailPoint(
        leftTrail,
        leftTrailPos,
        lx,
        ly
    );

    addTrailPoint(
        rightTrail,
        rightTrailPos,
        rx,
        ry
    );
}


// ============================================================
// TRAIL -> SCREEN COORDINATES
// ============================================================

static void trailToScreen(
    int cx,
    int cy,
    int radius,
    int x,
    int y,
    int& px,
    int& py)
{
    px =
        cx + (x * (radius - 12)) / 128;

    py =
        cy + (y * (radius - 12)) / 128;


    px = constrain(
        px,
        cx - radius + 8,
        cx + radius - 8
    );

    py = constrain(
        py,
        cy - radius + 8,
        cy + radius - 8
    );
}


// ============================================================
// DRAW CONTINUOUS FADING TRAIL
// ============================================================

static void drawTrail(
    int cx,
    int cy,
    TrailPoint* trail,
    int writePos,
    int radius)
{
    const uint32_t now =
        millis();


    // ========================================================
    // Удаляем протухшие точки
    // ========================================================

    for (int i = 0; i < TRAIL_SIZE; i++) {

        if (!trail[i].valid)
            continue;

        uint32_t age =
            now - trail[i].time;

        if (age >= TRAIL_LIFETIME_MS)
            trail[i].valid = false;
    }


    // ========================================================
    // Рисуем сегменты
    // ========================================================

    for (int n = 0; n < TRAIL_SIZE - 1; n++) {

        int i1 =
            (writePos + n) % TRAIL_SIZE;

        int i2 =
            (writePos + n + 1) % TRAIL_SIZE;


        if (!trail[i1].valid)
            continue;

        if (!trail[i2].valid)
            continue;


        // ----------------------------------------------------
        // Возраст
        // ----------------------------------------------------

        uint32_t age1 =
            now - trail[i1].time;

        uint32_t age2 =
            now - trail[i2].time;


        if (age1 >= TRAIL_LIFETIME_MS)
            continue;

        if (age2 >= TRAIL_LIFETIME_MS)
            continue;


        // ----------------------------------------------------
        // Если между точками слишком большой временной разрыв,
        // не соединяем их.
        //
        // Иначе после паузы могла бы появиться длинная линия.
        // ----------------------------------------------------

        uint32_t dt =
            trail[i2].time - trail[i1].time;

        if (dt > 100)
            continue;


        // ----------------------------------------------------
        // Fade
        //
        // Новая линия: 255
        // Старая линия постепенно темнеет
        // ----------------------------------------------------

        int brightness =
            255 -
            (int)(
                (age2 * 235UL) /
                TRAIL_LIFETIME_MS
            );


        brightness =
            constrain(
                brightness,
                20,
                255
            );


        // ----------------------------------------------------
        // Coordinates
        // ----------------------------------------------------

        int x1, y1;
        int x2, y2;


        trailToScreen(
            cx,
            cy,
            radius,
            trail[i1].x,
            trail[i1].y,
            x1,
            y1
        );


        trailToScreen(
            cx,
            cy,
            radius,
            trail[i2].x,
            trail[i2].y,
            x2,
            y2
        );


        // ----------------------------------------------------
        // Line
        // ----------------------------------------------------

        GD.ColorRGB(
            0,
            brightness,
            brightness
        );


        GD.Begin(LINES);

        GD.LineWidth(
            2 * 16
        );

        GD.Vertex2f(
            x1 * 16,
            y1 * 16
        );

        GD.Vertex2f(
            x2 * 16,
            y2 * 16
        );

        GD.End();


        // ----------------------------------------------------
        // Маленькая точка на конце сегмента
        // ----------------------------------------------------

        GD.Begin(POINTS);

        GD.PointSize(
            2 * 2 * 16
        );

        GD.Vertex2f(
            x2 * 16,
            y2 * 16
        );

        GD.End();
    }
}


// ============================================================
// DRAW STICK
// ============================================================

static void drawStick(
    int cx,
    int cy,
    int valueX,
    int valueY,
    const char* name,
    TrailPoint* trail,
    int trailWritePos)
{
    const int radius = 55;


    // ========================================================
    // RAW values
    // ========================================================

    const int rawX = valueX;
    const int rawY = valueY;


    // ========================================================
    // DEAD ZONE
    // ========================================================

    bool xDead =
        abs(valueX) <= STICK_DEADZONE;

    bool yDead =
        abs(valueY) <= STICK_DEADZONE;


    if (xDead)
        valueX = 0;

    if (yDead)
        valueY = 0;


    bool inDeadZone =
        xDead && yDead;


    // ========================================================
    // Stick name
    // ========================================================

    GD.ColorRGB(
        255,
        255,
        255
    );

    GD.cmd_text(
        cx,
        cy - radius - 22,
        26,
        OPT_CENTER,
        name
    );


    // ========================================================
    // Outer circle
    // ========================================================

    GD.ColorRGB(
        100,
        110,
        120
    );

    GD.Begin(POINTS);

    GD.PointSize(
        radius * 2 * 16
    );

    GD.Vertex2f(
        cx * 16,
        cy * 16
    );

    GD.End();


    // ========================================================
    // Inner circle
    // ========================================================

    GD.ColorRGB(
        18,
        27,
        30
    );

    GD.Begin(POINTS);

    GD.PointSize(
        (radius - 4) * 2 * 16
    );

    GD.Vertex2f(
        cx * 16,
        cy * 16
    );

    GD.End();


    // ========================================================
    // DEAD ZONE circle
    // ========================================================

    int deadRadius =
        ((radius - 12) * STICK_DEADZONE) / 128;


    if (deadRadius < 3)
        deadRadius = 3;


    GD.ColorRGB(
        35,
        85,
        85
    );

    GD.Begin(POINTS);

    GD.PointSize(
        deadRadius * 2 * 16
    );

    GD.Vertex2f(
        cx * 16,
        cy * 16
    );

    GD.End();


    // ========================================================
    // Cross
    // ========================================================

    GD.ColorRGB(
        80,
        170,
        170
    );

    GD.Begin(LINES);

    GD.LineWidth(16);


    // Horizontal
    GD.Vertex2f(
        (cx - radius + 12) * 16,
        cy * 16
    );

    GD.Vertex2f(
        (cx + radius - 12) * 16,
        cy * 16
    );


    // Vertical
    GD.Vertex2f(
        cx * 16,
        (cy - radius + 12) * 16
    );

    GD.Vertex2f(
        cx * 16,
        (cy + radius - 12) * 16
    );


    GD.End();


    // ========================================================
    // FADING TRAIL
    // ========================================================

    drawTrail(
        cx,
        cy,
        trail,
        trailWritePos,
        radius
    );


    // ========================================================
    // RAW point
    //
    // Маленькая жёлтая точка
    // ========================================================

    int rawPX =
        cx + (rawX * (radius - 12)) / 128;

    int rawPY =
        cy + (rawY * (radius - 12)) / 128;


    rawPX = constrain(
        rawPX,
        cx - radius + 8,
        cx + radius - 8
    );

    rawPY = constrain(
        rawPY,
        cy - radius + 8,
        cy + radius - 8
    );


    GD.ColorRGB(
        255,
        220,
        0
    );

    GD.Begin(POINTS);

    GD.PointSize(
        4 * 2 * 16
    );

    GD.Vertex2f(
        rawPX * 16,
        rawPY * 16
    );

    GD.End();


    // ========================================================
    // Processed point
    // ========================================================

    int px =
        cx + (valueX * (radius - 12)) / 128;

    int py =
        cy + (valueY * (radius - 12)) / 128;


    px = constrain(
        px,
        cx - radius + 8,
        cx + radius - 8
    );

    py = constrain(
        py,
        cy - radius + 8,
        cy + radius - 8
    );


    // Green = DEAD ZONE
    // Red   = ACTIVE

    if (inDeadZone) {

        GD.ColorRGB(
            0,
            255,
            100
        );

    } else {

        GD.ColorRGB(
            255,
            80,
            40
        );
    }


    GD.Begin(POINTS);

    GD.PointSize(
        9 * 2 * 16
    );

    GD.Vertex2f(
        px * 16,
        py * 16
    );

    GD.End();


    // ========================================================
    // X/Y values
    // ========================================================

    char buf[64];


    snprintf(
        buf,
        sizeof(buf),
        "X:%4d Y:%4d",
        rawX,
        rawY
    );


    GD.ColorRGB(
        255,
        255,
        255
    );


    GD.cmd_text(
        cx,
        cy + radius + 12,
        26,
        OPT_CENTER,
        buf
    );


    // ========================================================
    // DEAD ZONE / ACTIVE
    // ========================================================

    if (inDeadZone) {

        GD.ColorRGB(
            0,
            255,
            100
        );


        snprintf(
            buf,
            sizeof(buf),
            "DEAD ZONE +/- %d",
            STICK_DEADZONE
        );

    } else {

        GD.ColorRGB(
            255,
            100,
            50
        );


        snprintf(
            buf,
            sizeof(buf),
            "ACTIVE"
        );
    }


    GD.cmd_text(
        cx,
        cy + radius + 31,
        20,
        OPT_CENTER,
        buf
    );
}


// ============================================================
// DRAW TRIGGER
// ============================================================

static void drawTrigger(
    int x,
    int y,
    int w,
    int value,
    const char* name)
{
    value =
        constrain(value, 0, 255);


    char buf[32];


    snprintf(
        buf,
        sizeof(buf),
        "%s %3d",
        name,
        value
    );


    // ========================================================
    // Text
    // ========================================================

    GD.ColorRGB(
        255,
        255,
        255
    );


    GD.cmd_text(
        x,
        y,
        26,
        0,
        buf
    );


    const int barY = y + 23;
    const int barH = 13;


    // ========================================================
    // Background
    // ========================================================

    GD.ColorRGB(
        90,
        95,
        95
    );


    GD.Begin(RECTS);

    GD.Vertex2f(
        x * 16,
        barY * 16
    );

    GD.Vertex2f(
        (x + w) * 16,
        (barY + barH) * 16
    );

    GD.End();


    // ========================================================
    // Fill
    // ========================================================

    int fill =
        (value * w) / 255;


    if (fill > 0) {

        GD.ColorRGB(
            0,
            210,
            255
        );


        GD.Begin(RECTS);

        GD.Vertex2f(
            x * 16,
            barY * 16
        );

        GD.Vertex2f(
            (x + fill) * 16,
            (barY + barH) * 16
        );

        GD.End();
    }
}


// ============================================================
// ROUND BUTTON
// ============================================================

static void drawButton(
    int x,
    int y,
    int radius,
    const char* text,
    bool pressed)
{
    // ========================================================
    // Outer
    // ========================================================

    if (pressed) {

        GD.ColorRGB(
            255,
            180,
            0
        );

    } else {

        GD.ColorRGB(
            80,
            85,
            90
        );
    }


    GD.Begin(POINTS);

    GD.PointSize(
        radius * 2 * 16
    );

    GD.Vertex2f(
        x * 16,
        y * 16
    );

    GD.End();


    // ========================================================
    // Inner
    // ========================================================

    if (pressed) {

        GD.ColorRGB(
            255,
            70,
            20
        );

    } else {

        GD.ColorRGB(
            25,
            30,
            35
        );
    }


    GD.Begin(POINTS);

    GD.PointSize(
        (radius - 3) * 2 * 16
    );

    GD.Vertex2f(
        x * 16,
        y * 16
    );

    GD.End();


    // ========================================================
    // Text
    // ========================================================

    if (pressed) {

        GD.ColorRGB(
            255,
            255,
            255
        );

    } else {

        GD.ColorRGB(
            180,
            185,
            190
        );
    }


    GD.cmd_text(
        x,
        y,
        20,
        OPT_CENTER,
        text
    );
}


// ============================================================
// RECT BUTTON
// ============================================================

static void drawRectButton(
    int x,
    int y,
    int w,
    int h,
    const char* text,
    bool pressed)
{
    // ========================================================
    // Background
    // ========================================================

    if (pressed) {

        GD.ColorRGB(
            0,
            200,
            255
        );

    } else {

        GD.ColorRGB(
            60,
            65,
            70
        );
    }


    GD.Begin(RECTS);

    GD.Vertex2f(
        x * 16,
        y * 16
    );

    GD.Vertex2f(
        (x + w) * 16,
        (y + h) * 16
    );

    GD.End();


    // ========================================================
    // Text
    // ========================================================

    if (pressed) {

        GD.ColorRGB(
            255,
            255,
            255
        );

    } else {

        GD.ColorRGB(
            180,
            180,
            180
        );
    }


    GD.cmd_text(
        x + w / 2,
        y + h / 2,
        20,
        OPT_CENTER,
        text
    );
}


// ============================================================
// GAMEPAD BUTTON PANEL
// ============================================================

static void drawGamepadButtons()
{
    // ========================================================
    // LB / RB
    // ========================================================

    drawRectButton(
        25,
        413,
        45,
        23,
        "LB",
        pad.LB()
    );


    drawRectButton(
        75,
        413,
        45,
        23,
        "RB",
        pad.RB()
    );


    // ========================================================
    // SELECT / START
    // ========================================================

    drawRectButton(
        125,
        413,
        55,
        23,
        "SEL",
        pad.Select()
    );


    drawRectButton(
        185,
        413,
        58,
        23,
        "START",
        pad.Start()
    );


    // ========================================================
    // CIRCLE / HOME
    // ========================================================

    drawRectButton(
        248,
        413,
        52,
        23,
        "CIR",
        pad.Circle()
    );


    drawRectButton(
        305,
        413,
        52,
        23,
        "HOME",
        pad.Home()
    );


    // ========================================================
    // M1 M2 M3 M4
    // ========================================================

    drawButton(
        377,
        424,
        13,
        "M1",
        pad.M1()
    );


    drawButton(
        407,
        424,
        13,
        "M2",
        pad.M2()
    );


    drawButton(
        437,
        424,
        13,
        "M3",
        pad.M3()
    );


    drawButton(
        467,
        424,
        13,
        "M4",
        pad.M4()
    );


    // ========================================================
    // SL / SR
    // ========================================================

    drawButton(
        497,
        424,
        12,
        "SL",
        pad.SL()
    );


    drawButton(
        525,
        424,
        12,
        "SR",
        pad.SR()
    );


    // ========================================================
    // ABXY
    // ========================================================

    const int cx = 585;
    const int cy = 424;


    // Y
    drawButton(
        cx,
        cy - 28,
        14,
        "Y",
        pad.Y()
    );


    // X
    drawButton(
        cx - 30,
        cy,
        14,
        "X",
        pad.X()
    );


    // B
    drawButton(
        cx + 30,
        cy,
        14,
        "B",
        pad.B()
    );


    // A
    drawButton(
        cx,
        cy + 28,
        14,
        "A",
        pad.A()
    );
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(200);


    Serial.println();

    Serial.println(
        "=== FLYDIGI APEX 4 EVE monitor ==="
    );


    // ========================================================
    // EVE
    //
    // PD, CS, SCK, MOSI, MISO, IO2, IO3
    // ========================================================

    GD.setPins(
        1,
        10,
        12,
        11,
        13,
        -1,
        -1
    );


    if (!GD.init(MODE640x480_57)) {

        Serial.println(
            "EVE init FAILED"
        );


        while (1) {
            delay(100);
        }
    }


    // ========================================================
    // GAMEPAD
    // ========================================================

    pad.begin();
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // ========================================================
    // GAMEPAD
    // ========================================================

    pad.loop();


    // ========================================================
    // BEGIN FRAME
    // ========================================================

    if (!GD.beginFrame()) {

        Serial.println(
            "beginFrame FAILED"
        );

        delay(10);

        return;
    }


    // ========================================================
    // BACKGROUND
    // ========================================================

    GD.ClearColorRGB(
        8,
        15,
        20
    );

    GD.Clear();


    // ========================================================
    // TITLE
    // ========================================================

    GD.ColorRGB(
        0,
        220,
        255
    );


    GD.cmd_text(
        GD.w / 2,
        18,
        30,
        OPT_CENTERX,
        "FLYDIGI APEX 4"
    );


    // ========================================================
    // NOT CONNECTED
    // ========================================================

    if (!pad.connected()) {

        GD.ColorRGB(
            255,
            80,
            80
        );


        GD.cmd_text(
            GD.w / 2,
            GD.h / 2,
            31,
            OPT_CENTER,
            "GAMEPAD NOT CONNECTED"
        );


        if (!GD.swap()) {

            Serial.println(
                "swap FAILED"
            );
        }


        delay(10);

        return;
    }


    // ========================================================
    // CONNECTED
    // ========================================================

    GD.ColorRGB(
        0,
        255,
        100
    );


    GD.cmd_text(
        GD.w / 2,
        50,
        27,
        OPT_CENTERX,
        "CONNECTED"
    );


    // ========================================================
    // READ GAMEPAD
    // ========================================================

    const int lsx =
        pad.LS_LR();

    const int lsy =
        pad.LS_UD();

    const int rsx =
        pad.RS_LR();

    const int rsy =
        pad.RS_UD();

    const int lt =
        pad.LTAnalog();

    const int rt =
        pad.RTAnalog();


    // ========================================================
    // UPDATE TRAILS
    // ========================================================

    uint32_t now =
        millis();


    if (
        now - lastTrailMs >=
        TRAIL_INTERVAL_MS
    ) {

        lastTrailMs = now;


        updateTrails(
            lsx,
            lsy,
            rsx,
            rsy
        );
    }


    // ========================================================
    // LEFT STICK
    // ========================================================

    drawStick(
        170,
        165,
        lsx,
        lsy,
        "LEFT STICK",
        leftTrail,
        leftTrailPos
    );


    // ========================================================
    // RIGHT STICK
    // ========================================================

    drawStick(
        470,
        165,
        rsx,
        rsy,
        "RIGHT STICK",
        rightTrail,
        rightTrailPos
    );


    // ========================================================
    // D-PAD
    // ========================================================

    char dpadBuf[48];


    snprintf(
        dpadBuf,
        sizeof(dpadBuf),
        "DPAD: %s",
        dpadName(pad)
    );


    GD.ColorRGB(
        255,
        255,
        255
    );


    GD.cmd_text(
        GD.w / 2,
        290,
        28,
        OPT_CENTERX,
        dpadBuf
    );


    // ========================================================
    // LT
    // ========================================================

    drawTrigger(
        30,
        320,
        270,
        lt,
        "LT"
    );


    // ========================================================
    // RT
    // ========================================================

    drawTrigger(
        340,
        320,
        270,
        rt,
        "RT"
    );


    // ========================================================
    // BUTTONS TITLE
    // ========================================================

    GD.ColorRGB(
        160,
        170,
        175
    );


    GD.cmd_text(
        25,
        385,
        20,
        0,
        "BUTTONS"
    );


    // ========================================================
    // BUTTONS
    // ========================================================

    drawGamepadButtons();


    // ========================================================
    // SWAP
    // ========================================================

    if (!GD.swap()) {

        Serial.println(
            "swap FAILED"
        );
    }
}