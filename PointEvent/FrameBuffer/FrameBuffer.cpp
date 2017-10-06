#include "FrameBuffer.h"
#include <stdio.h>  
#include <stdlib.h>  
#include <fcntl.h>  
#include <string.h> 
#include <unistd.h> 
#include <sys/mman.h>  
#include <sys/ioctl.h>  
#include <arpa/inet.h>
#include <sys/time.h>  
#include <pthread.h>
#include "bmp.h"
bool FrameBuffer::showfps = true;

/*
 * 函数名称 : FrameBuffer
 * 函数介绍 : 构造函数
 */
FrameBuffer::FrameBuffer()
{
	pSwapBuffer 	= NULL;
	pnowBackBuffer = NULL;
	pnowDrawBuffer = NULL; 
	bufferCount = 2;   	//默认使用两个缓冲区
    nowDrawBuffer = 0;  //初始当前显示使用的缓冲区
    nowBackBuffer = 1;  //初始当前后台绘制的缓冲区
    width = 0;
    height = 0;
    frameRateTime = 33000; //刷新间隔33000us, 也就是30fps
    showfps = true;
}
/*
 * 函数名称 : ~FrameBuffer
 * 函数介绍 : 构造函数
 */
FrameBuffer::~FrameBuffer()
{
	if(pSwapBuffer != NULL)
	{
		free(pSwapBuffer);
	}

	if(pnowBackBuffer != NULL)
	{
		free(pnowBackBuffer);
	}

	if(pnowDrawBuffer != NULL)
	{
		free(pnowDrawBuffer);
	}
}


/*
 * 函数名称 : ClearColor
 * 函数介绍 : 使用指定颜色刷新FrameBuffer
 * 参数介绍 : x,y,w,h：清空的矩形， r,g,b:使用的颜色
 * 返回值   : -1:失败，  0：成功
 */
int FrameBuffer::ClearColor( int x, int y, int w, int h, int r, int g, int b)
{
    int drawW = 0;
    int drawH = 0;
    int fbw = finfo.line_length/4;
    int fbh = vinfo.yres;
    
    if ( pnowBackBuffer == NULL)
    {
        return -1;
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

    for( int i = 0; i < drawH; i++ )
    {
        for( int j = 0; j < drawW; j++ )
        {
            pnowBackBuffer[fbw*4*(i+y)+(j+x)*4] = b;
            pnowBackBuffer[fbw*4*(i+y)+(j+x)*4+1] = g;
            pnowBackBuffer[fbw*4*(i+y)+(j+x)*4+2] = r;
            pnowBackBuffer[fbw*4*(i+y)+(j+x)*4+3] = 0;
        }
    }
    return 0;
}


/*
 * 函数名称 : ShowBmp
 * 函数介绍 : 将指定的图像显示在指定的位置
 * 参数介绍 : x,y:显示的位置， bmpBuff:图片数据， bw,bh:图片宽高
 * 返回值   : -1:失败，  0：成功, 
 */
int FrameBuffer::ShowBmp( int x, int y, char *bmpBuff, int bw, int bh)
{
    int drawW = 0;
    int drawH = 0;
    int fbw = finfo.line_length/4;
    int fbh = vinfo.yres;

    if ( pnowBackBuffer == NULL)
    {
        return -1;
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
            pnowBackBuffer[fbw*4*(i+y)+(j+x)*4] = bmpBuff[i*bw*3+j*3+0];
            pnowBackBuffer[fbw*4*(i+y)+(j+x)*4+1] = bmpBuff[i*bw*3+j*3+1];
            pnowBackBuffer[fbw*4*(i+y)+(j+x)*4+2] = bmpBuff[i*bw*3+j*3+2];
            pnowBackBuffer[fbw*4*(i+y)+(j+x)*4+3] = 255;
        }
    } 
}



void * FrameBuffer::displayFun(void *arg)
{
    static struct timeval ts, te;
    static int frameCount = 0;
    int ret;

    FrameBuffer *frameBuffer=(FrameBuffer *)arg;

    while(1)
    {
        if(frameBuffer->pnowDrawBuffer != NULL)
        {
            memcpy(frameBuffer->fbp, frameBuffer->pnowDrawBuffer, frameBuffer->finfo.smem_len);
            if( (showfps == true) && (frameCount % 20 == 0) )
            {
                gettimeofday(&te, 0);
                float tus = (te.tv_sec - ts.tv_sec) * 1000 + (te.tv_usec - ts.tv_usec) / 1000;
                float fps = 20000.0f / tus;
                printf("fps = %.3f\n", fps);
                gettimeofday(&ts, 0);
            }
        }
        usleep(frameBuffer->frameRateTime);
        frameCount++;
    }
    return NULL;
}

