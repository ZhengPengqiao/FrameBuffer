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
#include "GMouseIcon.h"

#define DEV_NAME "/dev/fb0"
InputEvent inputEvent;
GMouseIcon *gMouseIcon;
FrameBuffer framebuffer;

void inputEventLog(void *param, int type, int code, int value)
{
    static float x,y,id;


    printf("%d %d %d\n", type, code, value);
    //事件
    do {
        switch(type)
        {
            case EV_KEY:
                switch(code)
                {
                    case KEY_LEFT:
                    break;
                    case KEY_RIGHT:
                    break;
                    case KEY_UP:
                    break;
                    case KEY_DOWN:
                    break;
                    case KEY_ENTER:
                        if(value == 1)
                        {
                            gMouseIcon->setClickStatus(true);
                            gMouseIcon->onPaint();
                            framebuffer.swapDisplayBuffer();
                        }
                        else if( value == 0 )
                        {
                            gMouseIcon->setClickStatus(false);
                            gMouseIcon->onPaint();
                            framebuffer.swapDisplayBuffer();
                        }
                    break;
                    case 272:
                        if(value == 1)
                        {
                            gMouseIcon->setClickStatus(true);
                            gMouseIcon->onPaint();
                            framebuffer.swapDisplayBuffer();
                        }
                        else if( value == 0 )
                        {
                            gMouseIcon->setClickStatus(false);
                            gMouseIcon->onPaint();
                            framebuffer.swapDisplayBuffer();
                        }
                    break;
                }
            break;
            case EV_REL:
                switch(code)
                {
                    case REL_X:
                        gMouseIcon->setRelPosition(value, 0);
                        gMouseIcon->onPaint();
                        framebuffer.swapDisplayBuffer();
                    break;
                    case REL_Y:
                        gMouseIcon->setRelPosition(0, value);
                        gMouseIcon->onPaint();
                        framebuffer.swapDisplayBuffer();
                    break;
                }
            break;
            case EV_MSC:
                switch(code)
                {
                    case KEY_3:
                        if(value == 1)
                        {
                            gMouseIcon->setClickStatus(true);
                            gMouseIcon->onPaint();
                            framebuffer.swapDisplayBuffer();
                        }
                        else if( value == 0 )
                        {
                            gMouseIcon->setClickStatus(false);
                            gMouseIcon->onPaint();
                            framebuffer.swapDisplayBuffer();
                        }
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
    framebuffer.startDisplayThread();

    gMouseIcon = new GMouseIcon();
    gMouseIcon->framebuffer = &framebuffer;
    gMouseIcon->setPosition(16,16,16,16);
    gMouseIcon->setOnBackground((char*)"./assert/mouseOn.bmp");
    gMouseIcon->setOffBackground((char*)"./assert/mouseOff.bmp");
    gMouseIcon->onPaint();
    framebuffer.swapDisplayBuffer();

    inputEvent.PostClickEvent();
    
    while(exitFlag)
    {
        usleep(33000);
    }

    /* 回复之前的状态 */
    tcsetattr(0,TCSANOW,&init_setting);

    framebuffer.Release();
    
    return 0;  
}  

