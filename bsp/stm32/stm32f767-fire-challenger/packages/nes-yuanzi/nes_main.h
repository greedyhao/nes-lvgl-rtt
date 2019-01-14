#ifndef __NES_MAIN_H
#define __NES_MAIN_H

#include <rtthread.h>

#define NES_SKIP_FRAME 	2		//定义模拟器跳帧数,默认跳2帧
 
 
#define INLINE 	static inline
#define int8 	char
#define int16 	short
#define int32 	int
#define uint8 	unsigned char
#define uint16 	unsigned short
#define uint32 	unsigned int
#define boolean uint8 



//nes信息头结构体
typedef struct
{
	unsigned char id[3]; // 'NES'
	unsigned char ctrl_z; // control-z
	unsigned char num_16k_rom_banks;
	unsigned char num_8k_vrom_banks;
	unsigned char flags_1;
	unsigned char flags_2;
	unsigned char reserved[8];
}NES_header;   

extern uint8_t nes_frame_cnt;		//nes帧计数器
extern int MapperNo;			//map编号
extern int NES_scanline;		//扫描线
extern NES_header *RomHeader;	//rom文件头 
extern int VROM_1K_SIZE;
extern int VROM_8K_SIZE; 
extern uint8_t cpunmi;  				//cpu中断标志  在 6502.s里面
extern uint8_t cpuirq;			
extern uint8_t PADdata;   			//手柄1键值 
extern uint8_t PADdata1;   			//手柄1键值 
extern uint8_t lianan_biao;			//连按标志 
#define  CPU_NMI  cpunmi=1;
#define  CPU_IRQ  cpuirq=1;
#define  NES_RAM_SPEED	0 	 	//1:内存占用小  0:速度快


void cpu6502_init(void);		//在 cart.s
void run6502(uint32_t); 		   		//在 6502.s 
uint8_t nes_load_rom(void);
void nes_sram_free(void);
uint8_t nes_sram_malloc(uint32_t romsize);
uint8_t nes_load(const char* pname);
void nes_set_window(void);
void nes_get_gamepadval(void);
void nes_emulate_frame(void);
void debug_6502(uint16_t reg0,uint8_t reg1);

void nes_sai_dma_tx_callback(void);
int nes_sound_open(int samples_per_sync,int sample_rate);
void nes_sound_close(void);
void nes_apu_fill_buffer(int samples,uint16_t* wavebuf); 

extern uint32_t get_crc32(uint8_t* buf, uint32_t len);
#endif







