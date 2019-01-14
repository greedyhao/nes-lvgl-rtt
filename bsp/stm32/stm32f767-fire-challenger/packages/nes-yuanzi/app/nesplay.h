#ifndef __NESPLAY_H
#define __NESPLAY_H 	

#include <rtthread.h> 

extern uint8_t nesruning ;	//退出NES的标志
extern uint8_t frame_cnt;	//统计帧数
 
// void nes_clock_set(uint8_t PLL);	 
void load_nes(uint8_t* path);   
uint16_t nes_play(void);

#endif























