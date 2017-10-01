#include <stdio.h>  
#include <stdlib.h>  
#include <fcntl.h>  
#include <string.h> 
#include <unistd.h> 
#include <linux/fb.h>  
#include <sys/mman.h>  
#include <sys/ioctl.h>  
#include <arpa/inet.h>  
  
#define DEV_NAME "/dev/fb0"
int main ( int argc, char *argv[] )  
{  
    int fd = 0;  
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    struct fb_bitfield red;
    struct fb_bitfield green;
    struct fb_bitfield blue;
    char *fbp = 0;
    char * pDstBuf;

    /*打开设备文件*/  
    fd = open(DEV_NAME, O_RDWR);  
    if (!fd)  
    {
        perror("open fb device ERR:"); 
        return 0; 
    }
  
    /*取得屏幕相关参数*/

  
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo))  
    {
        perror("reading variable information ERR:\n"); 
        close(fd);  
        return 0; 
    }
    /* != 0 Gray levels instead of colors */ 
    printf("vinfo.grayscale=%d\n", vinfo.grayscale); 
    /* != 0 Non standard pixel format */ 
    printf("vinfo.nonstd=%d\n", vinfo.nonstd); 
    /* see FB_ACTIVATE_* */
    printf("vinfo.activate=%d\n", vinfo.activate);
    /* width of picture in mm, height of picture in mm */   
    printf("vinfo.height=%d width=%d\n", vinfo.height, vinfo.width);
    /* acceleration flags (hints) */ 
    printf("vinfo.accel_flags=%d\n", vinfo.accel_flags);
    /* pixel clock in ps (pico seconds) */ 
    printf("vinfo.pixclock=%d\n", vinfo.pixclock); 
    /* time from sync to picture */ 
    printf("vinfo.left_margin=%d\n", vinfo.left_margin); 
    /* time from picture to sync */ 
    printf("vinfo.right_margin=%d\n", vinfo.right_margin); 
    /* time from sync to picture */ 
    printf("vinfo.upper_margin=%d\n", vinfo.upper_margin); 
    printf("vinfo.lower_margin=%d\n", vinfo.lower_margin);
    /* length of horizontal sync */  
    printf("vinfo.hsync_len=%d\n", vinfo.hsync_len); 
    /* length of vertical sync */ 
    printf("vinfo.vsync_len=%d\n", vinfo.vsync_len);
    /* see FB_SYNC_* */  
    printf("vinfo.sync=%d\n", vinfo.sync); 
    /* see FB_VMODE_*/ 
    printf("vinfo.vmode=%d\n", vinfo.vmode); 

    printf("vinfo.red   offset=%d   length=%d   msb_right=%d\n",  vinfo.red.offset,  vinfo.red.length,  vinfo.red.msb_right);
    printf("vinfo.green   offset=%d   length=%d   msb_right=%d\n",  vinfo.green.offset,  vinfo.green.length,  vinfo.green.msb_right);
    printf("vinfo.blue   offset=%d   length=%d   msb_right=%d\n",  vinfo.blue.offset,  vinfo.blue.length,  vinfo.blue.msb_right);
    printf("vinfo.transp   offset=%d   length=%d   msb_right=%d\n",  vinfo.transp.offset,  vinfo.transp.length,  vinfo.transp.msb_right);
    printf("vinfo.bits_per_pixel=%d\n", vinfo.bits_per_pixel);
    printf("vinfo.xres_virtual=%d vinfo.yres_virtual=%d\n", vinfo.xres_virtual, vinfo.yres_virtual);
    printf("vinfo.xoffset=%d  vinfo.yoffset=%d\n", vinfo.xoffset, vinfo.yoffset);
    printf("vinfo.xresXvinfo.yres=%dX%d\n", vinfo.xres, vinfo.yres);  
  
    /* finfo 它描述显示卡的属性，并且系统运行时不能被修改*/
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo))  
    {
        perror("reading fixed information ERR:"); 
        close(fd); 
        return 0; 
    }
    /* identification string eg "TT Builtin" */ 
    printf("finfo.id=%s \n", finfo.id); 
    /* Start of frame buffer mem */ 
    printf("finfo.smem_start=%ld \n", finfo.smem_start); 
    /* (physical address) */ 
    /* Length of frame buffer mem */ 
    printf("finfo.smem_len=%d \n", finfo.smem_len); 
    /* see FB_TYPE_* */  
    printf("finfo.type=%d \n", finfo.type); 
    /* Interleave for interleaved Planes */ 
    printf("finfo.type_aux=%d \n", finfo.type_aux); 
    /* see FB_VISUAL_* */ 
    printf("finfo.visual=%d \n", finfo.visual);   
    /* zero if no hardware panning */ 
    printf("finfo.xpanstep=%d \n", finfo.xpanstep); 
    /* zero if no hardware panning */ 
    printf("finfo.ypanstep=%d \n", finfo.ypanstep); 
    /* zero if no hardware ywrap */ 
    printf("finfo.ywrapstep=%d \n", finfo.ywrapstep); 
    /* length of a line in bytes */ 
    printf("finfo.line_length=%d \n", finfo.line_length); 
    /* Start of Memory Mapped I/O */ 
    printf("finfo.mmio_start=%ld \n", finfo.mmio_start); 
    /* (physical address) */ 
    /* Length of Memory Mapped I/O */ 
    printf("finfo.mmio_len=%d \n", finfo.mmio_len); 
    /* Type of acceleration available */ 
    printf("finfo.accel=%d \n", finfo.accel); 

    /*映射屏幕缓冲区到用户地址空间*/  
    fbp = (char *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  
    if (fbp == MAP_FAILED)  
    {  
        perror("map framebuffer device to memory ERR:\n");  
        return 0;
    }  

    int cr = 0,cg = 0,cb = 0;
    while(1)
    {
        for( int i = 0; i < 768; i++ )
        {
            for( int j = 0; j < 1366; j++ )
            {
                fbp[finfo.line_length*i+j*4] = cb;
                fbp[finfo.line_length*i+j*4+1] = cg;
                fbp[finfo.line_length*i+j*4+2] = cr;
                fbp[finfo.line_length*i+j*4+3] = 0;
            }
        }
        cr += 10;
        if( cr > 255  )
        {
            cr += 50;
            cg++;
            if( cg > 255  )
            {
                cg += 50;
                cb++;
                if( cb > 255  )
                {
                    cb = 0;
                }
            }
        }
        usleep(200000);

    }


    /*释放缓冲区，关闭设备*/
    munmap(fbp, screensize);  
    close(fd);  
    return 0;  
}  
