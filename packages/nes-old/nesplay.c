// #include "stm32f10x.h"   /* for data type */
#include <rtthread.h>
#include <dfs_posix.h>
#include "littlevgl2rtt.h" 
#include <lvgl.h>

#include "nesplay.h"
#include "nes_main.h"	 
#include "nes_rom.h"

#include <board.h>
#include "nesplay.h"

#define NES_PATH       		 "/nes"
#define NES_LIST_PATH        "/nes/filelist.txt"

#define FILE_NAME_LEN	100			/* �洢���ļ�����󳤶� */
#define FILE_NUM	    80

// #if defined(_ILI_HORIZONTAL_DIRECTION_)
// #define key_enter_GETVALUE()  GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_7)
// #define key_down_GETVALUE()   GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_3)
// #define key_up_GETVALUE()     GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_15)
// #define key_right_GETVALUE()  GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_13)
// #define key_left_GETVALUE()   GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_14)
// #else
// #define key_enter_GETVALUE()  GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_7)
// #define key_down_GETVALUE()   GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_14)
// #define key_up_GETVALUE()     GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_13)
// #define key_right_GETVALUE()  GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_3)
// #define key_left_GETVALUE()   GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_15)
// #endif

static rt_err_t scan_files (const char* path);

volatile uint32_t s_fileNum;
volatile uint32_t s_fileIndex;
volatile uint32_t s_stop;


/*�ο����� xiaowei061 �Ĵ���  �����������¾�*/
uint8_t *rom_file;


/* ��nes�ļ������ڴ� ���cpuƵ�ʵ�128M */
int load_nes(uint8_t* path)
{
	volatile rt_uint32_t len,br;
	rt_uint32_t file_size;
	int f_nes;		 

	rt_kprintf("to load nes file %s \n",path);
	//lcd_fill_rect(0,24, 320-24,240,Black);
#if defined(_ILI_HORIZONTAL_DIRECTION_)
	// lcd_fill_rect(24,0, 240,400-24,Black);
	// lcd_fill_rect(400-24,0, 240,12,Blue2);
	// lcd_fill_rect(400-12,0,240,12,Blue2-100);
    // lcd_display_string(400-16-4,5,path+5,Cyan,9999,1);
	lv_obj_t * obj1;
    obj1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj1, 400, 240);
    lv_obj_set_style(obj1, &lv_style_plain_color);
    lv_obj_align(obj1, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 100);
#else
	lv_obj_t * obj1;
    obj1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj1, 400, 240);
    static lv_style_t style1;
    lv_style_copy(&style1, &lv_style_plain_color);
    style1.body.main_color = LV_COLOR_BLACK;
    style1.body.grad_color = LV_COLOR_BLACK;
    lv_obj_set_style(obj1, &style1);
    lv_obj_align(obj1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 20);
	lv_obj_t *label = lv_label_create(obj1, NULL);
    lv_label_set_text(label, path+5);
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 40);
#endif	
	rt_thread_delay(100);
 	f_nes=open((const char *)path,O_RDONLY, 0);
	
	if(f_nes>=0)
	{
		/* �ȵõ��ļ��ܴ�С*/
		file_size = lseek(f_nes, 0, SEEK_END);
		/* ��λ���ļ���ʼ��*/
		lseek(f_nes, 0, SEEK_SET);
		rom_file=(uint8_t *)rt_malloc(file_size);
		if (rom_file==RT_NULL)
		{
			rt_kprintf("mem malloc for rom_file faile\r\n");
			close(f_nes);
			return -1;
		}		
		
		/*����Ϸ�ļ��������ڲ�ram*/   
		br=read(f_nes,rom_file,file_size);			
		close(f_nes);
		
		nes_main();

		rt_free(rom_file);		
		return 0;
	}
	else
	{
		return -2;
	}	
} 

