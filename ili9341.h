#ifndef ILI9341_H
#define ILI9341_H
#include "lvgl.h"
#define LCD_PORTRAIT 0
#define LCD_LANDSCAPE 1
#define LCD_INV_PORTRAIT 2
#define LCD_INV_LANDSCAPE 3

void ili9341_Init(uint rot);

void ili9341_HardReset();

void ili9341_SstLED(uint parcent);

void ili9341_SendInitStr();

void ili9341_SetCS(bool val);

void ili9341_SetDC(bool val);

void ili9341_SendData(uint8_t cmd, uint8_t *data, uint length);

void ili9341_setRotate(uint rot);

void ili9341_SetWindow(uint x, uint y, uint w, uint h);

void lcd_Flash_CB(lv_disp_t *disp, const lv_area_t *area, unsigned char *buf);

void lcd_Send_Color_DMA(void *buf, uint16_t length);

uint lcd_Get_Width(void);

uint lcd_Get_height(void);

#endif

