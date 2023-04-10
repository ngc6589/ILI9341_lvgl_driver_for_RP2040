#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "ili9341.h"
#include "lvgl.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define LCD_SPI_PORT spi1
#define LCD_MISO 12
#define LCD_CS 9
#define LCD_SCK 10
#define LCD_MOSI 11
#define LCD_RESET 15
#define LCD_DC 8
#define LCD_LED 13
#define LCD_LED_PWM_MAX 2000 

#define MADCTL_MX 0x40
#define MADCTL_MY 0x80
#define MADCTL_MV 0x20
#define MADCTL_ML 0x10
#define MADCTL_BGR 0x08
#define MADCTL_MH 0x04
#define BGR 0x08

#define COLOR_Black       0x0000
#define COLOR_Navy        0x000F
#define COLOR_DarkGreen   0x03E0
#define COLOR_DarkCyan    0x03EF
#define COLOR_Maroon      0x7800
#define COLOR_Purple      0x780F
#define COLOR_Olive       0x7BE0
#define COLOR_LightGrey   0xC618
#define COLOR_DarkGrey    0x7BEF
#define COLOR_Blue        0x001F
#define COLOR_Green       0x07E0
#define COLOR_Cyan        0x07FF
#define COLOR_Red         0xF800
#define COLOR_Magenta     0xF81F
#define COLOR_Yellow      0xFFE0
#define COLOR_White       0xFFFF
#define COLOR_Orange      0xFD20
#define COLOR_GreenYellow 0xAFE5
#define COLOR_Pink        0xF81F
#define COLOR_INV_Black       0x0000
#define COLOR_INV_Navy        0x7800
#define COLOR_INV_DarkGreen   0x03E0
#define COLOR_INV_DarkCyan    0x7BE0
#define COLOR_INV_Maroon      0x000F
#define COLOR_INV_Purple      0x780F
#define COLOR_INV_Olive       0x03EF
#define COLOR_INV_LightGrey   0xC618
#define COLOR_INV_DarkGrey    0x7BEF
#define COLOR_INV_Blue        0xF800
#define COLOR_INV_Green       0x07E0
#define COLOR_INV_Cyan        0xFFE0
#define COLOR_INV_Red         0x001F
#define COLOR_INV_Magenta     0xF81F
#define COLOR_INV_Yellow      0x07FF
#define COLOR_INV_White       0xFFFF
#define COLOR_INV_Orange      0x053F
#define COLOR_INV_GreenYellow 0x2FF5
#define COLOR_INV_Pink        0xF81F

uint slice_num;

typedef struct {
        uint8_t cmd;
        uint8_t dat[16];
        uint datLen;
        uint32_t sleep;
} ili9341_ini_str_t;

ili9341_ini_str_t lcd_ini_str[] = {
        {0x01, {0x00}, 0, 200},                         /* software reset */
        {0xEF, {0x03, 0x80, 0x02}, 3, 0},
        {0xCF, {0x00, 0xC1, 0x30}, 3, 0},               /* power control B */
        {0xED, {0x64, 0x03, 0x12, 0x81}, 4, 0},         /* power-on sequence control */
        {0xE8, {0x85, 0x00, 0x78}, 3, 0},               /* driver timing control A */
        {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5, 0},   /* power control A */
        {0xF7, {0x20}, 1, 0},                           /* pump ratio control */
        {0xEA, {0x00, 0x00}, 2, 0},                     /* driver timing control B */
        {0xC0, {0x23}, 1, 0},                           /* power control VRH[5:0] */
        {0xC1, {0x10}, 1, 0},                           /* power control SAP[2:0] BT[3:0] */
        {0xC5, {0x3E, 0x28}, 2, 0},                     /* VCM control */
        {0xC7, {0x86}, 1, 0},                           /* VCM control 2 */
        {0x36, {0x00}, 1, 0},                           /* memory access control */
        {0x37, {0x00, 0x00}, 2, 0},                     /* Vertical Scrolling Start Address */
        {0x3A, {0x55}, 1, 0},                           /* pixel format */
        {0xB1, {0x00, 0x18}, 2, 0},                     /* frame ratio control, standard RGB color */
        {0xB6, {0x08, 0x82, 0x27}, 3, 0},               /* display function control */
        {0xF2, {0x00}, 1, 0},                           /* 3-Gamma function disable */
        {0x26, {0x01}, 1, 0},                           /* Gamma Set */
        {0xE0, {                                        /* positive gamma correction*/
                0x0F, 0x31, 0x2B, 0x0C, 0x0E,
                0x08, 0x4E, 0xF1, 0x37, 0x07,
                0x10, 0x03, 0x0E, 0x09, 0x00
                }, 15, 0},
        {0xE1, {                                        /* negative gamma correction */
                0x00, 0x0E, 0x14, 0x03, 0x11,
                0x07, 0x31, 0xC1, 0x48, 0x08,
                0x0F, 0x0C, 0x31, 0x36, 0x0F
                }, 15, 0},
        {0x11, {0x00}, 0, 120},                         /* exit sleep */
        {0x29, {0x00}, 0, 120},                         /* turn on display */
        {0x00, {0x00}, 0, 0}                            /* EOL */
};

 struct
{
        uint width;
        uint height;
} ili9341_resolution;

