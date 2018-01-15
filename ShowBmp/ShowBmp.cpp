#include <stdio.h>  
#include <stdlib.h>  
#include <fcntl.h>  
#include <string.h> 
#include <unistd.h> 
#include <linux/fb.h>  
#include <sys/mman.h>  
#include <sys/ioctl.h>  
#include <arpa/inet.h>
#include <sys/time.h>  
#include "bmp.h"

#define DEV_NAME "/dev/fb0"


typedef struct FrameBufferInfo
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    char *fbp;
    int fd;  
}FrameBufferInfo;


/*
 * 函数名称 : ClearFrameBuff
 * 函数介绍 : 使用指定颜色刷新FrameBuffer
 * 参数介绍 : fbInfo:FrameBuffer相关信息， x,y,w,h：清空的矩形， r,g,b:使用的颜色,
 *           bps:framebuffer的bps
 * 返回值   : -1:失败，  0：成功
 */
int ClearFrameBuff(FrameBufferInfo fbInfo, int x, int y, int w, int h, int r, int g, int b, int bps)
{
    unsigned int rgb = 0;
    int drawW = 0;
    int drawH = 0;
    int fbw = 0;
    int fbh = fbInfo.vinfo.yres;
    switch( bps )
    {
        case 32:
            fbw = fbInfo.finfo.line_length/4;
        break;
        case 16:
            fbw = fbInfo.finfo.line_length/2;
        break;
    }
    if ( x + w > fbw )
    {
        drawW = fbw-x;
    }
    else
    {
        drawW = w;
    }

    if ( y + h > fbh )
    {
        drawH = fbh-y;
    }
    else
    {
        drawH = h;
    }

    switch( bps )
    {
        case 32:
            rgb = (r<<24) | (g<<16) | (b<<8);
        break;
        case 16:
            rgb = (((unsigned(r) << 8) & 0xF800) | 
                    ((unsigned(g) << 3) & 0x7E0) | 
                    ((unsigned(b) >> 3)));
        break;
    }

    for( int i = 0; i < drawH; i++ )
    {
        for( int j = 0; j < drawW; j++ )
        {
            switch( bps )
            {
                case 32:
                    fbInfo.fbp[fbw*4*(i+y)+(j+x)*4] = (rgb>>24)&0xFF;
                    fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+1] = (rgb>>16)&0xFF;
                    fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+2] = (rgb>>8)&0xFF;
                    fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+3] = (rgb)&0xFF;
                break;
                case 16:
                    fbInfo.fbp[fbw*2*(i+y)+(j+x)*2] = (rgb>>8)&0xFF;
                    fbInfo.fbp[fbw*2*(i+y)+(j+x)*2+1] = (rgb)&0xFF;
                break;
            }
        }
    }
    return 0;
}


/*
 * 函数名称 : ShowBmp
 * 函数介绍 : 将指定的图像显示在指定的位置
 * 参数介绍 : fbInfo:FrameBuffer信息， x,y:显示的位置， bmpBuff:图片数据， bw,bh:图片宽高,  
 *           bps:framebuffer的bps
 * 返回值   : -1:失败，  0：成功,
 */
int ShowBmp( FrameBufferInfo fbInfo, int x, int y, char *bmpBuff, int bw, int bh, int bps)
{
    unsigned int rgb = 0;
    int drawW = 0;
    int drawH = 0;
    int fbw = 0;
    int fbh = fbInfo.vinfo.yres;

    switch( bps )
    {
        case 32:
            fbw = fbInfo.finfo.line_length/4;
        break;
        case 16:
            fbw = fbInfo.finfo.line_length/2;
        break;
    }

    if ( x + bw > fbw )
    {
        drawW = fbw-x;
    }
    else
    {
        drawW = bw;
    }

    if ( y + bh > fbh )
    {
        drawH = fbh-y;
    }
    else
    {
        drawH = bh;
    }

    for( int i = 0; i < drawH; i++ )
    {
        for( int j = 0; j < drawW; j++ )
        {
                switch( bps )
                {
                    case 32:
                        fbInfo.fbp[fbw*4*(i+y)+(j+x)*4] = bmpBuff[i*bw*3+j*3+0];
                        fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+1] = bmpBuff[i*bw*3+j*3+1];
                        fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+2] = bmpBuff[i*bw*3+j*3+2];
                        fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+3] = 255;
                    break;
                    case 16:
                        rgb = (((bmpBuff[i*bw*3+j*3+2] << 8) & 0xF800) | 
                                ((bmpBuff[i*bw*3+j*3+1] << 3) & 0x7E0) | 
                                ((bmpBuff[i*bw*3+j*3+0] >> 3)));
                        fbInfo.fbp[fbw*2*(i+y)+(j+x)*2] = (rgb)&0xFF;
                        fbInfo.fbp[fbw*2*(i+y)+(j+x)*2+1] = (rgb>>8)&0xFF;
                    break;
                }

        }
    } 
}


