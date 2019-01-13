#include <rtthread.h>

#ifdef RT_USING_DFS
#include <dfs_fs.h>

#include <littlevgl2rtt_demo.h>
#include <nes_lvgl.h>

int mnt_init(void)
{
    rt_thread_delay(RT_TICK_PER_SECOND);

    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("file system initialization done!\n");
        rt_lvgl_demo_init();
        // nes2rtt_init();
    }

    return 0;
}
INIT_ENV_EXPORT(mnt_init);
#endif

