#include "../EVE_esp32.h"

void EVE_esp32::ClearColorRGB(uint8_t r, uint8_t g, uint8_t b){
    ClearColorRGB(
        ((uint32_t)r << 16) |
        ((uint32_t)g << 8) |
        b
    );
}

void EVE_esp32::ColorRGB(uint8_t r, uint8_t g, uint8_t b){
    cmd32(DL_COLOR_RGB |
        ((uint32_t)r << 16) |
        ((uint32_t)g << 8) |
        (uint32_t)b);
}

void EVE_esp32::ColorRGB(uint32_t color){
    cmd32(DL_COLOR_RGB |
        (color & 0x00FFFFFFUL)
    );
}

void EVE_esp32::Clear(bool c, bool s, bool t){ 
    cmd32(DL_CLEAR | 
        ((uint32_t)c << 2) | 
        ((uint32_t)s << 1) | 
        ((uint32_t)t)); 
}

void EVE_esp32::Vertex2p(int16_t x, int16_t y){
    Vertex2f(
        (int16_t)((int32_t)x << 4), //_vxf),
        (int16_t)((int32_t)y << 4) //_vxf)
    );
}

void EVE_esp32::Vertex2f(int16_t x, int16_t y){
    cmd32(DL_VERTEX2F |
        ((x & 0x7FFFUL) << 15) |
        (y & 0x7FFFUL));

}

void EVE_esp32::Vertex2ii(uint16_t x, uint16_t y, uint8_t handle, uint8_t cell){
    cmd32(DL_VERTEX2II |
        ((uint32_t)(x & 0x1FF) << 21) |
        ((uint32_t)(y & 0x1FF) << 12) |
        ((uint32_t)(handle & 0x1F) << 7) |
        ((uint32_t)(cell & 0x7F)));
}

void EVE_esp32::cmd_text(int16_t x, int16_t y, int16_t font, uint16_t options, const char* s){
    if (!s) return;

    cmd32(CMD_TEXT);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16((uint16_t)font);

    cmd16(options);
    cs(s);
}

void EVE_esp32::cmd_button(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, uint16_t options, const char* s){
    if (!s) return;

    cmd32(CMD_BUTTON);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16(w);
    cmd16(h);
    cmd16((uint16_t)font);

    cmd16(options);
    cs(s);
}

void EVE_esp32::cmd_toggle(int16_t x, int16_t y, int16_t w, int16_t font, uint16_t options, uint16_t state, const char* s){
    if (!s) return;

    cmd32(CMD_TOGGLE);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16((uint16_t)w);
    cmd16((uint16_t)font);

    cmd16(options);
    cmd16(state);
    cs(s);
}

void EVE_esp32::cmd_slider(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t options, uint16_t val, uint16_t range){
    cmd32(CMD_SLIDER);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16(w);
    cmd16(h);

    cmd16(options);
    cmd16(val);
    cmd16(range);
    cmd16(0);
}

void EVE_esp32::cmd_progress(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t options, uint16_t val, uint16_t range){
    cmd32(CMD_PROGRESS);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16(w);
    cmd16(h);

    cmd16(options);
    cmd16(val);
    cmd16(range);
    cmd16(0);
}

void EVE_esp32::cmd_gauge(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t major, uint16_t minor, uint16_t val, uint16_t range){
    cmd32(CMD_GAUGE);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16((uint16_t)r);
    cmd16(options);

    cmd16(major);
    cmd16(minor);
    cmd16(val);
    cmd16(range);
}

void EVE_esp32::cmd_clock(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t h, uint16_t m, uint16_t s, uint16_t ms){
    cmd32(CMD_CLOCK);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16((uint16_t)r);
    cmd16(options);

    cmd16(h);
    cmd16(m);
    cmd16(s);
    cmd16(ms);
}

void EVE_esp32::cmd_dial(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t val){
    cmd32(CMD_DIAL);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16((uint16_t)r);
    cmd16(options);

    cmd16(val);
    cmd16(0);
}

void EVE_esp32::cmd_scrollbar(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t options, uint16_t val, uint16_t size, uint16_t range){
    cmd32(CMD_SCROLLBAR);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16(w);
    cmd16(h);

    cmd16(options);
    cmd16(val);
    cmd16(size);
    cmd16(range);
}

void EVE_esp32::cmd_keys(int16_t x, int16_t y, uint16_t w, uint16_t h, int16_t font, uint16_t options, const char* s){
    if (!s) return;

    cmd32(CMD_KEYS);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16(w);
    cmd16(h);

    cmd16((uint16_t)font);
    cmd16(options);

    cs(s);
}

void EVE_esp32::cmd_number(int16_t x, int16_t y, int16_t font, uint16_t options, int32_t n){
    cmd32(CMD_NUMBER);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16((uint16_t)font);
    cmd16(options);

    cmd32((uint32_t)n);
}

void EVE_esp32::cmd_gradient(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1){
    cmd32(CMD_GRADIENT);

    cmd16((uint16_t)x0);
    cmd16((uint16_t)y0);
    cmd32(rgb0);

    cmd16((uint16_t)x1);
    cmd16((uint16_t)y1);
    cmd32(rgb1);
}

void EVE_esp32::cmd_spinner(int16_t x, int16_t y, uint16_t style, uint16_t scale){
    cmd32(CMD_SPINNER);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16(style);
    cmd16(scale);
}

void EVE_esp32::cmd_sketch(int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t ptr, uint16_t format){
    cmd32(CMD_SKETCH);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16(w);
    cmd16(h);

    cmd32(ptr);

    cmd16(format);
    cmd16(0);
}

