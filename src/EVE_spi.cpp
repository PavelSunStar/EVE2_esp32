#include "EVE_spi.h"
#include <cstring>
#include <esp_heap_caps.h>

#include "eve/host_cmd.h"

static constexpr uint32_t EVE_REG_ID        = 0x302000UL;
static constexpr uint32_t EVE_REG_SPI_WIDTH = 0x302188UL;
static constexpr uint8_t  EVE_SPI_SINGLE    = 0x00;
static constexpr uint8_t  EVE_SPI_QUAD      = 0x02;
static constexpr uint8_t  EVE_SPI_DUMMY     = 0x04;

EVE_spi::~EVE_spi(){
    destroy();
}

void EVE_spi::destroy(){
    if (spiDevice) {
        spi_bus_remove_device(spiDevice);
        spiDevice = nullptr;
    }

    if (_busInitialized) {
        spi_bus_free(SPI_HOST);
        _busInitialized = false;
    }

    _ready = false;
    _quadMode = false;
}

void EVE_spi::setPins(spiPins pins){
    _pins.PD = pins.PD;
    _pins.CS = pins.CS;
    _pins.SCK = pins.SCK;
    _pins.MOSI = pins.MOSI;
    _pins.MISO = pins.MISO;
    _pins.IO2 = pins.IO2;
    _pins.IO3 = pins.IO3;
    _quadAvailable = (_pins.IO2 >= 0 && _pins.IO3 >= 0);
}

bool EVE_spi::eveHardReset(){
    if (_pins.PD < 0) return false;

    gpio_set_direction((gpio_num_t)_pins.PD, GPIO_MODE_OUTPUT);

    Serial.println("EVE hard reset...");
    gpio_set_level((gpio_num_t)_pins.PD, 0);
    delay(20);
    gpio_set_level((gpio_num_t)_pins.PD, 1);
    delay(50);
    _quadMode = false;  // Hardware reset always restores Single SPI.
    Serial.println("EVE hard reset OK");

    if (!hostCommand(CLKEXT) ||
        !hostCommand(CLKSEL, 0x46) ||
        !hostCommand(RST_PULSE) ||
        !hostCommand(ACTIVE)){
        
            Serial.println("EVE host commands FAILED");
        return false;
    }

    Serial.println("EVE host commands OK");
    delay(300);
    return true;
}

bool EVE_spi::begin(uint8_t spiMode){
    destroy();

    Serial.println("\nInit EVE...");
    if (!initSPI()) return false;
    if (!eveHardReset()) return false;
    if (!selfTest()) return false;

    if (spiMode) {
        if (_quadAvailable) {
            if (!setSPIMode(true)) return false;
        } else {
            Serial.println("Quad mode requested but IO2/IO3 are disabled; using Single SPI");
        }
    }

    return (_ready = true);
}

bool EVE_spi::initSPI(){
    const bool hasIO2 = _pins.IO2 >= 0;
    const bool hasIO3 = _pins.IO3 >= 0;

    if (hasIO2 != hasIO3) {
        Serial.println("SPI pin error: IO2 and IO3 must both be set or both be -1");
        return false;
    }

    _quadAvailable = hasIO2 && hasIO3;
    _quadMode = false;

    spi_bus_config_t bus = {};
    bus.mosi_io_num = _pins.MOSI;
    bus.miso_io_num = _pins.MISO;
    bus.sclk_io_num = _pins.SCK;

    // In Single-only mode both pins stay at -1 and QUAD capability is omitted.
    bus.quadwp_io_num = _quadAvailable ? _pins.IO2 : -1;
    bus.quadhd_io_num = _quadAvailable ? _pins.IO3 : -1;

    bus.max_transfer_sz = 4096;
    bus.flags =
        SPICOMMON_BUSFLAG_MASTER |
        SPICOMMON_BUSFLAG_SCLK |
        SPICOMMON_BUSFLAG_MOSI |
        SPICOMMON_BUSFLAG_MISO;

    if (_quadAvailable) {
        bus.flags |= SPICOMMON_BUSFLAG_QUAD;
    }

    esp_err_t err = spi_bus_initialize(
        SPI_HOST,
        &bus,
        SPI_DMA_CH_AUTO
    );

    if (err != ESP_OK) {
        Serial.printf(
            "spi_bus_initialize failed: %s\n",
            esp_err_to_name(err)
        );
        return false;
    }
    _busInitialized = true;

    spi_device_interface_config_t dev = {};
    dev.clock_speed_hz = 30 * 1000 * 1000;
    dev.mode = 0;                   // SPI MODE0
    dev.spics_io_num = _pins.CS;    // CS управляется драйвером
    dev.queue_size = 4;
    // EVE reads are address -> dummy -> RX. QSPI also changes bus direction,
    // therefore the device must use separate TX and RX phases.
    dev.flags = SPI_DEVICE_HALFDUPLEX;

    err = spi_bus_add_device(
        SPI_HOST,
        &dev,
        &spiDevice
    );

    if (err != ESP_OK) {
        Serial.printf(
            "spi_bus_add_device failed: %s\n",
            esp_err_to_name(err)
        );
        spi_bus_free(SPI_HOST);
        _busInitialized = false;
        return false;
    }

    Serial.printf("SPI2 initialized at 30 MHz (%s)\n",
                  _quadAvailable ? "Single + Quad" : "Single only");
    return true;
}

