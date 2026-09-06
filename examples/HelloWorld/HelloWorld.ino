#include <EVE_esp32.h>

EVE_esp32 GD;

void setup(){
    Serial.begin(115200);
    while(!Serial){;}

    GD.setPins(1, 3, 20, 5, 4, 32, 33);
    if (!GD.init(MODE_640x480_57)){
        Serial.println("GD3X init FAILED");
        while (1) delay(100);
    }  

    GD.ClearColorRGB(0x103000);
    GD.Clear();
    GD.cmd_text(GD.w / 2, GD.h / 2, 31, OPT_CENTER, "Hello world");
    GD.swap();      
}

void loop(){

}