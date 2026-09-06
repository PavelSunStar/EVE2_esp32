#pragma once

struct spiPins {
    int CS;      // (Chip Select) SPI slave select input , active low 
    int PD;      // (Power Down / Reset) Active low power down input.
    int INT;     // Host interrupt open drain output, active low. On board 10kΩ pull-up to 3.3V.
    int MOSI;    // SPI Single mode: SPI MOSI input SPI Dual/Quad mode: SPI data line 0
    int MISO;    // SPI Single mode: SPI MISO output SPI Dual/Quad mode: SPI data line 1
    int IO2;     // SPI Single mode: General purpose IO 0 SPI Quad mode: SPI data line 2
    int IO3;     // SPI Single mode: General purpose IO 1 SPI Quad mode: SPI data line 3
    int SCK;     // CLK	SCK SPI Clock input
    uint32_t freq;
};

struct Mode {
    uint32_t pclkHz;

    uint16_t hRes;
    uint16_t hCycle;
    uint16_t hOffset;
    uint16_t hSync0;
    uint16_t hSync1;

    uint16_t vRes;
    uint16_t vCycle;
    uint16_t vOffset;
    uint16_t vSync0;
    uint16_t vSync1;

    uint8_t div;
    uint32_t frequency;
};

struct cmdStruct{
    void*buf        = nullptr;
    uint32_t pos    = 0; 
};

struct dlStruct{
    void* buf       = nullptr;
    uint32_t pos    = 0; 
};