void * FrameBuffer::frameBufferDrawOnce(void *arg)
{
    static struct timeval ts, te;
    static int frameCount = 0;
    int ret;

    FrameBuffer *frameBuffer=(FrameBuffer *)arg;
    if(frameBuffer->pnowDrawBuffer != NULL)
    {
        memcpy(frameBuffer->fbp, frameBuffer->pnowDrawBuffer, frameBuffer->finfo.smem_len);
        if( (showfps == true) && (frameCount % 20 == 0) )
        {
            gettimeofday(&te, 0);
            float tus = (te.tv_sec - ts.tv_sec) * 1000 + (te.tv_usec - ts.tv_usec) / 1000;
            float fps = 20000.0f / tus;
            printf("fps = %.3f\n", fps);
            gettimeofday(&ts, 0);
        }
    }
    frameCount++;
    return NULL;
}


/*
 * 函数名称 : Init
 * 函数介绍 : 初始化设备
 * 参数介绍 : fileName: 将要使用的设备名称
 * 返回值   : -1:失败，  0：成功
 */
int FrameBuffer::Init(char * fileName)
{
    /*打开设备文件*/  
    fd = open(fileName, O_RDWR);  
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
    /* Length of frame buffer mem */ 
    printf("finfo.smem_len=%d \n", finfo.smem_len); 
    /* see FB_TYPE_* */  
    printf("finfo.line_length=%d \n", finfo.line_length); 
    /* Start of Memory Mapped I/O */ 

    width = finfo.line_length / 4;
    height = vinfo.yres;

    printf("buffer width=%d height=%d\n", width,height);

    /*映射屏幕缓冲区到用户地址空间*/  
    fbp = (char *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  
    if (fbp == MAP_FAILED)  
    {  
        perror("map framebuffer device to memory ERR:\n");  
        return 0;
    }  

    /* 分配两个framebuffer的大小 */
    pSwapBuffer = (char *)malloc(finfo.smem_len*bufferCount);
    if(pSwapBuffer == NULL)
    {
        return -1;
    }

    pnowDrawBuffer = pSwapBuffer+finfo.smem_len*nowDrawBuffer;
    pnowBackBuffer = pSwapBuffer+finfo.smem_len*nowBackBuffer;
}


/*
 * 函数名称 : startDisplayThread
 * 函数介绍 : 开启绘制的线程
 * 参数介绍 : 无
 * 返回值   : -1:失败，  0：成功
 */
int FrameBuffer::startDisplayThread()
{
    pthread_t thr1;
    if(pthread_create(&thr1,NULL,displayFun,this)!=0)
    {
        printf("create thread failed!\n");
        return -1;
    }
}

/*
 * 函数名称 : Release
 * 函数介绍 : 释放分配的资源
 * 参数介绍 : 无
 * 返回值   : -1:失败，  0：成功
 */
int FrameBuffer::Release()
{
    /*释放缓冲区，关闭设备*/
    if( fbp != NULL )
    {
    	munmap(fbp, finfo.smem_len);  
    }

    if( fd <= 0)
    {
    	close(fd);
    }
    return 0;
}

/*
 * 函数名称 : swapDisplayBuffer
 * 函数介绍 : 交换绘制图像的buffer
 * 参数介绍 : 无
 * 返回值   : -1:失败，  0：成功
 */
int FrameBuffer::swapDisplayBuffer()
{
	/* 更新当前屏幕要显示的缓冲区 */
    if( nowDrawBuffer < (bufferCount-1) )
    {
	    nowDrawBuffer++;
    }
    else
    {
    	nowDrawBuffer = 0;
    }
    pnowDrawBuffer = pSwapBuffer+finfo.smem_len*nowDrawBuffer;

	/* 更新当前后台绘制的缓冲区 */
	if( nowBackBuffer < (bufferCount-1) )
    {
	    nowBackBuffer++;
    }
    else
    {
    	nowBackBuffer = 0;
    }
    pnowBackBuffer = pSwapBuffer+finfo.smem_len*nowBackBuffer;

    return 0;
}

/*
 * 函数名称 : setShowFPSStatus
 * 函数介绍 : 设置显示fps的状态
 * 参数介绍 : 无
 * 返回值   : -1:失败，  0：成功
 */
int FrameBuffer::setShowFPSStatus(bool status)
{
    showfps = status;
}