bool EVE_spi::spiWrite(const void* data, size_t size){
    if (!spiDevice || !data || size == 0) return false;

    spi_transaction_t transaction = {};
    transaction.length = size * 8;      // длина указывается в битах
    transaction.tx_buffer = data;

    esp_err_t err = spi_device_transmit(
        spiDevice,
        &transaction
    );

    if (err != ESP_OK) {
        Serial.printf(
            "SPI write failed: %s\n",
            esp_err_to_name(err)
        );
        return false;
    }

    return true;
}

bool EVE_spi::spiTransfer(const void* txData, void* rxData, size_t size){
    spi_transaction_t transaction = {};

    transaction.length = size * 8;
    transaction.rxlength = size * 8;
    transaction.tx_buffer = txData;
    transaction.rx_buffer = rxData;

    esp_err_t err = spi_device_transmit(
        spiDevice,
        &transaction
    );

    return err == ESP_OK;
}

bool EVE_spi::spiWriteQuad(const void* data, size_t size){
    spi_transaction_t transaction = {};

    transaction.flags = SPI_TRANS_MODE_QIO;
    transaction.length = size * 8;
    transaction.tx_buffer = data;

    esp_err_t err = spi_device_transmit(
        spiDevice,
        &transaction
    );

    return err == ESP_OK;
}

uint8_t EVE_spi::rd8(uint32_t address){
    alignas(4) uint8_t data[4] = {};
    if (!readBlock(address, data, 1)) return 0;
    return data[0];
}

bool EVE_spi::setSPIMode(bool quad){
    if (!spiDevice) return false;

    if (quad && !_quadAvailable) {
        Serial.println("QSPI unavailable: IO2 and IO3 are disabled");
        return false;
    }

    if (quad == _quadMode) return true;

    if (quad) {
        // EVE is still in Single mode; this write must be sent on one line.
        alignas(4) uint8_t width[4] = {
            uint8_t(EVE_SPI_QUAD | EVE_SPI_DUMMY), 0, 0, 0
        };
        if (!transactionBlock(EVE_REG_SPI_WIDTH, width, nullptr, 1, false))
            return false;

        _quadMode = true;
        delay(1);

        uint8_t id = 0;
        if (!qrd8(EVE_REG_ID, id) || id != 0x7C) {
            Serial.printf("Switch to QSPI FAILED: REG_ID=0x%02X\n", id);
            return false; // Keep true: EVE was already commanded into Quad.
        }
        Serial.println("SPI mode: QUAD");
        return true;
    }

    // EVE currently accepts the transaction only over four data lines.
    alignas(4) uint8_t width[4] = { EVE_SPI_SINGLE, 0, 0, 0 };
    if (!transactionBlock(EVE_REG_SPI_WIDTH, width, nullptr, 1, true))
        return false;

    _quadMode = false;
    delay(1);

    uint8_t id = rd8(EVE_REG_ID);
    if (id != 0x7C) {
        Serial.printf("Switch to Single SPI FAILED: REG_ID=0x%02X\n", id);
        return false;
    }
    Serial.println("SPI mode: SINGLE");
    return true;
}

uint16_t EVE_spi::rd16(uint32_t address){
    alignas(4) uint8_t data[4] = {};
    if (!readBlock(address, data, 2)) return 0;
    return uint16_t(data[0]) | (uint16_t(data[1]) << 8);
}

uint32_t EVE_spi::rd32(uint32_t address){
    alignas(4) uint8_t data[4] = {};
    if (!readBlock(address, data, 4)) return 0;
    return uint32_t(data[0]) | (uint32_t(data[1]) << 8) |
           (uint32_t(data[2]) << 16) | (uint32_t(data[3]) << 24);
}

