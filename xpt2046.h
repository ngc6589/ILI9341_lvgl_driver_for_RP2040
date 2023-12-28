#ifndef XPT2046_H
#define XPT2046_H
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TP_PORTRAIT 0
#define TP_LANDSCAPE 1
#define TP_INV_PORTRAIT 2
#define TP_INV_LANDSCAPE 3

void xpt2046_Init(uint rot);
int xpt2046_raw_pos(uint16_t * x, uint16_t * y);
int xpt2046_Pos(uint16_t * posX, uint16_t * posY);
void xpt2046_read_cb(lv_indev_t * indev, lv_indev_data_t * data);

#ifdef __cplusplus
}
#endif
#endif // XPT2046_H