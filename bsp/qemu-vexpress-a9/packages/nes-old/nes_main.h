#ifndef _NES_MAIN_H_
#define _NES_MAIN_H_

#include <rtthread.h>
// #include <drv_gpio.h>
#include <stdio.h>
#include <string.h>   
#include "6502.h"
#include "PPU.h"
#include "nes_joypad.h"	 
#include "nes_rom.h"
#include "nes_main.h"

// #if defined(_ILI_HORIZONTAL_DIRECTION_)
// #define key_enter_GETVALUE()  rt_pin_read(GET_PIN(G,7))
// #define key_down_GETVALUE()   rt_pin_read(GET_PIN(D,3))
// #define key_up_GETVALUE()     rt_pin_read(GET_PIN(G,15))
// #define key_right_GETVALUE()  rt_pin_read(GET_PIN(G,13))
// #define key_left_GETVALUE()   rt_pin_read(GET_PIN(G,14))
// #else
// #define key_enter_GETVALUE()  rt_pin_read(GET_PIN(G,7))
// #define key_down_GETVALUE()   rt_pin_read(GET_PIN(G,14))
// #define key_up_GETVALUE()     rt_pin_read(GET_PIN(G,13))
// #define key_right_GETVALUE()  rt_pin_read(GET_PIN(D,3))
// #define key_left_GETVALUE()   rt_pin_read(GET_PIN(G,15))
// #endif

#define key_enter_GETVALUE() 
#define key_down_GETVALUE()   
#define key_up_GETVALUE()     
#define key_right_GETVALUE()  
#define key_left_GETVALUE()  

typedef struct
{
	char filetype[4]; 	//字符串“NES^Z”用来识别.NES文件 		 
	rt_uint8_t romnum;			//16kB ROM的数目 						 
	rt_uint8_t vromnum;			//8kB VROM的数目				 
	rt_uint8_t romfeature;		//D0：1＝垂直镜像，0＝水平镜像 
						// D1：1＝有电池记忆，SRAM地址$6000-$7FFF
						// D2：1＝在$7000-$71FF有一个512字节的trainer 
						// D3：1＝4屏幕VRAM布局 
						//  D4－D7：ROM Mapper的?1?74?1?7 	  
	rt_uint8_t rommappernum;	// D0－D3：保留，必须是0（准备作为副Mapper号^_^）
						// D4－D7：ROM Mapper的高4位 		    
	//uint8_t reserve[8];	// 保留，必须是0 					    
	//OM段升序排列，如果存在trainer，它的512字节摆在ROM段之前 
	//VROM段, 升序排列 
}NesHeader;																		 

rt_uint8_t nes_main(void);
void NesFrameCycle(void);
void NES_ReadJoyPad(rt_uint8_t JoyPadNum);


//PPU使用
extern rt_uint8_t *NameTable;			//2K的变量
extern rt_uint16_t	*Buffer_scanline;	//行显示缓存,上下标越界最大为7，显示区 7 ~ 263  0~7 263~270 为防止溢出区
//CPU使用
extern rt_uint8_t *ram6502;  			//RAM  2K字节,由malloc申请

rt_uint8_t nes_mem_creat(void);		//开辟nes运行所需的RAM.
void nes_mem_delete(void);	//删除nes运行时申请的RAM	
										 
#endif











