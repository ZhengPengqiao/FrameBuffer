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
#include <termios.h>
#include <signal.h>
#include "Graphic2D.h"
#include "bmp.h"
#include "FrameBuffer.h"
#include "InputEvent.h"

#define DEV_NAME "/dev/fb0"
InputEvent inputEvent;
int topx = 0;
int topy = 0;
BmpInfo *bmpInfo;
BmpInfo *bmpInfo1;
BmpInfo *bmpInfo2;
BmpInfo *bmpInfo3;
BmpInfo *bmpInfo4;

void inputEventLog(void *param, int type, int code, int value)
{
    static float x,y,id;
    //事件
    do {
        switch(type)
        {
            case EV_KEY:
                switch(code)
                {
                    case KEY_LEFT:
                        bmpInfo = bmpInfo1;
                    break;
                    case KEY_RIGHT:
                        bmpInfo = bmpInfo2;
                    break;
                    case KEY_UP:
                        bmpInfo = bmpInfo3;
                    break;
                    case KEY_DOWN:
                        bmpInfo = bmpInfo4;
                    break;
                    case KEY_ENTER:
                        topx += 50;
                        topy += 50;
                    break;
                }
            break;
        }
    } while(0);
    fflush(stdout);
}


void *thread_inputEventInput(void *param)
{
    inputEvent.inputEventDeal = inputEventLog;
    inputEvent.dealParam = param;

    if ( inputEvent.init() ) 
    {
        inputEvent.run();
        return NULL;
    }
    else 
    {
        printf("init fail!\n");
    }
    return NULL;
}
bool exitFlag = true;

void CTRL_C_Stop(int signo)   
{  
    exitFlag = false;
} 


int main ( int argc, char *argv[] )  
{
    FrameBuffer framebuffer;
    Graphic2D graphic2D;
    pthread_t inputEventptid;
    /* 取消回显的状态 */
    struct termios new_setting,init_setting;

    signal(SIGINT,  CTRL_C_Stop);
    signal(SIGKILL, CTRL_C_Stop);
    signal(SIGQUIT, CTRL_C_Stop);
    signal(SIGTERM, CTRL_C_Stop);

    tcgetattr(0,&init_setting);
    new_setting=init_setting;
    new_setting.c_lflag&=~ECHO;
    tcsetattr(0,TCSANOW,&new_setting);


    int ptre = pthread_create(&inputEventptid, NULL, thread_inputEventInput, NULL);
    if (ptre != 0) 
    {
        printf("thread_inputEventInput thread create fail!\n");
    } 


    framebuffer.Init((char*)DEV_NAME);
    framebuffer.setShowFPSStatus(false);
    bmpInfo1 = new BmpInfo((char*)"./assert/1.bmp");
    bmpInfo2 = new BmpInfo((char*)"./assert/2.bmp");
    bmpInfo3 = new BmpInfo((char*)"./assert/3.bmp");
    bmpInfo4 = new BmpInfo((char*)"./assert/4.bmp");
    /*BMP的上下是翻转的，这里将图像数据翻转过来*/
    bmpInfo1->MirrorByV();
    bmpInfo2->MirrorByV();
    bmpInfo3->MirrorByV();
    bmpInfo4->MirrorByV();

    bmpInfo = bmpInfo1;

    printf("fileName=%s  width=%d height=%d length=%d\n", bmpInfo->fileName, bmpInfo->imagewidth, bmpInfo->imageheight, bmpInfo->pixellength);
    inputEvent.PostClickEvent();
    while(exitFlag)
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

    /* 回复之前的状态 */
    tcsetattr(0,TCSANOW,&init_setting);

    delete bmpInfo1;
    delete bmpInfo2;
    delete bmpInfo3;
    delete bmpInfo4;
    framebuffer.Release();
    
    return 0;  
}  