int dmaChannel;
dma_channel_config c;

void ili9341_Init(uint rot)
{
        // // Get a free channel, panic() if there are none
        int chan = dma_claim_unused_channel(true);
        c = dma_channel_get_default_config(dmaChannel);
        channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
        channel_config_set_dreq(&c, DREQ_SPI1_TX);
        channel_config_set_read_increment(&c, true);
        channel_config_set_write_increment(&c, false);

        // SPI initialisation. This example will use SPI at 1MHz.
        spi_init(LCD_SPI_PORT, 1 * 1000 * 1000);
        spi_set_baudrate(LCD_SPI_PORT, 46000000);
        gpio_set_function(LCD_MISO, GPIO_FUNC_SPI);
        gpio_set_function(LCD_CS, GPIO_FUNC_SIO);
        gpio_set_function(LCD_SCK, GPIO_FUNC_SPI);
        gpio_set_function(LCD_MOSI, GPIO_FUNC_SPI);

        // Chip select is active-low, so we'll initialise it to a driven-high state
        gpio_init(LCD_CS);
        gpio_set_dir(LCD_CS, GPIO_OUT);
        gpio_put(LCD_CS, 1);
        gpio_init(LCD_DC);
        gpio_set_dir(LCD_DC, GPIO_OUT);
        gpio_put(LCD_DC, 1);
        gpio_init(LCD_RESET);
        gpio_set_dir(LCD_RESET, GPIO_OUT);
        gpio_put(LCD_RESET, 1);

        // LCD BackLight PWM control
        gpio_set_function(LCD_LED, GPIO_FUNC_PWM);
        slice_num = pwm_gpio_to_slice_num(LCD_LED);
        pwm_set_wrap(slice_num, (LCD_LED_PWM_MAX - 1));
        pwm_set_chan_level(slice_num, LCD_LED, 0);
        pwm_set_enabled(slice_num, true);

        // initialize LCD
        ili9341_HardReset();
        ili9341_SstLED(10);
        ili9341_SendInitStr();
        ili9341_setRotate(rot);
        ili9341_SetWindow(0, 0, ili9341_resolution.width, ili9341_resolution.width);
        ili9341_SstLED(100);
}

void ili9341_HardReset()
{
        gpio_put(LCD_RESET, 1);
        sleep_ms(10);
        gpio_put(LCD_RESET, 0);
        sleep_ms(100);
        gpio_put(LCD_RESET, 1);
        sleep_ms(100);
}

void ili9341_SstLED(uint parcent)
{
        if (parcent > 100)
        {
                parcent = 100;
        }
        pwm_set_chan_level(slice_num, LCD_LED, parcent * 20);
}

void ili9341_SendInitStr()
{
        ili9341_SetCS(0);
        uint i = 0;
        while(lcd_ini_str[i].cmd != 0x00)
        {
                uint8_t cmd = lcd_ini_str[i].cmd;
                uint datLen = lcd_ini_str[i].datLen;
                uint8_t *dat;
                dat = &(lcd_ini_str[i].dat[0]);
                uint32_t slp = lcd_ini_str[i].sleep;

                ili9341_SetDC(0);
                spi_write_blocking(LCD_SPI_PORT, &cmd, 1);

                if(datLen > 0)
                {
                        ili9341_SetDC(1);
                        spi_write_blocking(LCD_SPI_PORT, dat, datLen);
                }
                if(slp > 0)
                {
                        sleep_ms(slp);
                }
                i++;
        }
        ili9341_SetCS(1);
}

