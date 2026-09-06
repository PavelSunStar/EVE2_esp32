#include "EVE_esp32.h"
#include <cstring>

EVE_esp32::EVE_esp32(){
    _pins.PD    = PIN_PD;
    _pins.CS    = PIN_CS;
    _pins.SCK   = PIN_SCK;
    _pins.MOSI  = PIN_MOSI;
    _pins.MISO  = PIN_MISO;
    _pins.IO2   = PIN_IO2;
    _pins.IO3   = PIN_IO3;

    initLUT();    
}

void EVE_esp32::destroy(){
    if (_cmd.buf){
        free(_cmd.buf);
        _cmd.buf = nullptr;
    }

    if (_dl.buf){
        free(_dl.buf);
        _dl.buf = nullptr;
    }
        
    _inited = false;
}

void EVE_esp32::setPins(int8_t pinPD, int8_t pinCS, int8_t pinSCK, int8_t pinMOSI, int8_t pinMISO, int8_t pinIO2, int8_t pinIO3){
    _pins.PD    = pinPD;
    _pins.CS    = pinCS;
    _pins.SCK   = pinSCK;
    _pins.MOSI  = pinMOSI;
    _pins.MISO  = pinMISO;
    _pins.IO2   = pinIO2;
    _pins.IO3   = pinIO3;
    _spi.setPins(_pins);
}

bool EVE_esp32::setSPIMode(bool quad){
    if (!_inited) {
        Serial.println("setSPIMode failed: EVE is not initialized");
        return false;
    }

    if (!_spi.setSPIMode(quad)) return false;
    _quadMode = _spi.isQuadMode();
    return true;
}

uint8_t EVE_esp32::rd8(uint32_t address){
    return _spi.rd8(address);
}

uint16_t EVE_esp32::rd16(uint32_t address){
    return _spi.rd16(address);
}

uint32_t EVE_esp32::rd32(uint32_t address){
    return _spi.rd32(address);
}

bool EVE_esp32::wr8(uint32_t address, uint8_t value){
    return _spi.wr8(address, value);
}

bool EVE_esp32::wr16(uint32_t address, uint16_t value){
    return _spi.wr16(address, value);
}

bool EVE_esp32::wr32(uint32_t address, uint32_t value){
    return _spi.wr32(address, value);
}

bool EVE_esp32::readBlock(uint32_t address, void* data, size_t size){
    return _spi.readBlock(address, data, size);
}

bool EVE_esp32::writeBlock(uint32_t address, const void* data, size_t size){
    return _spi.writeBlock(address, data, size);
}

bool EVE_esp32::putDL8(uint8_t data){
    if (!_dl.buf) {
        Serial.println("DL buffer is not allocated");
        return false;
    }
    if (_dl.pos >= RAM_DL_SIZE) {
        Serial.println("DL buffer overflow");
        return false;
    }

    reinterpret_cast<uint8_t*>(_dl.buf)[_dl.pos++] = data;
    return true;
}

bool EVE_esp32::putCMD8(uint8_t data){
    if (!_cmd.buf) {
        Serial.println("CMD buffer is not allocated");
        return false;
    }
    if (_cmd.pos >= CMD_BUF_SIZE) {
        Serial.println("CMD buffer overflow");
        _cmdError = true;
        return false;
    }

    reinterpret_cast<uint8_t*>(_cmd.buf)[_cmd.pos++] = data;
    return true;
}

void EVE_esp32::dl(uint32_t command){
    if (!_dl.buf || _dl.pos + 4 > RAM_DL_SIZE) {
        Serial.println("DL buffer overflow");
        return;
    }

    putDL8(uint8_t(command));
    putDL8(uint8_t(command >> 8));
    putDL8(uint8_t(command >> 16));
    putDL8(uint8_t(command >> 24));
}

void EVE_esp32::cmd8(uint8_t data){
    putCMD8(data);
}

void EVE_esp32::cmd16(uint16_t data){
    if (!_cmd.buf || _cmd.pos + 2 > CMD_BUF_SIZE) {
        Serial.println("CMD buffer overflow");
        _cmdError = true;
        return;
    }

    putCMD8(uint8_t(data));
    putCMD8(uint8_t(data >> 8));
}

