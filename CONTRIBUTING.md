# Contributing

Thank you for helping improve EVE2_esp32.

## Reporting a problem

Before opening an issue, test the latest revision and include:

- ESP32 board and Arduino-ESP32 core version
- EVE controller and module name
- display model and timing preset
- complete wiring, including whether quad SPI is enabled
- a minimal sketch and the complete serial log

Never connect a module until its logic level and power requirements have been
verified against the manufacturer's datasheet.

## Pull requests

1. Keep changes focused and preserve the existing public API unless a breaking
   change is intentional and documented.
2. Add or update an example when introducing user-facing behavior.
3. Compile all examples for an ESP32 target.
4. Update `README.md` and `CHANGELOG.md` when appropriate.
5. Use clear commit messages and explain how the change was tested.

Code should remain compatible with the Arduino ESP32 core and avoid unnecessary
runtime dependencies.
