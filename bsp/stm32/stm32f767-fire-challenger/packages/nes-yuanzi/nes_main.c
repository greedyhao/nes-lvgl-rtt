#include <rtthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <dfs_posix.h>
#include <string.h>
#include <sys/types.h>

#include "nes_main.h" 
#include "nes_ppu.h"
#include "nes_mapper.h"
#include "nes_apu.h"

//#define DRV_DEBUG
#define LOG_TAG             "nes.sys.main"
#include <drv_log.h>


// struct NesHeader_tag
// {
//   uint8_t byID[ 4 ];
//   uint8_t byRomSize;
//   uint8_t byVRomSize;
//   uint8_t byInfo1;
//   uint8_t byInfo2;
//   uint8_t byReserve[ 8 ];
// };

// struct NesHeader_tag NesHeader;	

extern volatile uint8_t framecnt;	//nes帧计数器 
int MapperNo;			//map编号
int NES_scanline;		//nes扫描线
int VROM_1K_SIZE;
int VROM_8K_SIZE;
uint32_t NESrom_crc32;

uint8_t PADdata0;   			//手柄1键值 [7:0]右7 左6 下5 上4 Start3 Select2 B1 A0  
uint8_t PADdata1;   			//手柄2键值 [7:0]右7 左6 下5 上4 Start3 Select2 B1 A0  
uint8_t *NES_RAM;			//保持1024字节对齐
uint8_t *NES_SRAM;  
NES_header *RomHeader; 	//rom文件头
MAPPER *NES_Mapper;		 
MapperCommRes *MAPx;  


uint8_t* spr_ram;			//精灵RAM,256字节
ppu_data* ppu;			//ppu指针
uint8_t* VROM_banks;
uint8_t* VROM_tiles;

apu_t *apu; 			//apu指针
uint16_t *wave_buffers; 		
uint16_t *saibuf1; 			//音频缓冲帧,占用内存数 367*4 字节@22050Hz
uint16_t *saibuf2; 			//音频缓冲帧,占用内存数 367*4 字节@22050Hz

uint8_t* romfile;			//nes文件指针,指向整个nes文件的起始地址.

//加载ROM
//返回值:0,成功
//    1,内存错误
//    3,map错误
uint8_t nes_load_rom(void)
{  
    uint8_t* p;  
	uint8_t i;
	uint8_t ret=0;
	p=(uint8_t*)romfile;
	
	// if(strncmp((char*)p,"NES",3)==0)
	if(memcmp(p,"NES",3)==0)
	{  
		RomHeader->ctrl_z=p[3];
		RomHeader->num_16k_rom_banks=p[4];
		RomHeader->num_8k_vrom_banks=p[5];
		RomHeader->flags_1=p[6];
		RomHeader->flags_2=p[7]; 
		if(RomHeader->flags_1&0x04)p+=512;		//有512字节的trainer:
		if(RomHeader->num_8k_vrom_banks>0)		//存在VROM,进行预解码
		{		
			VROM_banks=p+16+(RomHeader->num_16k_rom_banks*0x4000);
#if	NES_RAM_SPEED==1	//1:内存占用小 0:速度快	 
			VROM_tiles=VROM_banks;	 
#else  
			VROM_tiles=malloc(RomHeader->num_8k_vrom_banks*8*1024);//这里可能申请多达1MB内存!!!
			if(VROM_tiles==0)VROM_tiles=VROM_banks;//内存不够用的情况下,尝试VROM_titles与VROM_banks共用内存			
			compile(RomHeader->num_8k_vrom_banks*8*1024/16,VROM_banks,VROM_tiles);  
#endif	
		}else 
		{
			VROM_banks=malloc(8*1024);
			VROM_tiles=malloc(8*1024);
			if(!VROM_banks||!VROM_tiles)ret=1;
		}  	
		VROM_1K_SIZE = RomHeader->num_8k_vrom_banks * 8;
		VROM_8K_SIZE = RomHeader->num_8k_vrom_banks;  
		MapperNo=(RomHeader->flags_1>>4)|(RomHeader->flags_2&0xf0);
		if(RomHeader->flags_2 & 0x0E)MapperNo=RomHeader->flags_1>>4;//忽略高四位，如果头看起来很糟糕 
		rt_kprintf("use map:%d\r\n",MapperNo);
		for(i=0;i<255;i++)  // 查找支持的Mapper号
		{		
			if (MapTab[i]==MapperNo)break;		
			if (MapTab[i]==-1)ret=3; 
		} 
		if(ret==0)
		{
			switch(MapperNo)
			{
				case 1:  
					MAP1=malloc(sizeof(Mapper1Res)); 
					if(!MAP1)ret=1;
					break;
				case 4:  
				case 6: 
				case 16:
				case 17:
				case 18:
				case 19:
				case 21: 
				case 23:
				case 24:
				case 25:
				case 64:
				case 65:
				case 67:
				case 69:
				case 85:
				case 189:
					MAPx=malloc(sizeof(MapperCommRes)); 
					if(!MAPx)ret=1;
					break;  
				default:
					break;
			}
		}
	}
	else
	{
		return RT_ERROR;
	} 
	return ret;	//返回执行结果
} 