void EVE_esp32::cmd32(uint32_t data){
    if (!_cmd.buf || _cmd.pos + 4 > CMD_BUF_SIZE) {
        Serial.println("CMD buffer overflow");
        _cmdError = true;
        return;
    }

    putCMD8(uint8_t(data));
    putCMD8(uint8_t(data >> 8));
    putCMD8(uint8_t(data >> 16));
    putCMD8(uint8_t(data >> 24));
}

void EVE_esp32::cs(const char* s){
    if (!s) s = "";

    const size_t length = strlen(s) + 1; // Include the terminating zero.
    const size_t padded = (length + 3U) & ~size_t(3U);
    if (!_cmd.buf || _cmd.pos + padded > CMD_BUF_SIZE) {
        Serial.println("CMD string does not fit in buffer");
        _cmdError = true;
        return;
    }

    while (*s) putCMD8(uint8_t(*s++));
    putCMD8(0);
    while (_cmd.pos & 3U) putCMD8(0);
}

uint16_t EVE_esp32::cmdSpace(){
    return _spi.rd16(REG_CMDB_SPACE);
}

bool EVE_esp32::beginFrame(void* bg, uint8_t colorMode)
{
    if (!_inited || !_cmd.buf) {
        Serial.println("beginFrame failed: EVE is not ready");
        return false;
    }

    const int bgWidth  = w >> 1;
    const int bgHeight = h >> 1;

    // Адрес в RAM_G, который не используется другими ресурсами
    constexpr uint32_t BG_ADDRESS = RAM_G;

    if (bg) {
        const uint8_t bytesPerPixel =
            (colorMode == RGB565 ||
             colorMode == ARGB1555 ||
             colorMode == ARGB4) ? 2 : 1;

        const uint32_t bgSize =
            uint32_t(bgWidth) *
            uint32_t(bgHeight) *
            bytesPerPixel;

        // Загрузить буфер ESP32 в видеопамять EVE
        if (!uploadRAM(BG_ADDRESS, bg, bgSize)) {
            Serial.println("Background upload FAILED");
            return false;
        }
    }

    _cmd.pos = 0;
    _cmdError = false;

    cmd_dlstart();

    ClearColorRGB(0, 0, 0);
    Clear(true, true, true);

    if (bg) {
        SaveContext();

        BitmapHandle(0);
        BitmapSource(BG_ADDRESS);

        BitmapLayout(
            colorMode,
            bgWidth * (
                colorMode == RGB565 ||
                colorMode == ARGB1555 ||
                colorMode == ARGB4 ? 2 : 1
            ),
            bgHeight
        );

        /*
         * Матрица bitmap увеличивает изображение в 2 раза.
         * 2.0 в формате 16.16 = 2 * 65536.
         */
        cmd_loadidentity();
        cmd_scale(2 * 65536, 2 * 65536);
        cmd_setmatrix();

        BitmapSize(
            NEAREST,
            BORDER,
            BORDER,
            w,
            h
        );

        Begin(BITMAPS);
        Vertex2ii(0, 0, 0, 0);
        End();

        RestoreContext();
    }

    return !_cmdError;
}

bool EVE_esp32::sendCMD(uint32_t timeoutMs){
    if (!_inited || !_cmd.buf || _cmdError) {
        Serial.println("sendCMD failed: invalid CMD buffer");
        return false;
    }
    if (_cmd.pos == 0) return true;
    if ((_cmd.pos & 3U) != 0 || _cmd.pos > CMD_BUF_SIZE) {
        Serial.printf("sendCMD failed: invalid size=%lu\n",
                      (unsigned long)_cmd.pos);
        return false;
    }

    const uint32_t start = millis();
    while (true) {
        const uint16_t space = cmdSpace();

        // REG_CMDB_SPACE bit 0 is set when the coprocessor is in fault state.
        if (space & 1U) {
            Serial.printf("EVE coprocessor fault: REG_CMDB_SPACE=0x%04X\n", space);
            return false;
        }
        if ((space & 0x0FFCU) >= _cmd.pos) break;

        if (millis() - start >= timeoutMs) {
            Serial.printf("EVE CMD space timeout: need=%lu space=%u\n",
                          (unsigned long)_cmd.pos,
                          unsigned(space & 0x0FFCU));
            return false;
        }
        delay(1);
    }

    if (!_spi.writeBlock(REG_CMDB_WRITE, _cmd.buf, _cmd.pos)) {
        Serial.println("EVE CMD write FAILED");
        return false;
    }

    _cmd.pos = 0;
    return true;
}

