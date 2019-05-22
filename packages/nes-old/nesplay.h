#ifndef __NESPLAY_H
#define __NESPLAY_H 	

#include <rtthread.h>
#include <lvgl.h>
#include "rtconfig.h"

extern uint8_t nesruning ;	//退出NES的标志
extern uint8_t frame_cnt;	//统计帧数



// lv_img_dsc_t nes_fb_img_src= {
//   .header.always_zero = 0,
//   .header.w = LV_HOR_RES,         //2D buffer
//   .header.h = LV_VER_RES,          //2D buffer
//   .data_size = LV_HOR_RES * LV_VER_RES * LV_COLOR_SIZE / 8,
//   .header.cf = LV_IMG_CF_TRUE_COLOR,
// //   .data = nes_frame_buffer,
// };

int load_nes(uint8_t* path);  
void nes_play(void* paramete);
#endif