int main ( int argc, char *argv[] )  
{  
    FrameBufferInfo fbInfo;
    int topx = 0;
    int topy = 0;
    int frameCount = 0;
    int bps = 0;
    struct timeval ts, te;

    int r = 0;
    int g = 0;
    int b = 0;

    if(argc == 4)
    {
        r = atoi(argv[1]);
        g = atoi(argv[2]);
        b = atoi(argv[3]);
        printf("clear r=%d, g=%d, b=%d \n", r, g, b);
    }

    /*打开设备文件*/  
    fbInfo.fd = open(DEV_NAME, O_RDWR);  
    if (!fbInfo.fd)  
    {
        perror("open fb device ERR:"); 
        return 0; 
    }
  
    /*取得屏幕相关参数*/

  
    if (ioctl(fbInfo.fd, FBIOGET_VSCREENINFO, &fbInfo.vinfo))  
    {
        perror("reading variable information ERR:\n"); 
        close(fbInfo.fd);  
        return 0; 
    }
    /* != 0 Gray levels instead of colors */ 
    printf("vinfo.grayscale=%d\n", fbInfo.vinfo.grayscale); 
    /* != 0 Non standard pixel format */ 
    printf("vinfo.nonstd=%d\n", fbInfo.vinfo.nonstd); 
    /* see FB_ACTIVATE_* */
    printf("vinfo.activate=%d\n", fbInfo.vinfo.activate);
    /* width of picture in mm, height of picture in mm */   
    printf("vinfo.height=%d width=%d\n", fbInfo.vinfo.height, fbInfo.vinfo.width);
    /* acceleration flags (hints) */ 
    printf("vinfo.accel_flags=%d\n", fbInfo.vinfo.accel_flags);
    /* pixel clock in ps (pico seconds) */ 
    printf("vinfo.pixclock=%d\n", fbInfo.vinfo.pixclock); 
    /* time from sync to picture */ 
    printf("vinfo.left_margin=%d\n", fbInfo.vinfo.left_margin); 
    /* time from picture to sync */ 
    printf("vinfo.right_margin=%d\n", fbInfo.vinfo.right_margin); 
    /* time from sync to picture */ 
    printf("vinfo.upper_margin=%d\n", fbInfo.vinfo.upper_margin); 
    printf("vinfo.lower_margin=%d\n", fbInfo.vinfo.lower_margin);
    /* length of horizontal sync */  
    printf("vinfo.hsync_len=%d\n", fbInfo.vinfo.hsync_len); 
    /* length of vertical sync */ 
    printf("vinfo.vsync_len=%d\n", fbInfo.vinfo.vsync_len);
    /* see FB_SYNC_* */  
    printf("vinfo.sync=%d\n", fbInfo.vinfo.sync); 
    /* see FB_VMODE_*/ 
    printf("vinfo.vmode=%d\n", fbInfo.vinfo.vmode); 

    printf("vinfo.red   offset=%d   length=%d   msb_right=%d\n",  fbInfo.vinfo.red.offset,  fbInfo.vinfo.red.length,  fbInfo.vinfo.red.msb_right);
    printf("vinfo.green   offset=%d   length=%d   msb_right=%d\n",  fbInfo.vinfo.green.offset,  fbInfo.vinfo.green.length,  fbInfo.vinfo.green.msb_right);
    printf("vinfo.blue   offset=%d   length=%d   msb_right=%d\n",  fbInfo.vinfo.blue.offset,  fbInfo.vinfo.blue.length,  fbInfo.vinfo.blue.msb_right);
    printf("vinfo.transp   offset=%d   length=%d   msb_right=%d\n",  fbInfo.vinfo.transp.offset,  fbInfo.vinfo.transp.length,  fbInfo.vinfo.transp.msb_right);
    printf("vinfo.bits_per_pixel=%d\n", fbInfo.vinfo.bits_per_pixel);
    bps = fbInfo.vinfo.bits_per_pixel;
    printf("vinfo.xres_virtual=%d vinfo.yres_virtual=%d\n", fbInfo.vinfo.xres_virtual, fbInfo.vinfo.yres_virtual);
    printf("vinfo.xoffset=%d  vinfo.yoffset=%d\n", fbInfo.vinfo.xoffset, fbInfo.vinfo.yoffset);
    printf("vinfo.xresXvinfo.yres=%dX%d\n", fbInfo.vinfo.xres, fbInfo.vinfo.yres);  
  
    /* finfo 它描述显示卡的属性，并且系统运行时不能被修改*/
    if (ioctl(fbInfo.fd, FBIOGET_FSCREENINFO, &fbInfo.finfo))  
    {
        perror("reading fixed information ERR:"); 
        close(fbInfo.fd); 
        return 0; 
    }
    /* identification string eg "TT Builtin" */ 
    printf("finfo.id=%s \n", fbInfo.finfo.id); 
    /* Start of frame buffer mem */ 
    printf("finfo.smem_start=%ld \n", fbInfo.finfo.smem_start); 
    /* (physical address) */ 
    /* Length of frame buffer mem */ 
    printf("finfo.smem_len=%d \n", fbInfo.finfo.smem_len); 
    /* see FB_TYPE_* */  
    printf("finfo.type=%d \n", fbInfo.finfo.type); 
    /* Interleave for interleaved Planes */ 
    printf("finfo.type_aux=%d \n", fbInfo.finfo.type_aux); 
    /* see FB_VISUAL_* */ 
    printf("finfo.visual=%d \n", fbInfo.finfo.visual);   
    /* zero if no hardware panning */ 
    printf("finfo.xpanstep=%d \n", fbInfo.finfo.xpanstep); 
    /* zero if no hardware panning */ 
    printf("finfo.ypanstep=%d \n", fbInfo.finfo.ypanstep); 
    /* zero if no hardware ywrap */ 
    printf("finfo.ywrapstep=%d \n", fbInfo.finfo.ywrapstep); 
    /* length of a line in bytes */ 
    printf("finfo.line_length=%d \n", fbInfo.finfo.line_length); 
    /* Start of Memory Mapped I/O */ 
    printf("finfo.mmio_start=%ld \n", fbInfo.finfo.mmio_start); 
    /* (physical address) */ 
    /* Length of Memory Mapped I/O */ 
    printf("finfo.mmio_len=%d \n", fbInfo.finfo.mmio_len); 
    /* Type of acceleration available */ 
    printf("finfo.accel=%d \n", fbInfo.finfo.accel); 

    /*映射屏幕缓冲区到用户地址空间*/  
    fbInfo.fbp = (char *)mmap(0, fbInfo.finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbInfo.fd, 0);  
    if (fbInfo.fbp == MAP_FAILED)  
    {  
        perror("map framebuffer device to memory ERR:\n");  
        return 0;
    }  

    BmpInfo *bmpInfo = new BmpInfo((char*)"./assert/image.bmp");
    /*BMP的上下是翻转的，这里将图像数据翻转过来*/
    bmpInfo->MirrorByV();
    printf("fileName=%s  width=%d height=%d length=%d\n", bmpInfo->fileName, bmpInfo->imagewidth, bmpInfo->imageheight, bmpInfo->pixellength);
    while(1)
    {
        ClearFrameBuff(fbInfo, 0, 0, fbInfo.vinfo.xres, fbInfo.vinfo.yres, r, g, b, bps);
        ShowBmp(fbInfo, topx, topy,\
            bmpInfo->pixeldata, bmpInfo->imagewidth, bmpInfo->imageheight, bps);
        usleep(33000);

        topx+=3;
        if( topx > fbInfo.vinfo.xres )
        {
            topx = 0;
        }
        topy+=5;

        if( topy > fbInfo.vinfo.yres )
        {
            topy = 0;
        }


        if( frameCount % 20 == 0 )
        {
            gettimeofday(&te, 0);
            float tus = (te.tv_sec - ts.tv_sec) * 1000 + (te.tv_usec - ts.tv_usec) / 1000;
            float fps = 20000.0f / tus;
            printf("fps = %.3f\n", fps);
            gettimeofday(&ts, 0);
        }
        frameCount++;
    }


    /*释放缓冲区，关闭设备*/
    munmap(fbInfo.fbp, fbInfo.finfo.smem_len);  
    close(fbInfo.fd);  
    return 0;  
}  
