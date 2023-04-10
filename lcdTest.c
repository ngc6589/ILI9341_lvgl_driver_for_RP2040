#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "lvgl.h"
void lv_example_style_13(void);
static lv_color_t buf1[240 * 320 / 10];
static lv_color_t buf2[240 * 320 / 10];

int main()
{
        stdio_init_all();
        ili9341_Init(LCD_INV_LANDSCAPE);
        xpt2046_Init(TP_INV_LANDSCAPE);

        lv_init();
        lv_disp_t *disp = lv_disp_create(lcd_Get_Width(), lcd_Get_height());
        lv_disp_set_flush_cb(disp, lcd_Flash_CB);
        lv_disp_set_draw_buffers(disp, buf1, buf2, sizeof(buf1), LV_DISP_RENDER_MODE_PARTIAL);
        lv_disp_set_color_format(disp, LV_COLOR_FORMAT_NATIVE_REVERSED);

        lv_indev_t * indev = lv_indev_create();
        lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); 
        lv_indev_set_read_cb(indev, xpt2046_read_cb); 

        lv_example_style_13();
        while(1)
        {
                sleep_ms(5);
                lv_timer_handler();
                lv_tick_inc(5);
        }
        return 0;
}

void lv_example_style_13(void)
{
    static lv_style_t style_indic;
    lv_style_init(&style_indic);
    lv_style_set_bg_color(&style_indic, lv_palette_lighten(LV_PALETTE_RED, 3));
    lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_HOR);

    static lv_style_t style_indic_pr;
    lv_style_init(&style_indic_pr);
    lv_style_set_shadow_color(&style_indic_pr, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_shadow_width(&style_indic_pr, 10);
    lv_style_set_shadow_spread(&style_indic_pr, 3);

    /*Create an object with the new style_pr*/
    lv_obj_t * obj = lv_slider_create(lv_scr_act());
    lv_obj_add_style(obj, &style_indic, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &style_indic_pr, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_slider_set_value(obj, 70, LV_ANIM_OFF);
    lv_obj_center(obj);
}