#include "../EVE_esp32.h"

void EVE_esp32::dlDisplay(){ dl(DL_DISPLAY); }
void EVE_esp32::dlCall(uint16_t dest){ dl(DL_CALL | dest); }
void EVE_esp32::dlJump(uint16_t dest){ dl(DL_JUMP | dest); }
void EVE_esp32::dlBegin(uint8_t primitive){ dl(DL_BEGIN | (primitive & 0x0F)); }
void EVE_esp32::dlEnd(){ dl(DL_END); }
void EVE_esp32::dlReturn(){ dl(DL_RETURN); }
void EVE_esp32::dlMacro(uint8_t m){ dl(DL_MACRO | (m & 1)); }
void EVE_esp32::dlNop(){ dl(DL_NOP); }

void EVE_esp32::dlBitmapSource(uint32_t addr){ dl(DL_BITMAP_SOURCE | (addr & 0x00FFFFFFUL)); }
void EVE_esp32::dlBitmapHandle(uint8_t handle){ dl(DL_BITMAP_HANDLE | (handle & 0x1F)); }
void EVE_esp32::dlCell(uint8_t cell){ dl(DL_CELL | (cell & 0x7F)); }

void EVE_esp32::dlBitmapLayout(uint8_t format, uint16_t lineStride, uint16_t height){
    dl(DL_BITMAP_LAYOUT | ((uint32_t)(format & 0x1F) << 19) |
       ((uint32_t)(lineStride & 0x03FF) << 9) | (height & 0x01FF));
}

void EVE_esp32::dlBitmapSize(uint8_t filter, uint8_t wrapX, uint8_t wrapY, uint16_t width, uint16_t height){
    dl(DL_BITMAP_SIZE | ((uint32_t)(filter & 1) << 20) |
       ((uint32_t)(wrapX & 1) << 19) | ((uint32_t)(wrapY & 1) << 18) |
       ((uint32_t)(width & 0x01FF) << 9) | (height & 0x01FF));
}

void EVE_esp32::dlBitmapLayoutH(uint16_t lineStride, uint16_t height){
    dl(DL_BITMAP_LAYOUT_H | ((uint32_t)((lineStride >> 10) & 3) << 2) | ((height >> 9) & 3));
}

void EVE_esp32::dlBitmapSizeH(uint16_t width, uint16_t height){
    dl(DL_BITMAP_SIZE_H | ((uint32_t)((width >> 9) & 3) << 2) | ((height >> 9) & 3));
}

void EVE_esp32::dlPaletteSource(uint32_t addr){ dl(DL_PALETTE_SOURCE | (addr & 0x00FFFFFFUL)); }
void EVE_esp32::dlBitmapExtFormat(uint16_t format){ dl(DL_BITMAP_EXT_FORMAT | format); }

void EVE_esp32::dlBitmapSwizzle(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    dl(DL_BITMAP_SWIZZLE | ((uint32_t)(r & 7) << 9) | ((uint32_t)(g & 7) << 6) |
       ((uint32_t)(b & 7) << 3) | (a & 7));
}

void EVE_esp32::dlPointSize(uint16_t size){ dl(DL_POINT_SIZE | (size & 0x1FFF)); }
void EVE_esp32::dlLineWidth(uint16_t width){ dl(DL_LINE_WIDTH | (width & 0x0FFF)); }
void EVE_esp32::dlVertexFormat(uint8_t frac){ dl(DL_VERTEX_FORMAT | (frac & 7)); }
void EVE_esp32::dlVertexTranslateX(int32_t x){ dl(DL_VERTEX_TRANSLATE_X | ((uint32_t)x & 0x1FFFF)); }
void EVE_esp32::dlVertexTranslateY(int32_t y){ dl(DL_VERTEX_TRANSLATE_Y | ((uint32_t)y & 0x1FFFF)); }

void EVE_esp32::dlVertex2f(int16_t x, int16_t y){
    dl(DL_VERTEX2F | (((uint32_t)x & 0x7FFF) << 15) | ((uint32_t)y & 0x7FFF));
}

void EVE_esp32::dlVertex2ii(uint16_t x, uint16_t y, uint8_t handle, uint8_t cell){
    dl(DL_VERTEX2II | ((uint32_t)(x & 0x01FF) << 21) | ((uint32_t)(y & 0x01FF) << 12) |
       ((uint32_t)(handle & 0x1F) << 7) | (cell & 0x7F));
}

