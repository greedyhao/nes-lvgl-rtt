/*
 * File      : drv_lcd.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author         Notes
 * 
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "drv_lcd.h"

#ifdef BSP_USING_LCD

#include <lcd_port.h>
#include "drv_gpio.h"

//#define DRV_DEBUG
#define LOG_TAG             "drv.lcd"
#include <drv_log.h>

#if !defined(LCD_WIDTH) || !defined(LCD_HEIGHT)
    #error "Please config lcd pixel parameters."
#endif

#if !defined(LCD_HFP) || !defined(LCD_HBP) || !defined(LCD_HSW) || \
    !defined(LCD_VFP) || !defined(LCD_VBP) || !defined(LCD_VSW)
    #error "Please config lcd timing parameters."
#endif

#if !defined(LCD_BL_PIN) || !defined(LCD_DISP_PIN)
    #error "Please config lcd backlight or reset pin."
#endif

#define RGBLCD_DEVICE(dev)    (struct stm32_lcd*)(dev)

// #include "image1.h"
// #define LV_COLOR_DEPTH = 16
// #define LV_COLOR_16_SWAP = 0
// extern uint8_t *wallpaper1;

struct stm32_lcd
{
    struct rt_device device;
    struct rt_device_graphic_info info; 
    
    LTDC_HandleTypeDef  ltdc;
    DMA2D_HandleTypeDef dma2d;

	int width;
    int height;

	uint8_t *fb;
};
static struct stm32_lcd _lcd;
uint32_t active_layer = 0;

void rgb_delay(uint32_t d)
{
    rt_thread_delay(rt_tick_from_millisecond(d)); 
}

rt_err_t rgb_init(void)
{    
    uint32_t HFP = LCD_HFP;
    uint32_t VFP = LCD_VFP;
    uint32_t HBP = LCD_HBP;
    uint32_t VBP = LCD_VBP;
    uint32_t HSW = LCD_HSW;
    uint32_t VSW = LCD_VSW;
    /* ��ʼ��STM32��ʾ��ʱ�� */ 
    __HAL_RCC_LTDC_CLK_ENABLE();
    __HAL_RCC_LTDC_FORCE_RESET();
    __HAL_RCC_LTDC_RELEASE_RESET();

    __HAL_RCC_DMA2D_CLK_ENABLE();
    __HAL_RCC_DMA2D_FORCE_RESET();
    __HAL_RCC_DMA2D_RELEASE_RESET();
    
    /* ����NVIC */ 
    HAL_NVIC_SetPriority(LTDC_IRQn,  3, 0);
    HAL_NVIC_SetPriority(DMA2D_IRQn, 3, 0);

    HAL_NVIC_EnableIRQ(LTDC_IRQn);
    HAL_NVIC_EnableIRQ(DMA2D_IRQn);

    _lcd.ltdc.Instance = LTDC;  

    _lcd.ltdc.Init.HorizontalSync        =   (HSW - 1);
    _lcd.ltdc.Init.VerticalSync          =   (VSW - 1);
    _lcd.ltdc.Init.AccumulatedHBP        =   (HSW + HBP - 1);
    _lcd.ltdc.Init.AccumulatedVBP        =   (VSW + VBP - 1);
    _lcd.ltdc.Init.AccumulatedActiveW    =   (HSW + HBP + LCD_WIDTH - 1);
    _lcd.ltdc.Init.AccumulatedActiveH    =   (VSW + VBP + LCD_HEIGHT - 1);
    _lcd.ltdc.Init.TotalWidth            =   (HSW + HBP + LCD_WIDTH + HFP - 1);
    _lcd.ltdc.Init.TotalHeigh            =   (VSW + VBP + LCD_HEIGHT + VFP - 1);
    
    _lcd.ltdc.LayerCfg->ImageWidth    = LCD_WIDTH;
    _lcd.ltdc.LayerCfg->ImageHeight   = LCD_HEIGHT; 
    _lcd.ltdc.Init.Backcolor.Blue     = 0x00;
    _lcd.ltdc.Init.Backcolor.Green    = 0x00;
    _lcd.ltdc.Init.Backcolor.Red      = 0x00;

    _lcd.ltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    _lcd.ltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    _lcd.ltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    _lcd.ltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

    HAL_LTDC_Init(&(_lcd.ltdc));
    
    return RT_EOK; 
}

