#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "lvgl.h"

static lv_color_t buf1[240 * 320 / 10];
static lv_color_t buf2[240 * 320 / 10];
static lv_obj_t *screen;
void lv_example_button_1(void);

int main()
{
    stdio_init_all();
    ili9341_Init(LCD_INV_LANDSCAPE);
    xpt2046_Init(TP_INV_LANDSCAPE);

    lv_init();
    lv_display_t *disp = lv_display_create(320, 240);
    lv_display_set_flush_cb(disp, lcd_Flash_CB);
    lv_display_set_draw_buffers(disp, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    screen = lv_obj_create(NULL);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, xpt2046_read_cb);

    lv_example_button_1();
    lv_display_load_scr(screen);
    while (1)
    {
        sleep_ms(5);
        lv_timer_handler();
        lv_tick_inc(5);
    }

    return 0;
}

static void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        LV_LOG_USER("Clicked");
    }
    else if (code == LV_EVENT_VALUE_CHANGED)
    {
        LV_LOG_USER("Toggled");
    }
}

void lv_example_button_1(void)
{
    lv_style_selector_t sty;
    sty = 0;
    lv_obj_t *label;

    lv_obj_t *btn1 = lv_button_create(screen);
    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_set_width(btn1, 122);
    lv_obj_set_height(btn1, 69);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0x00FF00),sty);
    lv_obj_set_style_outline_color(btn1, lv_color_hex(0xFF0000), sty);
    lv_obj_set_style_outline_opa(btn1, 255, sty);
    lv_obj_set_style_outline_width(btn1, 1, sty);
    lv_obj_set_style_outline_pad(btn1, 4, sty);
    lv_obj_set_style_shadow_width(btn1, 0, sty);
    lv_obj_set_style_shadow_spread(btn1, 0, sty);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);
}
