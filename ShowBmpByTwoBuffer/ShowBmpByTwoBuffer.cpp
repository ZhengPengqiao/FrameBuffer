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
#include <pthread.h>
#include "bmp.h"

#define DEV_NAME "/dev/fb0"

char *fbBuffer;
char *pDistBuffer;  //绘制的指针
char *pDrawBuffer;  //供给display绘制到显示缓存的数据

typedef struct FrameBufferInfo
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    char *fbp;
    int fd;  
}FrameBufferInfo;


/*
 * 函数名称 : FBClearColor
 * 函数介绍 : 使用指定颜色刷新FrameBuffer
 * 参数介绍 : fbInfo:FrameBuffer相关信息， x,y,w,h：清空的矩形， r,g,b:使用的颜色
 * 返回值   : -1:失败，  0：成功
 */
int FBClearColor(FrameBufferInfo fbInfo, int x, int y, int w, int h, int r, int g, int b)
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
 * 函数名称 : FBShowBmp
 * 函数介绍 : 将指定的图像显示在指定的位置
 * 参数介绍 : fbInfo:FrameBuffer信息， x,y:显示的位置， bmpBuff:图片数据， bw,bh:图片宽高
 * 返回值   : -1:失败，  0：成功, 
 */
int FBShowBmp( FrameBufferInfo fbInfo, int x, int y, char *bmpBuff, int bw, int bh)
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


/*
 * 函数名称 : BufferShowBmp
 * 函数介绍 : 将指定的图像显示在指定的位置
 * 参数介绍 : fbBuffer:fbBuffer缓存，fbw,fbh:fb宽高， x,y:显示的位置， bmpBuff:图片数据， bw,bh:图片宽高
 * 返回值   : -1:失败，  0：成功, 
 */
int BufferShowBmp( char* fbBuffer, int fbw, int fbh, int x, int y, char *bmpBuff, int bw, int bh)
{
    int drawW = 0;
    int drawH = 0;
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
            fbBuffer[fbw*4*(i+y)+(j+x)*4] = bmpBuff[i*bw*3+j*3+0];
            fbBuffer[fbw*4*(i+y)+(j+x)*4+1] = bmpBuff[i*bw*3+j*3+1];
            fbBuffer[fbw*4*(i+y)+(j+x)*4+2] = bmpBuff[i*bw*3+j*3+2];
            fbBuffer[fbw*4*(i+y)+(j+x)*4+3] = 255;
        }
    } 
}


/*
 * 函数名称 : BufferClearColor
 * 函数介绍 : 使用指定颜色刷新FrameBuffer
 * 参数介绍 : fbInfo:FrameBuffer相关信息， x,y,w,h：清空的矩形， r,g,b:使用的颜色
 * 返回值   : -1:失败，  0：成功
 */
int BufferClearColor(char* fbBuffer, int fbw, int fbh, int x, int y, int w, int h, int r, int g, int b)
{
    int drawW = 0;
    int drawH = 0;
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
            fbBuffer[fbw*4*(i+y)+(j+x)*4] = b;
            fbBuffer[fbw*4*(i+y)+(j+x)*4+1] = g;
            fbBuffer[fbw*4*(i+y)+(j+x)*4+2] = r;
            fbBuffer[fbw*4*(i+y)+(j+x)*4+3] = 0;
        }
    }
    return 0;
}


void * displayFun(void *arg)
{
    static struct timeval ts, te;
    static int frameCount = 0;

    FrameBufferInfo *fbInfo=(FrameBufferInfo *)arg;
    while(1)
    {
        if(pDrawBuffer != NULL)
        {
            memcpy(fbInfo->fbp, pDrawBuffer, fbInfo->finfo.smem_len);
            if( frameCount % 20 == 0 )
            {
                gettimeofday(&te, 0);
                float tus = (te.tv_sec - ts.tv_sec) * 1000 + (te.tv_usec - ts.tv_usec) / 1000;
                float fps = 20000.0f / tus;
                printf("fps = %.3f\n", fps);
                gettimeofday(&ts, 0);
            }
        }
        usleep(33000);
        frameCount++;
    }
    return NULL;
}



int main ( int argc, char *argv[] )  
{  
    FrameBufferInfo fbInfo;
    int BufferFlag = 0;
    int topx = 0;
    int topy = 0;
    pthread_t thr1;

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
    /* Length of frame buffer mem */ 
    printf("finfo.smem_len=%d \n", fbInfo.finfo.smem_len); 
    /* see FB_TYPE_* */  
    printf("finfo.line_length=%d \n", fbInfo.finfo.line_length); 
    /* Start of Memory Mapped I/O */ 

    /*映射屏幕缓冲区到用户地址空间*/  
    fbInfo.fbp = (char *)mmap(0, fbInfo.finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbInfo.fd, 0);  
    if (fbInfo.fbp == MAP_FAILED)  
    {  
        perror("map framebuffer device to memory ERR:\n");  
        return 0;
    }  

    /* 分配两个framebuffer的大小 */
    fbBuffer = (char *)malloc(fbInfo.finfo.smem_len*2);
    if(fbBuffer == NULL)
    {
        return -1;
    }

    pDistBuffer = fbBuffer;
    if(pthread_create(&thr1,NULL,displayFun,&fbInfo)!=0)
    {
        printf("create thread failed!\n");
        return -1;
    }

    BmpInfo *bmpInfo = new BmpInfo((char*)"./assert/image.bmp");
    /*BMP的上下是翻转的，这里将图像数据翻转过来*/
    bmpInfo->MirrorByV();
    printf("fileName=%s  width=%d height=%d length=%d\n", bmpInfo->fileName, bmpInfo->imagewidth, bmpInfo->imageheight, bmpInfo->pixellength);
    while(1)
    {
        // FBClearColor(fbInfo, 0, 0, fbInfo.vinfo.xres, fbInfo.vinfo.yres-40, 0, 0, 0);
        // FBShowBmp(fbInfo, topx, topy,\
        //     bmpInfo->pixeldata, bmpInfo->imagewidth, bmpInfo->imageheight);
             
        BufferClearColor(pDistBuffer, fbInfo.finfo.line_length/4, fbInfo.vinfo.yres,\
            0, 0, fbInfo.finfo.line_length/4, fbInfo.vinfo.yres, 0, 0, 0);
        BufferShowBmp(pDistBuffer, fbInfo.finfo.line_length/4, fbInfo.vinfo.yres, \
            topx, topy, bmpInfo->pixeldata, bmpInfo->imagewidth, bmpInfo->imageheight);
        pDrawBuffer = pDistBuffer;

        if( BufferFlag )
        {
            pDistBuffer = fbBuffer+fbInfo.finfo.smem_len;
            BufferFlag = ~BufferFlag;
        }
        else
        {
            pDistBuffer = fbBuffer;
            BufferFlag = ~BufferFlag;
        }
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



    }

    /*释放缓冲区，关闭设备*/
    munmap(fbInfo.fbp, fbInfo.finfo.smem_len);  
    close(fbInfo.fd);  
    return 0;  
}  