void rgb_layer_init(uint16_t index, uint32_t framebuffer)
{
    LTDC_LayerCfgTypeDef layer_cfg;

    layer_cfg.WindowX0        = 0;
    layer_cfg.WindowX1        = LCD_WIDTH; 
    layer_cfg.WindowY0        = 0;
    layer_cfg.WindowY1        = LCD_HEIGHT;
    // layer_cfg.PixelFormat     = LTDC_PIXEL_FORMAT_ARGB8888;
    // layer_cfg.PixelFormat     = LTDC_PIXEL_FORMAT_RGB888;
    layer_cfg.PixelFormat     = LTDC_PIXEL_FORMAT_RGB565;
    layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
	// layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
    // layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
    layer_cfg.Alpha           = 255;
    layer_cfg.Alpha0          = 0;
    layer_cfg.ImageWidth      = LCD_WIDTH;
    layer_cfg.ImageHeight     = LCD_HEIGHT;
    layer_cfg.Backcolor.Blue  = 0;
    layer_cfg.Backcolor.Green = 0;
    layer_cfg.Backcolor.Red   = 0;
    layer_cfg.FBStartAdress   = framebuffer;

    HAL_LTDC_ConfigLayer(&(_lcd.ltdc), &layer_cfg, index);
}

static void rgb_display_on(void)
{
    rt_pin_mode(LCD_DISP_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LCD_DISP_PIN,PIN_HIGH);
    rt_pin_mode(LCD_BL_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LCD_BL_PIN,PIN_HIGH);
}

static void rgb_display_off(void)
{
    rt_pin_mode(LCD_DISP_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LCD_DISP_PIN,PIN_LOW);
    rt_pin_mode(LCD_BL_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LCD_BL_PIN,PIN_LOW);
}

void LTDC_IRQHandler(void)
{
    rt_interrupt_enter();
    HAL_LTDC_IRQHandler(&(_lcd.ltdc)); 
    rt_interrupt_leave();
}

/* ���Դ���: ---------------------------------------- */ 
static void lcd_fill_buffer(void *addr, uint32_t x_size, uint32_t y_size, uint32_t offset, uint32_t color)
{
    _lcd.dma2d.Instance = DMA2D;
    
    _lcd.dma2d.Init.Mode         = DMA2D_R2M;
    // _lcd.dma2d.Init.ColorMode    = DMA2D_ARGB8888;
    _lcd.dma2d.Init.ColorMode    = DMA2D_RGB565;
    _lcd.dma2d.Init.OutputOffset = offset;
    
    if (HAL_DMA2D_Init(&_lcd.dma2d) == HAL_OK)
    {
        if (HAL_DMA2D_ConfigLayer(&_lcd.dma2d, active_layer) == HAL_OK)
        {
            if (HAL_DMA2D_Start(&_lcd.dma2d, color, (uint32_t)addr, x_size, y_size) == HAL_OK)
            {
                HAL_DMA2D_PollForTransfer(&_lcd.dma2d, 10);
            }
        }
    }
}

// static void lcd_fill_buffer_with_color(void *addr, uint32_t x_size, uint32_t y_size, uint32_t offset, uint32_t color)
// {
    
//     _lcd.dma2d.Instance = DMA2D;
    
//     _lcd.dma2d.Init.Mode         = DMA2D_M2M;
//     // _lcd.dma2d.Init.ColorMode    = DMA2D_ARGB8888;
//     _lcd.dma2d.Init.ColorMode    = DMA2D_RGB565;
//     _lcd.dma2d.Init.OutputOffset = offset;
    
//     if (HAL_DMA2D_Init(&_lcd.dma2d) == HAL_OK)
//     {
//         if (HAL_DMA2D_ConfigLayer(&_lcd.dma2d, active_layer) == HAL_OK)
//         {
//             if (HAL_DMA2D_Start(&_lcd.dma2d, color, (uint32_t)addr, x_size, y_size) == HAL_OK)
//             {
//                 HAL_DMA2D_PollForTransfer(&_lcd.dma2d, 10);
//             }
//             else
//             {
//                 rt_kprintf("transfer error!\n");
//             }
//         }
//     }
// } 

void lcd_clear(uint32_t color)
{
    /* Clear the LCD */
    lcd_fill_buffer((uint32_t *)(_lcd.ltdc.LayerCfg[active_layer].FBStartAdress), LCD_WIDTH, LCD_HEIGHT, 0, color);
}

#ifdef FINSH_USING_MSH
void msh_lcd_clear(uint8_t argc, char **argv)
{
    if (argc < 2)
    {
        rt_kprintf("please enter an digit(1-4) to chose color to clear the screen!\n");
    }
    else
    {
        switch (argv[1][0])
        {
            case '1':lcd_clear(0xFF0000FF);break;
            case '2':lcd_clear(0xFF00FF00);break;
            case '3':lcd_clear(0xFFFF0000);break;
            case '4':lcd_clear(0xFF00FFFF);break;
            default: 
                break;
        }
    }
}
MSH_CMD_EXPORT(msh_lcd_clear, lcd clear)
#endif



// #ifdef FINSH_USING_MSH
// void msh_lcd_fill_buffer(uint8_t argc, char **argv)
// {
//     if (argc < 2)
//     {
//         rt_kprintf("please enter an digit(1-4) to chose color to clear the screen!\n");
//     }
//     else
//     {
//         switch (argv[1][0])
//         {
//             case '1':
//                 // if (!image1)
//                 //     rt_kprintf("image is bad!\n");
//                 lcd_fill_buffer_with_color((uint32_t *)(_lcd.ltdc.LayerCfg[active_layer].FBStartAdress),272,272,0,(uint32_t)image1);                
//                 rt_kprintf("fill with image..\n");
//                 break;
//             default: 
//                 break;
//         }
//     }
// }
// MSH_CMD_EXPORT(msh_lcd_fill_buffer, msh lcd fill buffer)
// #endif