void ili9341_SetCS(bool val)
{
        asm volatile("nop\n");
        asm volatile("nop\n");
        gpio_put(LCD_CS, val);
        asm volatile("nop\n");
        asm volatile("nop\n");
}

void ili9341_SetDC(bool val)
{
        asm volatile("nop\n");
        asm volatile("nop\n");
        gpio_put(LCD_DC, val);
        asm volatile("nop\n");
        asm volatile("nop\n");
}

void ili9341_SendData(uint8_t cmd, uint8_t *data, uint length)
{
        ili9341_SetCS(0);
        ili9341_SetDC(0);
        spi_write_blocking(LCD_SPI_PORT, &cmd, 1);
        ili9341_SetDC(1);
        spi_write_blocking(LCD_SPI_PORT, data, length);
        ili9341_SetCS(1);
}

void ili9341_setRotate(uint rot)
{
        uint8_t cmd = 0x36;
        uint8_t r;
        switch (rot)
        {
                case LCD_PORTRAIT:
                        r = MADCTL_MX;
                        ili9341_resolution.width = 240;
                        ili9341_resolution.height = 320;
                        break;
                case LCD_LANDSCAPE:
                        r = MADCTL_MV;
                        ili9341_resolution.width = 320;
                        ili9341_resolution.height = 240;
                        break;
                case LCD_INV_PORTRAIT:
                        r = MADCTL_MY;
                        ili9341_resolution.width = 240;
                        ili9341_resolution.height = 320;
                        break;
                case LCD_INV_LANDSCAPE:
                        r = (MADCTL_MX | MADCTL_MY | MADCTL_MV);
                        ili9341_resolution.width = 320;
                        ili9341_resolution.height = 240;
                        break;
                default:
                        r = MADCTL_MX;
                        ili9341_resolution.width = 240;
                        ili9341_resolution.height = 320;
        }

        r |= 0x08;
        ili9341_SendData(cmd, &r, 1);
}

void ili9341_SetWindow(uint x, uint y, uint w, uint h)
{
        /* CASET */
        uint8_t cmd = 0x2A;
        uint8_t buf4[4];
        buf4[0] = (x >> 8) & 0xFF;
        buf4[1] = x & 0xFF;
        buf4[2] = ((x + w - 1) >> 8) & 0xFF;
        buf4[3] = (x + w - 1) & 0xFF;
        ili9341_SendData(cmd, buf4, 4);

        /* RASET */
        cmd = 0x2B;
        buf4[0] = (y >> 8) & 0xFF;
        buf4[1] = y & 0xFF;
        buf4[2] = ((y + h - 1) >> 8) & 0xFF;
        buf4[3] = (y + h - 1) & 0xFF;
        ili9341_SendData(cmd, buf4, 4);
}

void lcd_Flash_CB(lv_disp_t * disp, const lv_area_t * area, lv_color_t * buf)
{
        uint8_t cmd = 0x2C;
        uint x1, y1;
        x1 = area->x1;
        y1 = area->y1;
        uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);
        /* 
         *  transfer pixel data via DMA function
         */
        while(1)
        {
                bool a = dma_channel_is_busy(dmaChannel);
                if(a == false)  break;
        }
        ili9341_SetWindow(x1, y1, lv_area_get_width(area), lv_area_get_height(area));
        ili9341_SetCS(0);   
        ili9341_SetDC(0);
        spi_write_blocking(LCD_SPI_PORT, &cmd, 1);  /* RAMWR **/
        ili9341_SetDC(1);
        /* 
         *  transfer pixel data via SPI function
         */

        /* spi_write_blocking(LCD_SPI_PORT, (void *)buf, size*2); */
        /* ili9341_SetCS(1); */

        /* 
         *  transfer pixel data via DMA function
         */
        lcd_Send_Color_DMA((void *) buf,  size*2);
        lv_disp_flush_ready(disp);
}

void lcd_Send_Color_DMA(void * buf, uint16_t length)
{
        dma_channel_configure(
                dmaChannel,             // Channel to be configured
                &c,                     // The configuration we just created
                &spi_get_hw(LCD_SPI_PORT)->dr,                    // The initial write address
                buf,                    // The initial read address
                length,                 // Number of transfers; in this case each is 1 byte.
                true                    // Start immediately.
        );
}

lv_coord_t lcd_Get_Width()
{
        return(ili9341_resolution.width);
}

lv_coord_t lcd_Get_height()
{
        return(ili9341_resolution.height);
}