//释放内存 
void nes_sram_free(void)
{ 
	free(NES_RAM);
	free(NES_SRAM);
	free(RomHeader);
	free(NES_Mapper);
	free(spr_ram);
	free(ppu);
	free(apu);
	free(wave_buffers);
	free(saibuf1);
	free(saibuf2);
	free(romfile);
	if((VROM_tiles!=VROM_banks)&&VROM_banks&&VROM_tiles)//如果分别为VROM_banks和VROM_tiles申请了内存,则释放
	{
		free(VROM_banks);	 
		free(VROM_tiles);
	}
	switch (MapperNo)//释放map内存
	{
		case 1: 			//释放内存			
			free(MAP1);
			break;	 	
		case 4: 
		case 6: 
		case 16:
		case 17:
		case 18:
		case 19:
		case 21:
		case 23:
		case 24:
		case 25:
		case 64:
		case 65:
		case 67:
		case 69:
		case 85:
		case 189:
			free(MAPx);
		default:break; 
	}
	NES_RAM=0;	
	NES_SRAM=0;
	RomHeader=0;
	NES_Mapper=0;
	spr_ram=0;
	ppu=0;
	apu=0;
	wave_buffers=0;
	saibuf1=0;
	saibuf2=0;
	romfile=0; 
	VROM_banks=0;
	VROM_tiles=0; 
	MAP1=0;
	MAPx=0;
} 

