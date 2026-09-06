#pragma once

void Display()                          { cmd32(DL_DISPLAY);                                }
void ClearColorRGB(uint32_t color = 0)  { cmd32(DL_CLEAR_COLOR_RGB | (color & 0xFFFFFFUL)); }
void PointSize(uint16_t size)           { cmd32(DL_POINT_SIZE | (size & 0x1FFF));           }
void LineWidth(uint16_t width)          { cmd32(DL_LINE_WIDTH | (width & 0x0FFF));          }
void Begin(uint8_t prim)                { cmd32(DL_BEGIN | (prim & 0x0F));                  }
void End()                              { cmd32(DL_END);                                    }
void VertexFormat(uint8_t frac)         { cmd32(DL_VERTEX_FORMAT | (frac & 0x07));          }
void cmd_stop()                         { cmd32(CMD_STOP);                                  }
void BitmapSource(uint32_t addr)        { cmd32(DL_BITMAP_SOURCE | (addr & 0x3FFFFFUL));    }
void cmd_screensaver()                  { cmd32(CMD_SCREENSAVER);                           }
void BitmapHandle(uint8_t handle)       { cmd32(DL_BITMAP_HANDLE | (handle & 0x1F));        }
void Cell(uint8_t cell)                 { cmd32(DL_CELL | (cell & 0x7F));                   }
void ColorA(uint8_t alpha)              { cmd32(DL_COLOR_A | (uint32_t)alpha);              }
void cmd_setmatrix()                    { cmd32(CMD_SETMATRIX);                             }
void cmd_loadidentity()                 { cmd32(CMD_LOADIDENTITY);                          }
void PaletteSource(uint32_t addr)       { cmd32(DL_PALETTE_SOURCE | (addr & 0x3FFFFFUL));   }
void SaveContext()                      { cmd32(DL_SAVE_CONTEXT);                           }
void RestoreContext()                   { cmd32(DL_RESTORE_CONTEXT);                        }
void ClearColorA(uint8_t alpha)         { cmd32(DL_CLEAR_COLOR_A | (uint32_t)alpha);        }

void ClearColorRGB(uint8_t r, uint8_t g, uint8_t b);
void ColorRGB(uint8_t r, uint8_t g, uint8_t b);
void ColorRGB(uint32_t color = 0);
void Clear(bool c = true, bool s = true, bool t = true);
void BlendFunc(uint8_t src, uint8_t dst);
void ColorMask(bool r, bool g, bool b, bool a);
void ScissorXY(uint16_t x, uint16_t y);
void ScissorSize(uint16_t width, uint16_t height);
void polar(int& x, int& y, int radius, uint16_t angle);

void Vertex2f(int16_t x, int16_t y);
void Vertex2p(int16_t x, int16_t y);
void Vertex2ii(uint16_t x, uint16_t y, uint8_t handle, uint8_t cell);
void VertexTranslateX(int32_t x);
void VertexTranslateY(int32_t y);

void cmd_setfont(uint32_t font, uint32_t ptr);
void cmd_setfont2(uint32_t font, uint32_t ptr, uint32_t firstChar);
void cmd_text(int16_t x, int16_t y, int16_t font, uint16_t options, const char* s);
void cmd_button(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t font, uint16_t options, const char* s);
void cmd_toggle(int16_t x, int16_t y, int16_t w, int16_t font, uint16_t options, uint16_t state, const char* s);
void cmd_slider(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t options, uint16_t val, uint16_t range);
void cmd_progress(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t options, uint16_t val, uint16_t range);
void cmd_gauge(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t major, uint16_t minor, uint16_t val, uint16_t range);
void cmd_clock(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t h, uint16_t m, uint16_t s, uint16_t ms);
void cmd_dial(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t val);
void cmd_scrollbar(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t options, uint16_t val, uint16_t size, uint16_t range);
void cmd_keys(int16_t x, int16_t y, uint16_t w, uint16_t h, int16_t font, uint16_t options, const char* s);
void cmd_number(int16_t x, int16_t y, int16_t font, uint16_t options, int32_t n);
void cmd_gradient(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1);
void cmd_spinner(int16_t x, int16_t y, uint16_t style, uint16_t scale);
void cmd_sketch(int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t ptr, uint16_t format);
void cmd_memzero(uint32_t ptr, uint32_t num);
void cmd_translate(int32_t tx, int32_t ty);
void cmd_rotate(int32_t angle);
void cmd_rotatearound(int32_t x, int32_t y, uint32_t angle, int32_t scale);
void cmd_loadimage(uint32_t ptr, uint32_t options);
void cmd_scale(int32_t sx, int32_t sy);
void cmd_snapshot2(uint32_t format, uint32_t ptr, int16_t x, int16_t y, uint16_t width, uint16_t height);

void BitmapLayout(uint8_t format, uint16_t linestride, uint16_t height);
void BitmapSize(uint8_t filter, uint8_t wrapx, uint8_t wrapy, uint16_t width, uint16_t height);
void BitmapLayoutH(uint16_t linestride, uint16_t height);
void BitmapSizeH(uint16_t width, uint16_t height);

void cmd_dlstart()      { cmd32(0xFFFFFF00UL); }
void cmd_swap()         { cmd32(0xFFFFFF01UL); }

void cmd_memwrite(uint32_t ptr, uint32_t num);










