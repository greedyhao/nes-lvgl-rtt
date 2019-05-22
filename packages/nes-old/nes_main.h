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
	char filetype[4]; 	//�ַ�����NES^Z������ʶ��.NES�ļ� 		 
	rt_uint8_t romnum;			//16kB ROM����Ŀ 						 
	rt_uint8_t vromnum;			//8kB VROM����Ŀ				 
	rt_uint8_t romfeature;		//D0��1����ֱ����0��ˮƽ���� 
						// D1��1���е�ؼ��䣬SRAM��ַ$6000-$7FFF
						// D2��1����$7000-$71FF��һ��512�ֽڵ�trainer 
						// D3��1��4��ĻVRAM���� 
						//  D4��D7��ROM Mapper��?1?74?1?7 	  
	rt_uint8_t rommappernum;	// D0��D3��������������0��׼����Ϊ��Mapper��^_^��
						// D4��D7��ROM Mapper�ĸ�4λ 		    
	//uint8_t reserve[8];	// ������������0 					    
	//OM���������У��������trainer������512�ֽڰ���ROM��֮ǰ 
	//VROM��, �������� 
}NesHeader;																		 

rt_uint8_t nes_main(void);
void NesFrameCycle(void);
void NES_ReadJoyPad(rt_uint8_t JoyPadNum);


//PPUʹ��
extern rt_uint8_t *NameTable;			//2K�ı���
extern rt_uint16_t	*Buffer_scanline;	//����ʾ����,���±�Խ�����Ϊ7����ʾ�� 7 ~ 263  0~7 263~270 Ϊ��ֹ�����
//CPUʹ��
extern rt_uint8_t *ram6502;  			//RAM  2K�ֽ�,��malloc����

rt_uint8_t nes_mem_creat(void);		//����nes���������RAM.
void nes_mem_delete(void);	//ɾ��nes����ʱ�����RAM	
										 
#endif











