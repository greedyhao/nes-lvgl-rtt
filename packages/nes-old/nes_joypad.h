 
#ifndef _NES_JOYPAD_H_
#define _NES_JOYPAD_H_
// #include "joypad.h"
#include <rtthread.h>


#define JOYPAD_0 	0
#define JOYPAD_1 	1	 
 
typedef struct{
	rt_uint8_t state;   //״̬
	rt_uint8_t  index;	//��ǰ��ȡλ
	rt_uint32_t value;	//JoyPad ��ǰֵ	
}JoyPadType;

/* function ------------------------------------------------------------------*/
void NES_JoyPadInit(void);
void NES_JoyPadReset(void);
void NES_JoyPadDisable(void);
rt_uint8_t NES_GetJoyPadVlaue(int JoyPadNum);


#endif 













