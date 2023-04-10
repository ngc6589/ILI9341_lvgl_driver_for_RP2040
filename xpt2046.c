#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pio_spi.h"
#include "xpt2046.h"
#include "lvgl.h"

#define XPT2046_CS 16
#define XPT2046_CLK 17
#define XPT2046_MOSI 18
#define XPT2046_MISO 19

#define XPT2046_BITS 12
#define XPT2046_X_MIN  100
#define XPT2046_X_MAX 1900
#define XPT2046_Y_MIN  200
#define XPT2046_Y_MAX 1950

#define TP_PORTRAIT 0
#define TP_LANDSCAPE 1
#define TP_INV_PORTRAIT 2
#define TP_INV_LANDSCAPE 3
#define CHAN_X      0b01010000
#define CHAN_Y      0b00010000
#define CHAN_Z1     0b00110000
#define CHAN_Z2     0b01000000
#define CHAN_T0     0b00000000
#define CHAN_T1     0b01110000
#define CHAN_BAT    0b00100000
#define CHAN_AUX    0b01100000
#define CONV_8_BIT  0b00001000
#define CONV_12_BIT 0b00000000
#define START_BIT   0b10000000

pio_spi_inst_t spi = {
    .pio = pio0,
    .sm = 0,
    .cs_pin = XPT2046_CS};

uint8_t spiSendBuf[3];
uint8_t spiRecvBuf[3];

uint8_t bits = 12;
uint8_t conv = CONV_12_BIT;
struct  {
        uint xMin;
        uint xMax;
        uint yMin;
        uint yMax;
} range;

struct {
        uint width;
        uint height;
} dim;
uint8_t rotation;
struct {
        float x;
        float y;
} scale;
struct {
        uint x;
        uint y;
} origin;

void xpt2046_Init(uint rot)
{
        gpio_init(XPT2046_CS);
        gpio_set_dir(XPT2046_CS, GPIO_OUT);
        gpio_put(XPT2046_CS, 1);
        float clkdiv = 31.25f;  // 1 MHz @ 125 clk_sys
        uint offset = pio_add_program(spi.pio, &spi_cpha0_program);

        pio_spi_init(   spi.pio, 
                        spi.sm, 
                        offset,
                        8,       // 8 bits per SPI frame
                        31.25f,  // 1 MHz @ 125 clk_sys
                        false,   // CPHA = 0
                        false,   // CPOL = 0
                        XPT2046_CLK,
                        XPT2046_MOSI,
                        XPT2046_MISO
        );
        bits = CONV_12_BIT;
        range.xMin = 100;
        range.xMax = 1900;
        range.yMin = 200;
        range.yMax = 1950;
        dim.width = 240;
        dim.height = 320;
        rotation = rot;
        scale.x = (float)dim.width / (range.xMax - range.xMin);
        scale.y = (float)dim.height / (range.yMax - range.yMin);
        origin.x = range.xMin;
        origin.y = range.yMin;
}

int xpt2046_raw_pos(uint16_t * x, uint16_t * y)
{
        uint16_t xVal;
        uint16_t yVal;
        spiSendBuf[0] = (START_BIT | conv | CHAN_X);
        spiSendBuf[1] = 0;
        spiSendBuf[2] = 0;
        gpio_put(XPT2046_CS, 0);
        pio_spi_write8_read8_blocking(&spi, spiSendBuf, spiRecvBuf, 3);
        gpio_put(XPT2046_CS, 1);
        if(conv == CONV_8_BIT)
        {
                xVal = (uint)spiRecvBuf[1];
        }
        else
        {
                xVal = (uint)((spiRecvBuf[1] << 4) | (spiRecvBuf[2] >> 4)); 
        }

        spiSendBuf[0] = (START_BIT | conv | CHAN_Y);
        gpio_put(XPT2046_CS, 0);
        pio_spi_write8_read8_blocking(&spi, spiSendBuf, spiRecvBuf, 3);
        gpio_put(XPT2046_CS, 1);
        if(conv == CONV_8_BIT)
        {
                yVal = (uint)spiRecvBuf[1];
        }
        else
        {
                yVal = (uint)((spiRecvBuf[1] << 4) | (spiRecvBuf[2] >> 4)); 
        }

        if((xVal < range.xMin) | (xVal > range.xMax))
        {
                return(-1);
        }

        if((yVal < range.yMin) | (yVal > range.yMax))
        {
                return(-1);
        }
        *x = xVal;
        *y = yVal;
        return(0);
}

int xpt2046_Pos(uint16_t * posX, uint16_t * posY)
{
        int N = 10;
        int attempts = 20;
        uint16_t xx = 0;
        uint16_t yy = 0;
        uint done = 0;
        float mx = 0;
        float my = 0;
        uint16_t pixX = 0;
        uint16_t pixY = 0;
        uint16_t px = 0;
        uint16_t py = 0;;
        for(uint x = 0; x < attempts; x++)
        {
                int ret;
                ret = xpt2046_raw_pos(&px, &py);
                if(ret == -1)
                {
                        continue;
                }
                xx += px;
                yy += py;
                done += 1;
                if(done == N)
                {
                        break;
                }
        }
        if(done < N) 
        {
                return(-1);
        }
        mx = (float)xx * 1.0 / N;
        my = (float)yy * 1.0 / N;
        pixX = (uint16_t)(scale.x * (mx - origin.x));
        pixY = (uint16_t)(scale.y * (my - origin.y));
        if(rotation == TP_PORTRAIT)
        {
                *posX = pixX;
                *posY = dim.height - pixY;
        }
        else if(rotation == TP_LANDSCAPE)
        {
                *posX = dim.height - pixY;
                *posY = dim.width - pixX;
        }
        else if(rotation == TP_INV_PORTRAIT)
        {
                *posX = dim.width - pixX;
                *posY = pixY;
        }
        else if(rotation == TP_INV_LANDSCAPE)
        {
                *posX = pixY;
                *posY = pixX;
        }
        else
        {
                *posX = 0;
                *posY = 0;
                return(-1);
        }
        return(0);
}

void xpt2046_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
        uint16_t posX;
        uint16_t posY;
        if(xpt2046_Pos(&posX, &posY) == -1)
        {
                data->point.x = 0;
                data->point.y = 0;
                data->state = 0;
        }
        else
        {
                data->point.x = posX;
                data->point.y = posY;
                data->state = 1;
        }
}