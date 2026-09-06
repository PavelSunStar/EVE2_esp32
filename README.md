# EVE2_esp32

Arduino library for driving Bridgetek EVE2/BT81x graphics controllers from an
ESP32. It provides single- and quad-SPI transport, display-list helpers,
co-processor commands, RAM_G uploads, bitmap rendering, and common VGA timing
presets.

The library is currently aimed at BT815, BT816, and BT817 based boards. It was
developed with the VM816C50A module and the Arduino ESP32 core.

> [!IMPORTANT]
> This is an early release. Verify the pinout, I/O voltage, and display timing
> against your module and panel datasheets before powering the hardware.

## Features

- ESP32 Arduino API with no third-party runtime dependencies
- Single SPI and optional quad-SPI operation
- EVE display-list and co-processor command helpers
- Buffered command submission with timeout and fault detection
- Block transfers to and from EVE memory
- VGA timing presets from 640x350 through 800x600
- Processing-based image converter for RGB565, ARGB1555, RGB332, and L8 assets
- Examples for text rendering and animated sprites

## Requirements

- An ESP32 board supported by Arduino-ESP32 3.x
- A Bridgetek BT815, BT816, or BT817 based display module
- Arduino IDE 2.x or Arduino CLI
- Two additional data pins (`IO2` and `IO3`) when quad SPI is used

## Installation

Until the library is available through Arduino Library Manager, install it from
GitHub:

1. Download this repository as a ZIP file.
2. In Arduino IDE, select **Sketch > Include Library > Add .ZIP Library**.
3. Open **File > Examples > EVE2_esp32 > HelloWorld**.

Alternatively, clone the repository into your Arduino `libraries` directory.

## Wiring

The constructor supplies defaults matching the original development setup:

| EVE signal | Default ESP32 GPIO | Purpose |
| --- | ---: | --- |
| `PD` | 1 | Active-low power-down/reset |
| `CS` | 3 | Chip select |
| `SCK` | 20 | SPI clock |
| `MOSI / IO0` | 5 | SPI data 0 |
| `MISO / IO1` | 4 | SPI data 1 |
| `IO2` | 32 | Quad-SPI data 2 |
| `IO3` | 33 | Quad-SPI data 3 |

These pins are not universal. Set the pins for your board before calling
`init()`:

```cpp
GD.setPins(pinPD, pinCS, pinSCK, pinMOSI, pinMISO, pinIO2, pinIO3);
```

Pass `-1` for `pinIO2` and `pinIO3` if quad SPI is not wired. Use 3.3 V logic
and follow the power requirements of your particular EVE module.

## Quick start

```cpp
#include <EVE_esp32.h>

EVE_esp32 GD;

void setup() {
    Serial.begin(115200);

    GD.setPins(1, 3, 20, 5, 4, 32, 33);
    if (!GD.init(MODE_640x480_57)) {
        Serial.println("EVE initialization failed");
        while (true) delay(100);
    }

    if (!GD.beginFrame()) {
        Serial.println("Could not start frame");
        return;
    }

    GD.ClearColorRGB(0x10, 0x30, 0x00);
    GD.Clear();
    GD.cmd_text(GD.w / 2, GD.h / 2, 31, OPT_CENTER, "Hello world");

    if (!GD.swap()) {
        Serial.println("Frame submission failed");
    }
}

void loop() {}
```

Call `setSPIMode(true)` after a successful `init()` to request quad SPI. The
method returns `false` when the extra data pins are unavailable or the mode
switch fails.

## Display modes

The following presets are declared in `config.h`:

| Preset | Resolution | Nominal refresh |
| --- | ---: | ---: |
| `MODE640x350_70` | 640x350 | 70 Hz |
| `MODE640x400_70` | 640x400 | 70 Hz |
| `MODE720x400_70` | 720x400 | 70 Hz |
| `MODE_640x480_57` | 640x480 | 57 Hz |
| `MODE640x480_60` | 640x480 | 60 Hz |
| `MODE800x600_56` | 800x600 | 56 Hz |
| `MODE800x600_60` | 800x600 | 60 Hz |

You can pass a custom `Mode` to `init()` or `setVGAMode()`. Display timings are
hardware-specific; an incorrect mode can produce no picture or an unstable
picture.

## Frame lifecycle

For dynamic graphics, use this sequence in `loop()`:

```cpp
if (GD.beginFrame()) {
    // Add display-list and co-processor commands here.
    GD.swap();
}
```

`beginFrame()` clears the command buffer and starts a new display list.
`sendCMD()` can flush a partial command batch, while `swap()` finishes the list,
submits it, and waits for the EVE co-processor. Both methods accept a timeout in
milliseconds and report failure through their return value.

## Image converter

`tools/EVE_Converter/EVE_Converter.pde` is a Processing 4 sketch that turns
PNG, JPG, BMP, GIF, or TIFF images into an Arduino header containing a
zlib-compressed `CMD_INFLATE` stream.

1. Install [Processing 4](https://processing.org/) and the **ControlP5** library.
2. Open `tools/EVE_Converter/EVE_Converter.pde` and run it.
3. Select an image, pixel format, and conversion mode.
4. Include the generated `*_assets.h` file in your sketch.
5. Call its generated `LOAD_*()` macro after `GD.init()`.

In **Cells** mode all images must have identical dimensions. In **Separate**
mode images may differ in size, with a maximum of 32 bitmap handles. Generated
assets are loaded from the start of `RAM_G`, so plan other RAM_G allocations
accordingly.

See the `Sprites` example for a complete asset-loading workflow.

## API overview

- Setup: `setPins()`, `init()`, `setVGAMode()`, `setSPIMode()`
- Frames: `beginFrame()`, `sendCMD()`, `waitCMD()`, `swap()`
- Memory access: `rd8/16/32()`, `wr8/16/32()`, `readBlock()`, `writeBlock()`
- Raw commands: `dl()`, `cmd8/16/32()`, `cs()`
- Generated assets: `copy()`
- Status: `isQuadMode()`, `isQuadAvailable()`, public dimensions `w` and `h`

Display-list and widget helpers follow Bridgetek EVE command names, for example
`ClearColorRGB`, `Vertex2ii`, `cmd_text`, `cmd_button`, and `cmd_gradient`.
Consult the BT81x programming guide for command semantics and limits.

## Contributing

Bug reports and pull requests are welcome. Please include the ESP32 board,
Arduino-ESP32 version, EVE controller/module, wiring, display mode, and serial
log when reporting hardware issues. See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

Released under the [MIT License](LICENSE).