//为NES运行申请内存
//romsize:nes文件大小
//返回值:0,申请成功
//       1,申请失败
uint8_t nes_sram_malloc(uint32_t romsize)
{	
	NES_RAM = rt_malloc_align(0x800,1024);
 	NES_SRAM=malloc(0X2000);
	RomHeader=malloc(sizeof(NES_header));
	NES_Mapper=malloc(sizeof(MAPPER));
	spr_ram=malloc(0X100);		
	ppu=malloc(sizeof(ppu_data));  
	// apu=malloc(sizeof(apu_t));		//sizeof(apu_t)=  12588
	wave_buffers=malloc(APU_PCMBUF_SIZE*2);
	saibuf1=malloc(APU_PCMBUF_SIZE*4+10);
	saibuf2=malloc(APU_PCMBUF_SIZE*4+10);
 	// romfile=malloc(romsize);			//申请游戏rom空间,等于nes文件大小  
	if(!NES_RAM||!NES_SRAM||!RomHeader||!NES_Mapper||!spr_ram||!ppu||!apu||!wave_buffers||!saibuf1||!saibuf2||!romfile)
	{
		nes_sram_free();
		return 1;
	}
	memset(NES_SRAM,0,0X2000);				//清零
	memset(RomHeader,0,sizeof(NES_header));	//清零
	memset(NES_Mapper,0,sizeof(MAPPER));	//清零
	memset(spr_ram,0,0X100);				//清零
	memset(ppu,0,sizeof(ppu_data));			//清零
	// memset(apu,0,sizeof(apu_t));			//清零
	memset(wave_buffers,0,APU_PCMBUF_SIZE*2);//清零
	memset(saibuf1,0,APU_PCMBUF_SIZE*4+10);	//清零
	memset(saibuf2,0,APU_PCMBUF_SIZE*4+10);	//清零
	// memset(romfile,0,romsize);				//清零 

	rt_kprintf("sram malloc success.\n");

	return 0;
} 
//开始nes游戏
//pname:nes游戏路径
//返回值:
//0,正常退出
//1,内存错误
//2,文件错误
//3,不支持的map
uint8_t nes_load(const char* pname)
{
	// int fd;
	
	uint8_t *buf;			//缓存
	// uint8_t *p;
	// uint32_t readlen;		//总读取长度
	// uint16_t bread;			//读取的长度
	
	uint8_t ret=0;  
	// if(audiodev.status&(1<<7))//当前在放歌??
	// {
	// 	audio_stop_req(&audiodev);	//停止音频播放
	// 	audio_task_delete();		//删除音乐播放任务.
	// }  
	// app_wm8978_volset(wm8978set.mvol);	 
	// WM8978_ADDA_Cfg(1,0);	//开启DAC
	// WM8978_Input_Cfg(0,0,0);//关闭输入通道
	// WM8978_Output_Cfg(1,0);	//开启DAC输出
	
	// buf=malloc(1024);  
	// file=malloc(sizeof(FIL));  
	

	// if(file==0)//内存申请失败.
	// {
	// 	myfree(buf);
	// 	return 1;						  
	// }
	// ret=f_open(file,(char*)pname,FA_READ);

	// fd = open(pname,O_RDONLY | O_BINARY);
	// if (fd < 0)
	// {
	// 	LOG_E("open nes file failed!\n");
	// 	close(fd);
	// 	free(buf);
	// 	return 1;
	// }

	

	FILE *fp;

	/* Open ROM file */
	fp = fopen( pname, "rb" );
	if ( fp == NULL )
		return 1;

	/* Read ROM Header */
	fread( romfile, sizeof RomHeader, 1, fp );
	// if ( memcmp( NesHeader.byID, "NES\x1a", 4 ) != 0 )
	if ( memcmp( romfile, "NES\x1a", 4 ) != 0 )
	{
		/* not .nes file */
		LOG_E("not nes file!\n");
		fclose( fp );
		return 1;
	}
	else
	{
		LOG_I("nes check successful.\n");
	}

	nes_sram_malloc(0);

	/* If trainer presents Read Triner at 0x7000-0x71ff */
	if ( RomHeader->flags_1 & 4 )
	{
		fread( &NES_SRAM[ 0x1000 ], 512, 1, fp );
	}

	romfile = malloc(RomHeader->num_16k_rom_banks * 0x4000);

	// fread( romfile, 0x4000, RomHeader->num_16k_rom_banks, fp );

	// if (RomHeader->num_8k_vrom_banks > 0)
	// {
	// 	/* Allocate Memory for VROM Image */
	// 	VROM_banks = (BYTE *)malloc( NesHeader.byVRomSize * 0x2000 );

	// 	/* Read VROM Image */
	// 	fread( VROM, 0x2000, NesHeader.byVRomSize, fp );
	// }

	// fread( VROM_tiles, 8*1024, RomHeader->num_8k_vrom_banks, fp );

	// fclose(fp);

	// // if(ret!=FR_OK)	//打开文件失败
	// // {
	// // 	myfree(buf);
	// // 	myfree(file);
	// // 	return 2;
	// // }	 

	// off_t size = lseek(fd, 0, SEEK_END);
	// ret=nes_sram_malloc(size);			//申请内存 

	// // if(ret==0)
	// // {
	// // 	p=romfile;
	// // 	readlen=0;
	// // 	while(readlen<file->obj.objsize)
	// // 	{
	// // 		ret=f_read(file,buf,1024,(UINT*)&bread);//读出文件内容
	// // 		readlen+=bread;
	// // 		mymemcpy(p,buf,bread); 
	// // 		p+=bread;
	// // 		if(ret)break;
	// // 	}    
	// // 	NESrom_crc32=get_crc32(romfile+16, file->obj.objsize-16);//获取CRC32的值	
	// // 	ret=nes_load_rom();						//加载ROM
	// // 	if(ret==0) 					
	// // 	{   
	// // 		cpu6502_init();						//初始化6502,并复位	 
	// // 		Mapper_Init();						//map初始化 	 
	// // 		PPU_reset();						//ppu复位
	// // 		apu_init(); 						//apu初始化 
	// // 		nes_sound_open(0,APU_SAMPLE_RATE);	//初始化播放设备
	// // 		nes_emulate_frame();				//进入NES模拟器主循环 
	// // 		nes_sound_close();					//关闭声音输出
	// // 	}
	// // }

	// LOG_I("size of nes file is:%ld",size);
	// buf = malloc(size+1);
	// if (!buf)
	// {
	// 	LOG_E("malloc for buffer failed!\n");
	// 	goto exit_nes_load_error;
	// }
	
	// // ret = read(fd, buf, size);
	// // if (ret <= 0)
	// // {
	// // 	LOG_E("read nes file to buf error!\n");
	// // 	goto exit_nes_load_error;
	// // }

	// ssize_t ret_r;
	// while (size != 0 && (ret_r = read(fd, buf, size)) != 0)
	// {
	// 	if (ret_r == -1) 
	// 	{
	// 		if (errno == EINTR)
	// 			continue;
	// 		LOG_E("read");
	// 		break;
	// 	}
	// 	size -= ret_r;
	// 	buf += ret_r;
	// }
	// LOG_I("read done");
	// if (memcpy(romfile,buf,size) == RT_NULL)
	// {
	// 	LOG_E("romfile memcpy error!\n");
	// 	goto exit_nes_load_error;
	// }
	

	

	// LOG_I("memcpy done");
	// LOG_I("buf[0-2]:%d %d %d",buf[0],buf[1],buf[2]);
	// LOG_I("romfile[0-2]:%d %d %d",romfile[0],romfile[1],romfile[2]);
	// LOG_I("NES:%d %d %d",'N','E','S');

	// NESrom_crc32=get_crc32(romfile+16, size-16);//获取CRC32的值
	// LOG_I("get_crc32 done,NESrom_crc32:%d",NESrom_crc32);

	ret = nes_load_rom();
	if (ret == RT_ERROR)
		LOG_E("nes_load_rom error!");
	// LOG_I("nes_load_rom done");
	// if(ret==0) 					
	// {   
	// 	// cpu6502_init();						//初始化6502,并复位
	// 	Mapper_Init();						//map初始化 	 
	// 	PPU_reset();						//ppu复位
	// 	// apu_init(); 						//apu初始化 
	// 	// nes_sound_open(0,APU_SAMPLE_RATE);	//初始化播放设备
	// 	// nes_emulate_frame();				//进入NES模拟器主循环 
	// 	// nes_sound_close();					//关闭声音输出
	// }
	// LOG_I("init done");
	
	// close(fd);
	// free(buf);
	// // nes_sram_free();	//释放内存
	// LOG_I("free done");
	fclose(fp);
	return ret;

exit_nes_load_error:
	// close(fd);
	fclose(fp);
	free(buf);
	nes_sram_free();	//释放内存
	return RT_ERROR;
}  
uint16_t nes_xoff=0;	//显示在x轴方向的偏移量(实际显示宽度=256-2*nes_xoff)
uint16_t nes_yoff=0;	//显示在y轴方向的偏移量