bool EVE_spi::hostCommand(uint8_t command, uint8_t parameter){
    if (!spiDevice) return false;

    uint8_t data[3] = {
        command,
        parameter,
        0x00
    };

    spi_transaction_t transaction = {};
    transaction.length = sizeof(data) * 8;
    transaction.tx_buffer = data;

    esp_err_t err = spi_device_transmit(
        spiDevice,
        &transaction
    );

    if (err != ESP_OK) {
        Serial.printf(
            "EVE hostCommand(0x%02X) failed: %s\n",
            command,
            esp_err_to_name(err)
        );
        return false;
    }

    return true;
}

bool EVE_spi::transactionBlock(uint32_t address, const void* txData,
                               void* rxData, size_t size, bool quad,
                               uint8_t dummyBytes){
    if (!spiDevice || size == 0 || (!txData && !rxData)) return false;
    if (quad && !_quadAvailable) {
        Serial.println("QSPI transaction rejected: IO2 and IO3 are disabled");
        return false;
    }

    spi_transaction_ext_t ext = {};
    ext.base.flags = SPI_TRANS_VARIABLE_ADDR;
    if (quad) {
        // QIO alone changes only the data phase. EVE also expects its complete
        // 24-bit memory header on IO0..IO3 after REG_SPI_WIDTH is set to Quad.
        ext.base.flags |= SPI_TRANS_MODE_QIO | SPI_TRANS_MULTILINE_ADDR;
    }
    if (dummyBytes) ext.base.flags |= SPI_TRANS_VARIABLE_DUMMY;

    ext.address_bits = 24;
    // dummy_bits is programmed as SPI clock cycles by the GP-SPI hardware.
    // One Quad clock transfers four bits, therefore one byte takes two clocks.
    ext.dummy_bits = dummyBytes * (quad ? 2 : 8);
    ext.base.addr = (address & 0x3FFFFFUL) | (txData ? 0x800000UL : 0);
    ext.base.length = txData ? size * 8 : 0;
    ext.base.rxlength = rxData ? size * 8 : 0;
    ext.base.tx_buffer = txData;
    ext.base.rx_buffer = rxData;

    esp_err_t err = spi_device_transmit(spiDevice, &ext.base);
    if (err != ESP_OK) {
        Serial.printf("EVE %s block 0x%06lX failed: %s\n",
                      quad ? "QSPI" : "SPI", (unsigned long)address,
                      esp_err_to_name(err));
        return false;
    }
    return true;
}

bool EVE_spi::writeBlock(uint32_t address, const void* data, size_t size){
    return transactionBlock(address, data, nullptr, size, _quadMode);
}

bool EVE_spi::readBlock(uint32_t address, void* data, size_t size){
    return transactionBlock(address, nullptr, data, size, _quadMode,
                            _quadMode ? 2 : 1);
}

bool EVE_spi::qwriteBlock(uint32_t address, const void* data, size_t size){
    return transactionBlock(address, data, nullptr, size, true);
}

bool EVE_spi::qreadBlock(uint32_t address, void* data, size_t size){
    // REG_SPI_WIDTH=0x06: normal dummy byte + one extra dummy byte.
    return transactionBlock(address, nullptr, data, size, true, 2);
}

bool EVE_spi::wr8(uint32_t address, uint8_t value){
    alignas(4) uint8_t b[4] = { value, 0, 0, 0 };
    return writeBlock(address, b, 1);
}

bool EVE_spi::wr16(uint32_t address, uint16_t value){
    uint8_t b[2] = { uint8_t(value), uint8_t(value >> 8) };
    return writeBlock(address, b, sizeof(b));
}

bool EVE_spi::wr32(uint32_t address, uint32_t value){
    uint8_t b[4] = { uint8_t(value), uint8_t(value >> 8),
                     uint8_t(value >> 16), uint8_t(value >> 24) };
    return writeBlock(address, b, sizeof(b));
}

bool EVE_spi::qwr8(uint32_t address, uint8_t value){
    alignas(4) uint8_t b[4] = { value, 0, 0, 0 };
    return qwriteBlock(address, b, 1);
}

bool EVE_spi::qwr16(uint32_t address, uint16_t value){
    uint8_t b[2] = { uint8_t(value), uint8_t(value >> 8) };
    return qwriteBlock(address, b, sizeof(b));
}

