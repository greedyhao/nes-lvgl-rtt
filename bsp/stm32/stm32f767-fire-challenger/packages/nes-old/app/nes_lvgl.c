#include <rtthread.h>
#include <rtdevice.h>
#include <lvgl.h>
#include "nesplay.h"

lv_obj_t * nes_fb;

static rt_device_t device; 
static struct rt_device_graphic_info info; 
//static struct rt_messagequeue *input_mq; 

lv_img_dsc_t nes_fb_img_src= {
  .header.always_zero = 0,
  // .header.w = LV_HOR_RES,         //2D buffer
  // .header.h = LV_VER_RES,          //2D buffer
  .header.w = 800,         //2D buffer
  .header.h = 480,          //2D buffer
  .data_size = LV_HOR_RES * LV_VER_RES * LV_COLOR_SIZE / 8,
  .header.cf = LV_IMG_CF_TRUE_COLOR,
//   .data = nes_frame_buffer,
};

rt_err_t nes2rtt_init(void)
{
    rt_thread_t play_thread;

    device = rt_device_find("lcd");
    RT_ASSERT(device != RT_NULL);
    if(rt_device_open(device, RT_DEVICE_OFLAG_RDWR) == RT_EOK)
    {
        rt_device_control(device, RTGRAPHIC_CTRL_GET_INFO, &info);
    }
    
    // nes_fb_img_src.header.w = info.width;
    // nes_fb_img_src.header.h = info.height;
    // nes_fb_img_src.data_size = info.width * info.height * LV_COLOR_SIZE / 8;
    nes_fb_img_src.data = info.framebuffer;

    //Create an image with the NES frame buffer image
    nes_fb = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(nes_fb, &nes_fb_img_src);

    // play_thread = rt_thread_create("play",
		// 						nes_play, RT_NULL,
		// 						1024*2, 20, 4);

    // if (play_thread != RT_NULL)
		// rt_thread_startup(play_thread);
    nes_play(RT_NULL);
    
    return RT_EOK;
}