void lcd_fill_rect(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height)
{
  uint32_t Xaddress = (_lcd.ltdc.LayerCfg[active_layer].FBStartAdress) + 4 * (LCD_WIDTH * x_pos + y_pos);
  lcd_fill_buffer((uint32_t *)Xaddress, width, height, (LCD_WIDTH - width), 0xFF00FF00);
}

static rt_err_t stm32_lcd_init(struct rt_device *device)
{
	// rgb_init(); 
	// memset(&_lcd, 0x0, sizeof(_lcd));

    _lcd.info.pixel_format  = RTGRAPHIC_PIXEL_FORMAT_RGB565;
    // _lcd.info.pixel_format      = RTGRAPHIC_PIXEL_FORMAT_RGB888;
    // _lcd.info.pixel_format  = RTGRAPHIC_PIXEL_FORMAT_ARGB888;
    _lcd.info.bits_per_pixel    = 16;
    _lcd.info.width             = _lcd.width;
    _lcd.info.height            = _lcd.height;

	_lcd.width  = LCD_WIDTH;
    _lcd.height = LCD_HEIGHT;
    _lcd.fb = rt_malloc_align(_lcd.width * _lcd.height * _lcd.info.bits_per_pixel / 8, 32);
	// _lcd.fb = rt_malloc(_lcd.width * _lcd.height * _lcd.info.bits_per_pixel / 8);
	if (!_lcd.fb)
	{
		rt_kprintf("frame_buffer malloc error!\n");
		return RT_ERROR;
	}

	memset(_lcd.fb, 0xff, (_lcd.width * _lcd.height * _lcd.info.bits_per_pixel / 8));

	rgb_layer_init(active_layer, (uint32_t)_lcd.fb); 

    _lcd.info.framebuffer       = _lcd.fb;

	rgb_display_on();
    struct stm32_lcd *lcd = RGBLCD_DEVICE(device);

    lcd = lcd; /* nothing, right now */
    return RT_EOK;
}

static rt_err_t stm32_lcd_on(rt_device_t dev, rt_uint16_t oflag)
{
    rgb_display_on();
	
	return RT_EOK;
}

static rt_err_t stm32_lcd_off(rt_device_t dev)
{
    rgb_display_off();
	
	return RT_EOK;
}

static rt_err_t stm32_lcd_control(struct rt_device *device, int cmd, void *args)
{
	struct stm32_lcd *lcd = RGBLCD_DEVICE(device);

    switch (cmd)
    {
    case RTGRAPHIC_CTRL_RECT_UPDATE:
        {
            struct rt_device_rect_info *info = (struct rt_device_rect_info*)args;

            info = info; /* nothing, right now */
        }
        break;

    case RTGRAPHIC_CTRL_GET_INFO:
        {
            struct rt_device_graphic_info* info = (struct rt_device_graphic_info*)args;

            RT_ASSERT(info != RT_NULL);
            
            info->pixel_format  = lcd->info.pixel_format;
            info->bits_per_pixel= lcd->info.bits_per_pixel;
            // info->pixel_format  = RTGRAPHIC_PIXEL_FORMAT_ARGB888;
            // info->bits_per_pixel= 32;
            info->width         = lcd->width;
            info->height        = lcd->height;
            info->framebuffer   = lcd->fb;
            // memcpy(info, &lcd->info, sizeof(lcd->info));
        }
        break;

	case RTGRAPHIC_CTRL_POWERON: 
		rgb_display_on(); 
		break;

	case RTGRAPHIC_CTRL_POWEROFF: 
		rgb_display_off(); 
		break;
    }
		
    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops rgblcd_ops = 
{
    stm32_lcd_init,
    stm32_lcd_on,
    stm32_lcd_off,
    // RT_NULL,
    RT_NULL,
    RT_NULL,
    stm32_lcd_control
};
#endif

int rt_hw_lcd_init(void)
{
    rt_err_t ret; 
	struct rt_device *device = &_lcd.device;

	memset(&_lcd, 0x0, sizeof(_lcd));

	rgb_init(); 

	device->type    = RT_Device_Class_Graphic;
#ifdef RT_USING_DEVICE_OPS
    device->ops     = &rgblcd_ops;
#else
    device->init    = stm32_lcd_init;
    device->open    = stm32_lcd_on;
    device->close   = stm32_lcd_off;
    device->control = stm32_lcd_control;
#endif

    ret = rt_device_register(device, "lcd", RT_DEVICE_FLAG_RDWR);

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_lcd_init); 

#endif /* BSP_USING_LCD */
