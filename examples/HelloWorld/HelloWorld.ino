#include <EVE_esp32.h>

EVE_esp32 GD;

void setup(){
    Serial.begin(115200);
    while(!Serial){;}

    GD.setPins(1, 10, 12, 11, 13, -1, -1);
    if (!GD.init(MODE640x480_57)) {
        Serial.println("EVE init FAILED");
        while (1) delay(100);
    }

    if (!GD.beginFrame()) {
        Serial.println("beginFrame FAILED");
        return;
    }

    GD.ClearColorRGB(0x103000);
    GD.Clear();
    GD.cmd_text(GD.w / 2, GD.h / 2, 31, OPT_CENTER, "Hello world");
    if (!GD.swap()) {
        Serial.println("swap FAILED");
    }
}

void loop(){

}