bool EVE_esp32::waitCMD(uint32_t timeoutMs){
    const uint32_t start = millis();
    while (true) {
        const uint16_t space = cmdSpace();
        if (space & 1U) {
            Serial.printf("EVE coprocessor fault: REG_CMDB_SPACE=0x%04X\n", space);
            return false;
        }
        if ((space & 0x0FFCU) == CMD_BUF_SIZE) return true;

        if (millis() - start >= timeoutMs) {
            Serial.printf("EVE CMD finish timeout: space=%u\n",
                          unsigned(space & 0x0FFCU));
            return false;
        }
        delay(1);
    }
}

bool EVE_esp32::swap(uint32_t timeoutMs)
{
    if (!_inited || _cmdError) {
        return false;
    }

    Display();
    cmd_swap();

    if (_cmdError) {
        return false;
    }

    const bool ok =
        sendCMD(timeoutMs) &&
        waitCMD(timeoutMs);

    _cmd.pos = 0;
    _cmdError = false;

    return ok;
}

bool EVE_esp32::setVGAMode(const Mode& mode){
    _spi.wr8(REG_PCLK, 0);
    _spi.wr8(REG_SWIZZLE, 0);
    _spi.wr8(REG_CSPREAD, 1);
    _spi.wr8(REG_DITHER, 1);
    _spi.wr8(REG_ROTATE, 0);
    _spi.wr8(REG_PCLK_POL, 0);

    _spi.wr16(REG_HSIZE, mode.hRes);
    _spi.wr16(REG_HCYCLE, mode.hCycle);
    _spi.wr16(REG_HOFFSET, mode.hOffset);
    _spi.wr16(REG_HSYNC0, mode.hSync0);
    _spi.wr16(REG_HSYNC1, mode.hSync1);

    _spi.wr16(REG_VSIZE, mode.vRes);
    _spi.wr16(REG_VCYCLE, mode.vCycle);
    _spi.wr16(REG_VOFFSET, mode.vOffset);
    _spi.wr16(REG_VSYNC0, mode.vSync0);
    _spi.wr16(REG_VSYNC1, mode.vSync1);

    _spi.wr32(RAM_DL + 0, DL_CLEAR_COLOR_RGB | 0x00FF00UL);
    _spi.wr32(RAM_DL + 4, DL_CLEAR | 0x07UL);
    _spi.wr32(RAM_DL + 8, DL_DISPLAY);
    _spi.wr8(REG_DLSWAP, DLSWAP_FRAME);

    uint32_t start = millis();
    while (_spi.rd8(REG_DLSWAP) != DLSWAP_DONE){
        if (millis() - start >= 1000){
            Serial.println("VGA DLSWAP timeout");
            return false;
        }
        delay(1);
    }

    _spi.wr8(REG_PCLK, mode.div);
    Serial.printf("VGA ready: %ux%u PCLK divider=%u\n", mode.hRes, mode.vRes, mode.div);

    return true;
}

