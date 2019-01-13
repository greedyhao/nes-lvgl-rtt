/*
 * File      : drv_lcd.h 
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author         Notes
 * 2018-07-28     liu2guang      the first version
 */
 
#ifndef __DRV_LCD_H_ 
#define __DRV_LCD_H_ 

#include <rtthread.h> 
#include <rtdevice.h>
#include <board.h> 

int rt_hw_lcd_init(void);
void lcd_clear(uint32_t color); 
void lcd_fill_rect(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height); 

#endif