//RGB屏需要的3个参数
//扩大4倍,参数计算方法(800*480为例):
//offset=lcdltdc.pixsize*(lcdltdc.pwidth*(lcdltdc.pheight-(i-sx)*2-1)+nes_yoff+LineNo) 
//offset=2*(800*(480+(sx-i)*2-1)+nes_yoff+LineNo)
//      =1600*(480+(sx-i)*2-1)+2*nes_yoff+LineNo*2
//      =766400+3200*(sx-i)+2*nes_yoff+LineNo*2 
//nes_rgb_parm1=766400
//nes_rgb_parm2=3200
//nes_rgb_parm3=nes_rgb_parm2/2

//不扩大,参数计算方法(480*272为例):
//offset=lcdltdc.pixsize*(lcdltdc.pwidth*(lcdltdc.pheight-(i-sx)-1)+nes_yoff+LineNo*2) 
//offset=2*(480*(272+sx-i-1)+nes_yoff+LineNo*2)
//      =960*(272+sx-i-1)+2*nes_yoff+LineNo*4
//      =260160+960*(sx-i)+2*nes_yoff+LineNo*4 
//nes_rgb_parm1=260160
//nes_rgb_parm2=960
//nes_rgb_parm3=nes_rgb_parm2/2

uint32_t nes_rgb_parm1;
uint16_t nes_rgb_parm2;
uint16_t nes_rgb_parm3;

