#pragma once

#include <Arduino.h>
#include <math.h>

constexpr int LUT_SIZE = 360;

inline int16_t sinLUT[LUT_SIZE];
inline int16_t cosLUT[LUT_SIZE];

inline void initLUT()
{
    constexpr float DEG_TO_RAD_LUT =
        3.14159265358979323846f / 180.0f;

    for (int angle = 0; angle < LUT_SIZE; angle++) {
        float rad = angle * DEG_TO_RAD_LUT;

        sinLUT[angle] =
            static_cast<int16_t>(sinf(rad) * 1000.0f);

        cosLUT[angle] =
            static_cast<int16_t>(cosf(rad) * 1000.0f);
    }
}

inline int normalizeLUTAngle(int angle)
{
    angle %= LUT_SIZE;

    if (angle < 0) {
        angle += LUT_SIZE;
    }

    return angle;
}

inline void LUT(
    int &x,
    int &y,
    int xx,
    int yy,
    int len,
    int angle
) {
    angle = normalizeLUTAngle(angle);

    x = xx + static_cast<int>(
        (static_cast<int64_t>(len) * cosLUT[angle]) / 1000
    );

    y = yy + static_cast<int>(
        (static_cast<int64_t>(len) * sinLUT[angle]) / 1000
    );
}

inline int xLUT(int x, int len, int angle)
{
    angle = normalizeLUTAngle(angle);

    return x + static_cast<int>(
        (static_cast<int64_t>(len) * cosLUT[angle]) / 1000
    );
}

inline int yLUT(int y, int len, int angle)
{
    angle = normalizeLUTAngle(angle);

    return y + static_cast<int>(
        (static_cast<int64_t>(len) * sinLUT[angle]) / 1000
    );
}