bool EVE_spi::qwr32(uint32_t address, uint32_t value){
    uint8_t b[4] = { uint8_t(value), uint8_t(value >> 8),
                     uint8_t(value >> 16), uint8_t(value >> 24) };
    return qwriteBlock(address, b, sizeof(b));
}

bool EVE_spi::qrd8(uint32_t address, uint8_t& value){
    alignas(4) uint8_t b[4] = {};
    if (!qreadBlock(address, b, 1)) return false;
    value = b[0];
    return true;
}

bool EVE_spi::qrd16(uint32_t address, uint16_t& value){
    uint8_t b[2];
    if (!qreadBlock(address, b, sizeof(b))) return false;
    value = uint16_t(b[0]) | (uint16_t(b[1]) << 8);
    return true;
}

bool EVE_spi::qrd32(uint32_t address, uint32_t& value){
    uint8_t b[4];
    if (!qreadBlock(address, b, sizeof(b))) return false;
    value = uint32_t(b[0]) | (uint32_t(b[1]) << 8) |
            (uint32_t(b[2]) << 16) | (uint32_t(b[3]) << 24);
    return true;
}

bool EVE_spi::benchmarkBlocks(const char* modeName){
    static constexpr uint32_t TEST_ADDRESS = 0x001000UL;
    static constexpr size_t BLOCK_SIZE = 4092;
    static constexpr uint32_t REPEATS = 256;

    uint8_t* saved = static_cast<uint8_t*>(
        heap_caps_malloc(BLOCK_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    uint8_t* pattern = static_cast<uint8_t*>(
        heap_caps_malloc(BLOCK_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    uint8_t* received = static_cast<uint8_t*>(
        heap_caps_malloc(BLOCK_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));

    if (!saved || !pattern || !received) {
        Serial.printf("%s speed test: memory allocation FAILED\n", modeName);
        if (saved) heap_caps_free(saved);
        if (pattern) heap_caps_free(pattern);
        if (received) heap_caps_free(received);
        return false;
    }

    for (size_t i = 0; i < BLOCK_SIZE; ++i)
        pattern[i] = uint8_t((i * 29U + 17U) ^ (i >> 3));

    bool ok = readBlock(TEST_ADDRESS, saved, BLOCK_SIZE);
    uint32_t completedWrites = 0;
    uint32_t completedReads = 0;

    uint32_t start = micros();
    if (ok) {
        for (; completedWrites < REPEATS; ++completedWrites) {
            if (!writeBlock(TEST_ADDRESS, pattern, BLOCK_SIZE)) {
                ok = false;
                break;
            }
        }
    }
    uint32_t writeUs = micros() - start;

    start = micros();
    if (ok) {
        for (; completedReads < REPEATS; ++completedReads) {
            if (!readBlock(TEST_ADDRESS, received, BLOCK_SIZE)) {
                ok = false;
                break;
            }
        }
    }
    uint32_t readUs = micros() - start;

    if (ok && memcmp(pattern, received, BLOCK_SIZE) != 0) ok = false;

    // Always attempt to restore the RAM_G region used by the benchmark.
    bool restored = writeBlock(TEST_ADDRESS, saved, BLOCK_SIZE);

    const uint64_t writeBytes = uint64_t(completedWrites) * BLOCK_SIZE;
    const uint64_t readBytes = uint64_t(completedReads) * BLOCK_SIZE;
    const float writeMBs = writeUs ? float(writeBytes) / float(writeUs) : 0.0f;
    const float readMBs = readUs ? float(readBytes) / float(readUs) : 0.0f;

    Serial.printf("%s block write:      %.3f MB/s  (%.3f Mbit/s)\n",
                  modeName, writeMBs, writeMBs * 8.0f);
    Serial.printf("%s block read:       %.3f MB/s  (%.3f Mbit/s)\n",
                  modeName, readMBs, readMBs * 8.0f);
    Serial.printf("%s benchmark data:   %llu bytes x2, block=%u\n",
                  modeName, (unsigned long long)(uint64_t(REPEATS) * BLOCK_SIZE),
                  unsigned(BLOCK_SIZE));

    heap_caps_free(saved);
    heap_caps_free(pattern);
    heap_caps_free(received);
    return ok && restored && completedWrites == REPEATS &&
           completedReads == REPEATS;
}

bool EVE_spi::selfTest(){
    static constexpr uint32_t TEST_SCALAR = 0x000100UL;
    static constexpr uint32_t TEST_BLOCK  = 0x000200UL;
    static constexpr size_t BLOCK_SIZE = 64;

    alignas(4) uint8_t savedScalar[8] = {};
    alignas(4) uint8_t savedBlock[BLOCK_SIZE] = {};
    alignas(4) uint8_t pattern[BLOCK_SIZE] = {};
    alignas(4) uint8_t received[BLOCK_SIZE] = {};

    for (size_t i = 0; i < BLOCK_SIZE; ++i)
        pattern[i] = uint8_t((i * 37U) ^ 0xA5U);

    Serial.println("----- EVE SPI self-test -----");

    bool scalarOK = readBlock(TEST_SCALAR, savedScalar, sizeof(savedScalar)) &&
                    wr8(TEST_SCALAR, 0xA5) && rd8(TEST_SCALAR) == 0xA5 &&
                    wr16(TEST_SCALAR + 2, 0x5AA5) && rd16(TEST_SCALAR + 2) == 0x5AA5 &&
                    wr32(TEST_SCALAR + 4, 0x1234ABCDUL) && rd32(TEST_SCALAR + 4) == 0x1234ABCDUL;
    writeBlock(TEST_SCALAR, savedScalar, sizeof(savedScalar));
    Serial.printf("SPI read/write:       %s\n", scalarOK ? "OK" : "FAILED");

    bool blockOK = readBlock(TEST_BLOCK, savedBlock, BLOCK_SIZE) &&
                   writeBlock(TEST_BLOCK, pattern, BLOCK_SIZE) &&
                   readBlock(TEST_BLOCK, received, BLOCK_SIZE) &&
                   memcmp(pattern, received, BLOCK_SIZE) == 0;
    writeBlock(TEST_BLOCK, savedBlock, BLOCK_SIZE);
    Serial.printf("SPI block:            %s\n", blockOK ? "OK" : "FAILED");
    if (!scalarOK || !blockOK) return false;

    bool singleSpeedOK = benchmarkBlocks("Single SPI");
    if (!singleSpeedOK) return false;

    if (!_quadAvailable) {
        Serial.println("QSPI tests:           SKIPPED (Single-only wiring)");
        Serial.println("-----------------------------");
        return true;
    }

    if (!setSPIMode(true)) {
        Serial.println("QSPI enable:          FAILED");
        return false;
    }

    uint8_t qid = 0;
    bool qLinkOK = qrd8(EVE_REG_ID, qid) && qid == 0x7C;
    Serial.printf("QSPI REG_ID:          %s (0x%02X)\n",
                  qLinkOK ? "OK" : "FAILED", qid);

    bool qScalarOK = false;
    bool qBlockOK = false;
    if (qLinkOK) {
        qScalarOK = qwr8(TEST_SCALAR, 0x3C) && qrd8(TEST_SCALAR, qid) && qid == 0x3C &&
                    qwr16(TEST_SCALAR + 2, 0xC33C) &&
                    qwr32(TEST_SCALAR + 4, 0x89ABCDEFUL);
        uint16_t q16 = 0;
        uint32_t q32 = 0;
        qScalarOK = qScalarOK && qrd16(TEST_SCALAR + 2, q16) && q16 == 0xC33C &&
                    qrd32(TEST_SCALAR + 4, q32) && q32 == 0x89ABCDEFUL;
        qwriteBlock(TEST_SCALAR, savedScalar, sizeof(savedScalar));

        memset(received, 0, sizeof(received));
        qBlockOK = qwriteBlock(TEST_BLOCK, pattern, BLOCK_SIZE) &&
                   qreadBlock(TEST_BLOCK, received, BLOCK_SIZE) &&
                   memcmp(pattern, received, BLOCK_SIZE) == 0;
        qwriteBlock(TEST_BLOCK, savedBlock, BLOCK_SIZE);
    }
    Serial.printf("QSPI read/write:      %s\n", qScalarOK ? "OK" : "FAILED");
    Serial.printf("QSPI block:           %s\n", qBlockOK ? "OK" : "FAILED");

    bool quadSpeedOK = qLinkOK && qScalarOK && qBlockOK &&
                       benchmarkBlocks("Quad SPI");

    bool singleAgain = setSPIMode(false);
    Serial.printf("Return to Single SPI: %s\n", singleAgain ? "OK" : "FAILED");
    Serial.println("-----------------------------");

    return qLinkOK && qScalarOK && qBlockOK && quadSpeedOK && singleAgain;
}