void EVE_esp32::dlClearColorRGB(uint8_t r, uint8_t g, uint8_t b){
    dl(DL_CLEAR_COLOR_RGB | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void EVE_esp32::dlClearColorRGB(uint32_t rgb){ dl(DL_CLEAR_COLOR_RGB | (rgb & 0x00FFFFFFUL)); }

void EVE_esp32::dlColorRGB(uint8_t r, uint8_t g, uint8_t b){
    dl(DL_COLOR_RGB | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void EVE_esp32::dlColorRGB(uint32_t rgb){ dl(DL_COLOR_RGB | (rgb & 0x00FFFFFFUL)); }
void EVE_esp32::dlClearColorA(uint8_t a){ dl(DL_CLEAR_COLOR_A | a); }
void EVE_esp32::dlColorA(uint8_t a){ dl(DL_COLOR_A | a); }
void EVE_esp32::dlClearStencil(uint8_t s){ dl(DL_CLEAR_STENCIL | s); }
void EVE_esp32::dlClearTag(uint8_t tag){ dl(DL_CLEAR_TAG | tag); }

void EVE_esp32::dlColorMask(bool r, bool g, bool b, bool a){
    dl(DL_COLOR_MASK | ((uint32_t)r << 3) | ((uint32_t)g << 2) |
       ((uint32_t)b << 1) | (uint32_t)a);
}

void EVE_esp32::dlClear(bool color, bool stencil, bool tag){
    dl(DL_CLEAR | ((uint32_t)color << 2) | ((uint32_t)stencil << 1) | (uint32_t)tag);
}

void EVE_esp32::dlAlphaFunc(uint8_t func, uint8_t ref){
    dl(DL_ALPHA_FUNC | ((uint32_t)(func & 7) << 8) | ref);
}

void EVE_esp32::dlStencilFunc(uint8_t func, uint8_t ref, uint8_t mask){
    dl(DL_STENCIL_FUNC | ((uint32_t)(func & 7) << 16) | ((uint32_t)ref << 8) | mask);
}

void EVE_esp32::dlBlendFunc(uint8_t src, uint8_t dst){
    dl(DL_BLEND_FUNC | ((uint32_t)(src & 7) << 3) | (dst & 7));
}

void EVE_esp32::dlStencilOp(uint8_t sfail, uint8_t spass){
    dl(DL_STENCIL_OP | ((uint32_t)(sfail & 7) << 3) | (spass & 7));
}

void EVE_esp32::dlStencilMask(uint8_t mask){ dl(DL_STENCIL_MASK | mask); }
void EVE_esp32::dlTag(uint8_t tag){ dl(DL_TAG | tag); }
void EVE_esp32::dlTagMask(bool enable){ dl(DL_TAG_MASK | (uint32_t)enable); }

void EVE_esp32::dlBitmapTransformA(int32_t a){ dl(DL_BITMAP_TRANSFORM_A | ((uint32_t)a & 0x1FFFF)); }
void EVE_esp32::dlBitmapTransformB(int32_t b){ dl(DL_BITMAP_TRANSFORM_B | ((uint32_t)b & 0x1FFFF)); }
void EVE_esp32::dlBitmapTransformC(int32_t c){ dl(DL_BITMAP_TRANSFORM_C | ((uint32_t)c & 0xFFFFFF)); }
void EVE_esp32::dlBitmapTransformD(int32_t d){ dl(DL_BITMAP_TRANSFORM_D | ((uint32_t)d & 0x1FFFF)); }
void EVE_esp32::dlBitmapTransformE(int32_t e){ dl(DL_BITMAP_TRANSFORM_E | ((uint32_t)e & 0x1FFFF)); }
void EVE_esp32::dlBitmapTransformF(int32_t f){ dl(DL_BITMAP_TRANSFORM_F | ((uint32_t)f & 0xFFFFFF)); }

void EVE_esp32::dlBitmapTransformA(uint8_t p, int32_t a){
    dl(DL_BITMAP_TRANSFORM_A_EXT | ((uint32_t)(p & 1) << 17) | ((uint32_t)a & 0x1FFFF));
}

void EVE_esp32::dlBitmapTransformB(uint8_t p, int32_t b){
    dl(DL_BITMAP_TRANSFORM_B_EXT | ((uint32_t)(p & 1) << 17) | ((uint32_t)b & 0x1FFFF));
}

void EVE_esp32::dlBitmapTransformD(uint8_t p, int32_t d){
    dl(DL_BITMAP_TRANSFORM_D_EXT | ((uint32_t)(p & 1) << 17) | ((uint32_t)d & 0x1FFFF));
}

void EVE_esp32::dlBitmapTransformE(uint8_t p, int32_t e){
    dl(DL_BITMAP_TRANSFORM_E_EXT | ((uint32_t)(p & 1) << 17) | ((uint32_t)e & 0x1FFFF));
}

void EVE_esp32::dlScissorXY(uint16_t x, uint16_t y){
    dl(DL_SCISSOR_XY | ((uint32_t)(x & 0x07FF) << 11) | (y & 0x07FF));
}

void EVE_esp32::dlScissorSize(uint16_t width, uint16_t height){
    dl(DL_SCISSOR_SIZE | ((uint32_t)(width & 0x0FFF) << 12) | (height & 0x0FFF));
}

void EVE_esp32::dlSaveContext(){ dl(DL_SAVE_CONTEXT); }
void EVE_esp32::dlRestoreContext(){ dl(DL_RESTORE_CONTEXT); }
