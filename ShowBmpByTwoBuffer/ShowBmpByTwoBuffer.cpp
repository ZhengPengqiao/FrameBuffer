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
#include "Graphic2D.h"
#include "bmp.h"
#include "FrameBuffer.h"

#define DEV_NAME "/dev/fb0"


int main ( int argc, char *argv[] )  
{
    FrameBuffer framebuffer;
    Graphic2D graphic2D;
    int topx = 0;
    int topy = 0;

    framebuffer.Init((char*)DEV_NAME);
    BmpInfo *bmpInfo = new BmpInfo((char*)"./assert/image.bmp");
    /*BMP的上下是翻转的，这里将图像数据翻转过来*/
    bmpInfo->MirrorByV();
    printf("fileName=%s  width=%d height=%d length=%d\n", bmpInfo->fileName, bmpInfo->imagewidth, bmpInfo->imageheight, bmpInfo->pixellength);
    while(1)
    {
        graphic2D.G2DClearColor(framebuffer.pnowBackBuffer, framebuffer.width, framebuffer.height,\
            0, 0, framebuffer.width, framebuffer.height, 0, 0, 0);
        graphic2D.G2DShowBmp(framebuffer.pnowBackBuffer, framebuffer.width, framebuffer.height, \
            topx, topy, bmpInfo->pixeldata, bmpInfo->imagewidth, bmpInfo->imageheight);

        framebuffer.swapDisplayBuffer();
        FrameBuffer::frameBufferDrawOnce(&framebuffer);

        usleep(33000);
        
        topx+=3;
        if( topx > framebuffer.vinfo.xres )
        {
            topx = 0;
        }
        topy+=5;
        if( topy > framebuffer.vinfo.yres )
        {
            topy = 0;
        }
    }

    delete bmpInfo;
    framebuffer.Release();
    return 0;  
}  
