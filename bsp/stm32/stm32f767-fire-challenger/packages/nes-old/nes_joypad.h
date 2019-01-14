 
#ifndef _NES_JOYPAD_H_
#define _NES_JOYPAD_H_
// #include "joypad.h"
#include <rtthread.h>


#define JOYPAD_0 	0
#define JOYPAD_1 	1	 
 
typedef struct{
	rt_uint8_t state;   //状态
	rt_uint8_t  index;	//当前读取位
	rt_uint32_t value;	//JoyPad 当前值	
}JoyPadType;

/* function ------------------------------------------------------------------*/
void NES_JoyPadInit(void);
void NES_JoyPadReset(void);
void NES_JoyPadDisable(void);
rt_uint8_t NES_GetJoyPadVlaue(int JoyPadNum);


#endif 













