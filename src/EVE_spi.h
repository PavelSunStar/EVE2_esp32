#pragma once

#include <Arduino.h>
#include <driver/spi_master.h>
#include <esp_err.h>

#include "structures.h"

class EVE_spi{
    public:
        void setPins(spiPins pins);
        bool begin(uint8_t spiMode = 0);
        bool setSPIMode(bool quad = false);
        bool isQuadMode() const { return _quadMode; }
        bool isQuadAvailable() const { return _quadAvailable; }

        uint8_t  rd8(uint32_t address);
        uint16_t rd16(uint32_t address);
        uint32_t rd32(uint32_t address);

        bool wr8(uint32_t address, uint8_t value);
        bool wr16(uint32_t address, uint16_t value);
        bool wr32(uint32_t address, uint32_t value);
        bool readBlock(uint32_t address, void* data, size_t size);
        bool writeBlock(uint32_t address, const void* data, size_t size);

        bool qrd8(uint32_t address, uint8_t& value);
        bool qrd16(uint32_t address, uint16_t& value);
        bool qrd32(uint32_t address, uint32_t& value);
        bool qwr8(uint32_t address, uint8_t value);
        bool qwr16(uint32_t address, uint16_t value);
        bool qwr32(uint32_t address, uint32_t value);
        bool qreadBlock(uint32_t address, void* data, size_t size);
        bool qwriteBlock(uint32_t address, const void* data, size_t size);

        bool selfTest();

        bool hostCommand(uint8_t command, uint8_t parameter = 0);

        bool spiWrite(const void* data, size_t size);
        bool spiWriteQuad(const void* data, size_t size);
        bool spiTransfer(const void* txData, void* rxData, size_t size);

    private:
        bool _ready = false;
        bool _quadMode = false;
        bool _quadAvailable = false;
        spiPins _pins = {};

        void destroy();
        bool initSPI(); 
        bool eveHardReset();
        bool transactionBlock(uint32_t address, const void* txData,
                              void* rxData, size_t size, bool quad,
                              uint8_t dummyBytes = 0);
        bool benchmarkBlocks(const char* modeName);

        spi_host_device_t SPI_HOST = SPI2_HOST;
        spi_device_handle_t spiDevice = nullptr;
};    