bool EVE_esp32::getMemory(){
    auto& d = _dl;
    if (d.buf){
        free(d.buf);
        d.buf = nullptr;
    }

    d.buf = malloc(RAM_DL_SIZE);
    if (!d.buf){
        Serial.printf("DL memory allocation failed: %u bytes\n", (unsigned)RAM_DL_SIZE);
        return false;
    } else {
        d.pos = 0;
        memset(d.buf, 0, RAM_DL_SIZE);
        Serial.printf("DL memory: %p, size=%u bytes\n", d.buf, (unsigned)RAM_DL_SIZE);    
    }

    auto& c = _cmd;
    if (c.buf){
        free(c.buf);
        c.buf = nullptr;
    }

    c.buf = malloc(RAM_CMD_SIZE);
    if (!c.buf){
        Serial.printf("CMD memory allocation failed: %u bytes\n", (unsigned)RAM_CMD_SIZE);
        return false;
    } else {
        c.pos = 0;
        memset(c.buf, 0, RAM_CMD_SIZE);
        Serial.printf("CMD memory: %p, size=%u bytes\n", c.buf, (unsigned)RAM_CMD_SIZE);    
    }

    return true;
}

bool EVE_esp32::init(const Mode& mode){
    destroy();

    auto& p = _pins;
    _spi.setPins(_pins);

    if (!_spi.begin()){
        Serial.println("EVE init FAILED");
        return false;
    }
    _quadMode = _spi.isQuadMode();

    if (!getMemory()) return false;

    if (!setVGAMode(mode)) return false;
    w = _spi.rd16(REG_HSIZE);
    h = _spi.rd16(REG_VSIZE);    

    return (_inited = true);
}

// Virtual Screen---------------------------------------------------
void* EVE_esp32::allocateMemory(size_t request, bool psram){
    if (request == 0) return nullptr;

    if (psram && !psramFound()) {
        Serial.println("allocateMemory: PSRAM not present");
        return nullptr;
    }

    constexpr size_t align = 32;
    const size_t alignedSize = ALIGN_UP(request, align);

    const uint32_t caps = psram
        ? (MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
        : (MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);

    void* buffer = heap_caps_aligned_calloc(
        align,
        1,
        alignedSize,
        caps
    );

    if (!buffer) {
        Serial.printf(
            "allocateMemory FAILED: request=%u aligned=%u caps=0x%X\n",
            (unsigned)request,
            (unsigned)alignedSize,
            (unsigned)caps
        );
        return nullptr;
    }

    Serial.printf(
        "allocateMemory: ptr=%p request=%u aligned=%u caps=0x%X\n",
        buffer,
        (unsigned)request,
        (unsigned)alignedSize,
        (unsigned)caps
    );

    return buffer;
}

uint32_t EVE_esp32::rgb332to888(uint8_t color)
{
    uint8_t r = (color >> 5) & 0x07;
    uint8_t g = (color >> 2) & 0x07;
    uint8_t b = color & 0x03;

    r = (r << 5) | (r << 2) | (r >> 1);
    g = (g << 5) | (g << 2) | (g >> 1);
    b = (b << 6) | (b << 4) | (b << 2) | b;

    return 0xFF000000UL |
           (uint32_t(r) << 16) |
           (uint32_t(g) << 8) |
            uint32_t(b);
}

bool EVE_esp32::uploadRAM(uint32_t address, const void* data, size_t size){
    if (!data || size == 0) return false;

    constexpr size_t BLOCK_SIZE = 4092;
    const uint8_t* source = static_cast<const uint8_t*>(data);

    while (size > 0) {
        const size_t block = (size > BLOCK_SIZE) ? BLOCK_SIZE : size;

        if (!_spi.writeBlock(address, source, block)) {
            Serial.printf(
                "uploadRAM FAILED: address=0x%06lX block=%u remaining=%u\n",
                (unsigned long)address,
                unsigned(block),
                unsigned(size)
            );

            return false;
        }

        address += block;
        source += block;
        size -= block;
    }

    return true;
}

bool EVE_esp32::copy(const uint8_t* src, int count){
    if (!src || count <= 0) return false;

    // Файл ресурсов содержит готовый поток команд EVE:
    // настройки bitmap, CMD_INFLATE и сжатые данные.
    for (int i = 0; i < count; i++) {
        cmd8(pgm_read_byte(src + i));
    }

    if (_cmdError) {
        Serial.println("Asset command buffer overflow");
        return false;
    }

    return sendCMD(3000) && waitCMD(3000);
}

