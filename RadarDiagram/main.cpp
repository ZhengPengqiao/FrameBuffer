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
#include "radardiagram.h"

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
 * 参数介绍 : fbInfo:FrameBuffer相关信息， x,y,w,h：清空的矩形， r,g,b:使用的颜色
 * 返回值   : -1:失败，  0：成功
 */
int ClearFrameBuff(FrameBufferInfo fbInfo, int x, int y, int w, int h, int r, int g, int b)
{
    int drawW = 0;
    int drawH = 0;
    int fbw = fbInfo.finfo.line_length/4;
    int fbh = fbInfo.vinfo.yres;
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

    for( int i = 0; i < drawH; i++ )
    {
        for( int j = 0; j < drawW; j++ )
        {
            fbInfo.fbp[fbw*4*(i+y)+(j+x)*4] = b;
            fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+1] = g;
            fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+2] = r;
            fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+3] = 0;
        }
    }
    return 0;
}


/*
 * 函数名称 : ShowBmp
 * 函数介绍 : 将指定的图像显示在指定的位置
 * 参数介绍 : fbInfo:FrameBuffer信息， x,y:显示的位置， bmpBuff:图片数据， bw,bh:图片宽高
 * 返回值   : -1:失败，  0：成功, 
 */
int ShowBmp( FrameBufferInfo fbInfo, int x, int y, char *bmpBuff, int bw, int bh)
{
    int drawW = 0;
    int drawH = 0;
    int fbw = fbInfo.finfo.line_length/4;
    int fbh = fbInfo.vinfo.yres;
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
            fbInfo.fbp[fbw*4*(i+y)+(j+x)*4] = bmpBuff[i*bw*3+j*3+0];
            fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+1] = bmpBuff[i*bw*3+j*3+1];
            fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+2] = bmpBuff[i*bw*3+j*3+2];
            fbInfo.fbp[fbw*4*(i+y)+(j+x)*4+3] = 255;
        }
    } 
}


int main ( int argc, char *argv[] )  
{  
    FrameBufferInfo fbInfo;
    int topx = 0;
    int topy = 0;
    int frameCount = 0;
    struct timeval ts, te;

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
   
    printf("vinfo.red   offset=%d   length=%d   msb_right=%d\n",  fbInfo.vinfo.red.offset,  fbInfo.vinfo.red.length,  fbInfo.vinfo.red.msb_right);
    printf("vinfo.green   offset=%d   length=%d   msb_right=%d\n",  fbInfo.vinfo.green.offset,  fbInfo.vinfo.green.length,  fbInfo.vinfo.green.msb_right);
    printf("vinfo.blue   offset=%d   length=%d   msb_right=%d\n",  fbInfo.vinfo.blue.offset,  fbInfo.vinfo.blue.length,  fbInfo.vinfo.blue.msb_right);
    printf("vinfo.transp   offset=%d   length=%d   msb_right=%d\n",  fbInfo.vinfo.transp.offset,  fbInfo.vinfo.transp.length,  fbInfo.vinfo.transp.msb_right);
    printf("vinfo.bits_per_pixel=%d\n", fbInfo.vinfo.bits_per_pixel);
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
   
    /*映射屏幕缓冲区到用户地址空间*/  
    fbInfo.fbp = (char *)mmap(0, fbInfo.finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbInfo.fd, 0);  
    if (fbInfo.fbp == MAP_FAILED)  
    {
        perror("map framebuffer device to memory ERR:\n");  
        return 0;
    }

    RadarDiagrame *radar = new RadarDiagrame();
    radar->mallocPixelData(200,200);
    printf("radar  width=%d height=%d length=%d\n", radar->pixelwidth, radar->pixelheight, radar->pixellength);
    while(1)
    {
        ClearFrameBuff(fbInfo, 0, 0, fbInfo.vinfo.xres, fbInfo.vinfo.yres, 0, 0, 0);
        ShowBmp(fbInfo, topx, topy,\
            radar->pixeldata, radar->pixelwidth, radar->pixelheight);
        usleep(33000);
    }


    /*释放缓冲区，关闭设备*/
    munmap(fbInfo.fbp, fbInfo.finfo.smem_len);  
    close(fbInfo.fd);  
    return 0;  
}  