void EVE_esp32::cmd_memzero(uint32_t ptr, uint32_t num){
    cmd32(CMD_MEMZERO);
    cmd32(ptr);
    cmd32(num);
}

void EVE_esp32::BitmapLayout(uint8_t format, uint16_t linestride, uint16_t height){
    cmd32(DL_BITMAP_LAYOUT |
        ((uint32_t)(format & 0x1F) << 19) |
        ((uint32_t)(linestride & 0x3FF) << 9) |
        (height & 0x1FF));

    BitmapLayoutH(linestride, height);
}

void EVE_esp32::BitmapSize(uint8_t filter, uint8_t wrapx, uint8_t wrapy,
                      uint16_t width, uint16_t height){
    cmd32(DL_BITMAP_SIZE |
        ((uint32_t)(filter & 1) << 20) |
        ((uint32_t)(wrapx & 1) << 19) |
        ((uint32_t)(wrapy & 1) << 18) |
        ((uint32_t)(width & 0x1FF) << 9) |
        (height & 0x1FF));

    BitmapSizeH(width, height);
}

void EVE_esp32::BitmapLayoutH(uint16_t linestride, uint16_t height){
    cmd32(DL_BITMAP_LAYOUT_H |
        ((uint32_t)((linestride >> 10) & 3) << 2) |
        ((uint32_t)((height >> 9) & 3)));
}

void EVE_esp32::BitmapSizeH(uint16_t width, uint16_t height){
    cmd32(DL_BITMAP_SIZE_H |
        ((uint32_t)((width >> 9) & 3) << 2) |
        ((uint32_t)((height >> 9) & 3)));
}

void EVE_esp32::cmd_translate(int32_t tx, int32_t ty){
    cmd32(CMD_TRANSLATE);
    cmd32((uint32_t)tx);
    cmd32((uint32_t)ty);
}

void EVE_esp32::cmd_rotate(int32_t angle){
    cmd32(CMD_ROTATE);
    cmd32((uint32_t)angle);
}

void EVE_esp32::cmd_rotatearound(int32_t x, int32_t y, uint32_t angle, int32_t scale){
    cmd32(CMD_ROTATEAROUND);
    cmd32((uint32_t)x);
    cmd32((uint32_t)y);
    cmd32(angle);
    cmd32((uint32_t)scale);
}

void EVE_esp32::ColorMask(bool r, bool g, bool b, bool a){
    cmd32(DL_COLOR_MASK |
        ((uint32_t)r << 3) |
        ((uint32_t)g << 2) |
        ((uint32_t)b << 1) |
        (uint32_t)a);
}

void EVE_esp32::BlendFunc(uint8_t src, uint8_t dst){
    cmd32(DL_BLEND_FUNC |
        ((uint32_t)(src & 7) << 3) |
        (uint32_t)(dst & 7));
}

void EVE_esp32::cmd_loadimage(uint32_t ptr, uint32_t options){
    cmd32(CMD_LOADIMAGE);
    cmd32(ptr);
    cmd32(options);
}

void EVE_esp32::cmd_scale(int32_t sx, int32_t sy){
    cmd32(CMD_SCALE);
    cmd32((uint32_t)sx);
    cmd32((uint32_t)sy);
}

void EVE_esp32::ScissorXY(uint16_t x, uint16_t y){
    cmd32(DL_SCISSOR_XY |
          ((uint32_t)(x & 0x7FF) << 11) |
          (y & 0x7FF));
}

void EVE_esp32::ScissorSize(uint16_t width, uint16_t height){
    cmd32(DL_SCISSOR_SIZE |
          ((uint32_t)(width & 0xFFF) << 12) |
          (height & 0xFFF));
}

void EVE_esp32::VertexTranslateX(int32_t x){
    cmd32(
        DL_VERTEX_TRANSLATE_X |
        ((uint32_t)x & 0x1FFFFUL)
    );
}

void EVE_esp32::VertexTranslateY(int32_t y){
    cmd32(
        DL_VERTEX_TRANSLATE_Y |
        ((uint32_t)y & 0x1FFFFUL)
    );
}

void EVE_esp32::polar(int& x, int& y, int radius, uint16_t angle){
    constexpr float ANGLE_SCALE =
        6.28318530717958647692f / 65536.0f;

    const float a = angle * ANGLE_SCALE;

    x = lroundf(cosf(a) * radius);
    y = lroundf(sinf(a) * radius);
}

void EVE_esp32::cmd_setfont(uint32_t font, uint32_t ptr){
    cmd32(CMD_SETFONT);
    cmd32(font);
    cmd32(ptr);
}

void EVE_esp32::cmd_setfont2(uint32_t font, uint32_t ptr, uint32_t firstChar){
    cmd32(CMD_SETFONT2);
    cmd32(font);
    cmd32(ptr);
    cmd32(firstChar);
}

void EVE_esp32::cmd_snapshot2(uint32_t format, uint32_t ptr, int16_t x, int16_t y, uint16_t width, uint16_t height){
    cmd32(CMD_SNAPSHOT2);
    cmd32(format);
    cmd32(ptr);

    cmd16((uint16_t)x);
    cmd16((uint16_t)y);
    cmd16(width);
    cmd16(height);
}

void EVE_esp32::cmd_memwrite(uint32_t ptr, uint32_t num){
    cmd32(CMD_MEMWRITE);

    cmd32(ptr);
    cmd32(num);    
}