// //设置游戏显示窗口
// void nes_set_window(void)
// {	
// 	uint16_t xoff=0,yoff=0; 
// 	uint16_t lcdwidth,lcdheight;
// 	if(lcddev.width==240)
// 	{
// 		lcdwidth=240;
// 		lcdheight=240;
// 		nes_xoff=(256-lcddev.width)/2;	//得到x轴方向的偏移量
// 	}else if(lcddev.width<=320) 
// 	{
// 		lcdwidth=256;
// 		lcdheight=240; 
// 		nes_xoff=0;
// 	}else if(lcddev.width>=480)
// 	{
// 		lcdwidth=480;
// 		lcdheight=480; 
// 		nes_xoff=(256-(lcdwidth/2))/2;//得到x轴方向的偏移量
// 	}
// 	xoff=(lcddev.width-lcdwidth)/2;  
// 	if(lcdltdc.pwidth)//RGB屏
// 	{
// 		if(lcddev.id==0X4342)nes_rgb_parm2=lcddev.height*2;
// 		else nes_rgb_parm2=lcddev.height*2*2; 
// 		nes_rgb_parm3=nes_rgb_parm2/2;
// 		if(lcddev.id==0X4342)nes_rgb_parm1=260160-nes_rgb_parm2*xoff; 
// 		else if(lcddev.id==0X7084)nes_rgb_parm1=766400-nes_rgb_parm3*xoff; 
// 		else if(lcddev.id==0X7016||lcddev.id==0X1016)nes_rgb_parm1=1226752-nes_rgb_parm3*xoff; 
// 		else if(lcddev.id==0X1018)nes_rgb_parm1=2045440-nes_rgb_parm3*xoff; 
// 	}
// 	yoff=(lcddev.height-lcdheight-gui_phy.tbheight)/2+gui_phy.tbheight;//屏幕高度 
// 	nes_yoff=yoff;
// 	LCD_Set_Window(xoff,yoff,lcdwidth,lcdheight);//让NES始终在屏幕的正中央显示
// 	LCD_SetCursor(xoff,yoff);
// 	LCD_WriteRAM_Prepare();//写入LCD RAM的准备  
// }
extern void KEYBRD_FCPAD_Decode(uint8_t *fcbuf,uint8_t mode);
// //读取游戏手柄数据
// void nes_get_gamepadval(void)
// {  
// 	uint8_t *pt;
// 	while((usbx.bDeviceState&0XC0)==0X40)//USB设备插入了,但是还没连接成功,猛查询.
// 	{
// 		usbapp_pulling();	//轮询处理USB事务
// 	}
// 	usbapp_pulling();		//轮询处理USB事务
// 	if(usbx.hdevclass==4)	//USB游戏手柄
// 	{	
// 		PADdata0=fcpad.ctrlval;
// 		PADdata1=0;
// 	}else if(usbx.hdevclass==3)//USB键盘模拟手柄
// 	{
// 		KEYBRD_FCPAD_Decode(pt,0);
// 		PADdata0=fcpad.ctrlval;
// 		PADdata1=fcpad1.ctrlval; 
// 	}	
// }    
// //nes模拟器主循环
// void nes_emulate_frame(void)
// {  
// 	uint8_t nes_frame; 
// 	// TIM8_Int_Init(10000-1,21600-1);//启动TIM8,1s中断一次	
// 	nes_set_window();//设置窗口
// 	while(1)
// 	{	
// 		// LINES 0-239
// 		PPU_start_frame();
// 		for(NES_scanline = 0; NES_scanline< 240; NES_scanline++)
// 		{
// 			run6502(113*256);
// 			NES_Mapper->HSync(NES_scanline);
// 			//扫描一行		  
// 			if(nes_frame==0)scanline_draw(NES_scanline);
// 			else do_scanline_and_dont_draw(NES_scanline); 
// 		}  
// 		NES_scanline=240;
// 		run6502(113*256);//运行1线
// 		NES_Mapper->HSync(NES_scanline); 
// 		start_vblank(); 
// 		if(NMI_enabled()) 
// 		{
// 			cpunmi=1;
// 			run6502(7*256);//运行中断
// 		}
// 		NES_Mapper->VSync();
// 		// LINES 242-261    
// 		for(NES_scanline=241;NES_scanline<262;NES_scanline++)
// 		{
// 			run6502(113*256);	  
// 			NES_Mapper->HSync(NES_scanline);		  
// 		}	   
// 		end_vblank(); 
// 		nes_get_gamepadval();//每3帧查询一次USB
// 		apu_soundoutput();//输出游戏声音	 
// 		framecnt++; 	
// 		nes_frame++;
// 		if(nes_frame>NES_SKIP_FRAME)nes_frame=0;//跳帧 
// 		if(system_task_return)break;//TPAD返回
// 		if(lcddev.id==0X1963)//对于1963,每更新一帧,都要重设窗口
// 		{
// 			nes_set_window();
// 		} 
// 	}
// 	LCD_Set_Window(0,0,lcddev.width,lcddev.height);//恢复屏幕窗口
// 	TIM8->CR1&=~(1<<0);//关闭定时器8
// }
//在6502.s里面被调用
void debug_6502(uint16_t reg0,uint8_t reg1)
{
	printf("6502 error:%x,%d\r\n",reg0,reg1);
}
////////////////////////////////////////////////////////////////////////////////// 	 
// //nes,音频输出支持部分
// vuint8_t nestransferend=0;	//sai传输完成标志
// vuint8_t neswitchbuf=0;		//saibufx指示标志
// //SAI音频播放回调函数
// void nes_sai_dma_tx_callback(void)
// {  
// 	if(DMA2_Stream3->CR&(1<<19))neswitchbuf=0; 
// 	else neswitchbuf=1;  
// 	nestransferend=1;
// }
// //NES打开音频输出
// int nes_sound_open(int samples_per_sync,int sample_rate) 
// {
// 	printf("sound open:%d\r\n",sample_rate);
// 	WM8978_ADDA_Cfg(1,0);	//开启DAC
// 	WM8978_Input_Cfg(0,0,0);//关闭输入通道
// 	WM8978_Output_Cfg(1,0);	//开启DAC输出  
// 	WM8978_I2S_Cfg(2,0);	//飞利浦标准,16位数据长度
// 	app_wm8978_volset(wm8978set.mvol);
// 	SAIA_Init(0,1,4);		//设置SAI,主发送,16位数据 
// 	SAIA_SampleRate_Set(sample_rate);		//设置采样率
// 	SAIA_TX_DMA_Init((uint8_t*)saibuf1,(uint8_t*)saibuf2,2*APU_PCMBUF_SIZE,1);//DMA配置 
//  	sai_tx_callback=nes_sai_dma_tx_callback;//回调函数指wav_sai_dma_callback
// 	SAI_Play_Start();						//开启DMA    
// 	return 1;
// }
// //NES关闭音频输出
// void nes_sound_close(void) 
// { 
// 	SAI_Play_Stop();
// 	app_wm8978_volset(0);				//关闭WM8978音量输出
// } 
// //NES音频输出到SAI缓存
// void nes_apu_fill_buffer(int samples,uint16_t* wavebuf)
// {	
//  	int i;	 
// 	while(!nestransferend)//等待音频传输结束
// 	{
// 		delay_ms(5);
// 	}
// 	nestransferend=0;
//     if(neswitchbuf==0)
// 	{
// 		for(i=0;i<APU_PCMBUF_SIZE;i++)
// 		{
// 			saibuf1[2*i]=wavebuf[i];
// 			saibuf1[2*i+1]=wavebuf[i];
// 		}
// 	}else 
// 	{
// 		for(i=0;i<APU_PCMBUF_SIZE;i++)
// 		{
// 			saibuf2[2*i]=wavebuf[i];
// 			saibuf2[2*i+1]=wavebuf[i];
// 		}
// 	}
// } 



















