#include <rtthread.h> 

#include "littlevgl2rtt.h" 
#include "lvgl.h" 

#include "nesplay.h"
#include "nes_main.h"	

//加载游戏界面
void nes_load_ui(void)
{	
	lv_obj_t * obj1;
    obj1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj1, LV_HOR_RES, LV_VER_RES);
    static lv_style_t style1;
    lv_style_copy(&style1, &lv_style_plain_color);
    style1.body.main_color = LV_COLOR_BLACK;
    style1.body.grad_color = LV_COLOR_BLACK;
    lv_obj_set_style(obj1, &style1);
    lv_obj_align(obj1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

	static lv_style_t style_txt;
    lv_style_copy(&style_txt, &lv_style_transp);
    style_txt.text.font = &lv_font_dejavu_20;
    style_txt.text.color = LV_COLOR_CYAN;

    lv_obj_t *txt = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(txt, &style_txt);
    lv_label_set_text(txt, "NES&SMS Emulator");
    lv_obj_align(txt, NULL, LV_ALIGN_CENTER, 0, 0);
} 

void nes_clear_lcd_black(void)
{
	lv_obj_t * obj1;
    obj1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj1, LV_HOR_RES, LV_VER_RES);
    static lv_style_t style1;
    lv_style_copy(&style1, &lv_style_plain_color);
    style1.body.main_color = LV_COLOR_BLACK;
    style1.body.grad_color = LV_COLOR_BLACK;
    lv_obj_set_style(obj1, &style1);
    lv_obj_align(obj1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
}

//NES游戏
uint16_t nes_play(void)
{
	nes_clear_lcd_black();
	nes_load("/nes/supermali.nes");
    // nes_load("/nes/maoxiandao.nes");

	return 0;  								  
}






