/* NES��Ϸ�߳� */
void nes_play(void* paramete)
{
	rt_err_t result;
	char fn_buff[100];
	int fd;
	

	/*���Ʊ���ͼƬ*/
	//lcd_show_bmp(0,0,"/resource/mali.bmp");
	
	//screen_shot( 0, 0, 240, 320, "/power_on.bmp");
	/*���Ʊ���*/
#if defined(_ILI_HORIZONTAL_DIRECTION_)
	// lcd_fill_rect(0,0, 240,12,Blue2);
	// lcd_fill_rect(12,0,240,12,Blue2-100);
	// lcd_display_string(4,72,"NESģ����",Cyan,9999,1);
#else
	// lcd_fill_rect(0,0, 12,240,Blue2);
	// lcd_fill_rect(0,12,12,240,Blue2-100);
	// //lcd_display_string(82,4,"��Ϊ��Ϸ��",Cyan,9999,1);
	// lcd_display_string(90,4,"NESģ����",Cyan,9999,1);
	// //lcd_display_string(82,4,"��Ϊ��Ϸ��",Blue,9999,0);
#endif
	rt_thread_delay( RT_TICK_PER_SECOND*1 ); 
	
	s_fileIndex = 0;
	s_fileNum = 0;
	if ((result = scan_files(NES_PATH))!=0)/* ɨ��nes�ļ������ļ�������nesFileName��ָ���ڴ�ռ�*/ 
	{
		rt_kprintf(" nes file scan  failed./r/n");
		return ;
	}
	if (s_fileNum<1) return;
	
	s_fileIndex=0;
	// while (1)
	// {	
		/*��ȡ�ļ��б��еĸ�������*/
	    fd = open (NES_LIST_PATH, O_RDONLY, 0);
	    lseek (fd, s_fileIndex*FILE_NAME_LEN, SEEK_SET);
        read(fd, fn_buff, FILE_NAME_LEN);
        close (fd);
		
		/*��ʼnes��Ϸ*/
		result = load_nes((uint8_t *)fn_buff);	
		// rt_thread_delay(2); 
	// }	
						  
}


/************************************************************************************
*	�� �� ��: rt_err_t scan_files (char* path) 
*	����˵��: �ļ�����
*   ��ڲ�����*path:Ҫɨ���·��
*   ���ڲ����� 0: ��ȷɨ�赽nes�ļ�
*              1: ��ջ����ʧ��
*              2��Ŀ¼�򿪻��ȡʧ��
*              3����ȡ�ļ�����ʧ��
*   ˵    ����
*   ���÷����� 
*************************************************************************************/
rt_err_t scan_files (const char* path) 
{ 
    rt_err_t res = 0; 
    DIR * dp; 
	struct dirent *dirp;
	int i;
	char *nes_Name;
	int fd;
	
	rt_kprintf("\r\n to read files .\r\n", path);
	
	nes_Name = rt_calloc(1,FILE_NAME_LEN*FILE_NUM);
	if (nes_Name==0)
	{
		return 1;
	}	
    dp = opendir(path); 
	if (dp == NULL)
	{
		rt_kprintf("open dir %s failed .\r\n", path);
		res = 2;
		goto scanfail;
    }
	{ 
        for (;;) 
		{ 
            dirp = readdir(dp); 
            if ((dirp == NULL) || (dirp->d_name[0] == 0)) 
			{	
				rt_kprintf("read dir %s over . \r\n \r\n", path);
				res = 0;
				goto scanfail;
			}

            if (dirp->d_name[0] == '.') continue; 
	
			if ((dirp->d_type)&FT_DIRECTORY)
			{ 
            //    res = scan_files(&(dirp->d_name[0]));/*�ݹ���� */ 
            //    if (res != 0) 
			//	    break;
				continue;
            } 
			else 
			{ 
                rt_kprintf("%s/%s. \r\n", path, &(dirp->d_name[0]));
				i = dirp->d_namlen;
				if ((((dirp->d_name[i-3])=='n')||((dirp->d_name[i-3])=='N')) && (((dirp->d_name[i-2])=='e')||((dirp->d_name[i-2])=='E')) && \
					(((dirp->d_name[i-1])=='s')||((dirp->d_name[i-1])=='S')))
			
				{
					if ((strlen(path) + dirp->d_namlen)<FILE_NAME_LEN-2)
					{
						sprintf(&nes_Name[s_fileNum*FILE_NAME_LEN], "%s/%s", path, &(dirp->d_name[0])); 
						
						s_fileNum++;//��¼�ļ�����
						if (s_fileNum>=FILE_NUM)
						{	
							   s_fileNum =  FILE_NUM-1;
						}
					}
				}
            } 
        } 
    } 
	
scanfail:
	closedir(dp);
	if (s_fileNum>=1)
	{	
		fd = open(NES_LIST_PATH, O_WRONLY | O_TRUNC, 0);
		if (fd < 0)
		{
			rt_kprintf("save file list fail \r\n");
		}
		else
		{
			write(fd, nes_Name, FILE_NAME_LEN*FILE_NUM);
			close(fd);
		}
	}	
	rt_free(nes_Name);
    return res; 
} 





















