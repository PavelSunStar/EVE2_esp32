#pragma once

#include <Arduino.h>
#include "EVE_spi.h"

#include "eve/EVE_reg.h"
#include "config.h"
#include "func.h"

class EVE_esp32{
    public:
        int w = 0; 
        int h = 0; 

        EVE_esp32();
        ~EVE_esp32();

        void setPins(int8_t pinPD, int8_t pinCS, int8_t pinSCK, int8_t pinMOSI, int8_t pinMISO, int8_t pinIO2, int8_t pinIO3);
        bool init(const Mode& mode = MODE640x480_57);
        bool setVGAMode(const Mode& mode);
        bool setSPIMode(bool quad = false);
        bool isQuadMode() const { return _quadMode; }
        bool isQuadAvailable() const { return _spi.isQuadAvailable(); }

        uint8_t  rd8(uint32_t address);
        uint16_t rd16(uint32_t address);
        uint32_t rd32(uint32_t address);
        bool wr8(uint32_t address, uint8_t value);
        bool wr16(uint32_t address, uint16_t value);
        bool wr32(uint32_t address, uint32_t value);
        bool readBlock(uint32_t address, void* data, size_t size);
        bool writeBlock(uint32_t address, const void* data, size_t size);
        bool copy(const uint8_t *src, int count);

        void dl(uint32_t cmd);
        void cmd8(uint8_t data);
        void cmd16(uint16_t data);
        void cmd32(uint32_t data);
        void cs(const char* s);

        bool beginFrame(void* bg = nullptr, uint8_t colorMode = 0xff);
        bool sendCMD(uint32_t timeoutMs = 1000);
        bool waitCMD(uint32_t timeoutMs = 1000);
        bool swap(uint32_t timeoutMs = 1000);
        uint16_t cmdSpace();

        bool initBackScreen();

        #include "eve/eve_dl.h"
        #include "eve/eve_cmd.h"

        uint32_t rgb332to888(uint8_t color);

    private:  
        friend class GFX;  
        bool _inited = false; 
        bool _sdReady = false;
        bool _quadMode = false;
        bool _cmdError = false;

        dlStruct _dl;
        cmdStruct _cmd;

        void destroy();
        bool getMemory();
        bool putDL8(uint8_t data);
        bool putCMD8(uint8_t data);

        // Virtual back screen
        void* allocateMemory(size_t request, bool psram);
        bool uploadRAM(uint32_t address, const void* data, size_t size);

        EVE_spi _spi;
        spiPins _pins = {};
        Mode _mode;
};
