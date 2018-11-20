#include <stdio.h>  
#include <stdlib.h>  
#include <fcntl.h>  
#include <string.h> 
#include <unistd.h> 
#include <linux/fb.h>  
#include <sys/mman.h>  
#include <sys/ioctl.h>  
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include "num.h"

#define DEV_NAME "/dev/fb0"


typedef struct FrameBufferInfo
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    char *fbp;
    int fd;  
}FrameBufferInfo;

/*
 * 函数名称 : showNum
 * 函数介绍 : 显示数字
 * 参数介绍 : fbInfo:FrameBuffer相关信息， x,y：清空的矩形，num:要显示的数字
 *           bps:framebuffer的bps
 * 返回值   : -1:失败，  0：成功
 */
int showNum(FrameBufferInfo fbInfo, int x, int y, unsigned char num, int bps)
{
    unsigned int rgb = 0;
    int fbw = 0;
    unsigned char *nump;
    unsigned char r,g,b;

    switch(num)
    {
        case '0': nump = data0; break;
        case '1': nump = data1; break;
        case '2': nump = data2; break;
        case '3': nump = data3; break;
        case '4': nump = data4; break;
        case '5': nump = data5; break;
        case '6': nump = data6; break;
        case '7': nump = data7; break;
        case '8': nump = data8; break;
        case '9': nump = data9; break;
        case ':': nump = datadd; break;
        case '.': nump = datad; break;
        case ' ': nump = dataspace; break;
        default: return -1;
    }

    switch( bps )
    {
        case 32:
            fbw = fbInfo.finfo.line_length/4;
        break;
        case 16:
            fbw = fbInfo.finfo.line_length/2;
        break;
    }
    for( int i = 0; i < font_width; i++ )
    {
        for( int j = 0; j < font_height; j++ )
        {
            b = nump[i*3+j*3*font_width];
            g = nump[i*3+j*3*font_width+1];
            r = nump[i*3+j*3*font_width+2];
            switch( bps )
            {
                case 32:
                    fbInfo.fbp[fbw*4*(j+y)+(i+x)*4] = b;
                    fbInfo.fbp[fbw*4*(j+y)+(i+x)*4+1] = g;
                    fbInfo.fbp[fbw*4*(j+y)+(i+x)*4+2] = r;
                    fbInfo.fbp[fbw*4*(j+y)+(i+x)*4+3] = 0xFF;
                break;
                case 16:
                    rgb = (((unsigned(r) << 8) & 0xF800) | ((unsigned(g) << 3) & 0x7E0) | ((unsigned(b) >> 3)));
                    fbInfo.fbp[fbw*2*(j+y)+(i+x)*2] = (rgb)&0xFF;
                    fbInfo.fbp[fbw*2*(j+y)+(i+x)*2+1] = (rgb>>8)&0xFF;
                break;
            }
        }
    }
    return 0;
}


/*
 * 函数名称 : showString
 * 函数介绍 : 显示字符串
 * 参数介绍 : fbInfo:FrameBuffer相关信息， x,y：清空的矩形, str:要显示的字符串
 *           bps:framebuffer的bps
 * 返回值   : -1:失败，  0：成功
 */
int showString(FrameBufferInfo fbInfo, int x, int y, char *str, int bps)
{
    int i = 0;
    for(i = 0; i < strlen(str); i++)
    {
        showNum(fbInfo, x+i*font_height, y, str[i], bps);
    }
}


void showHelp()
{
    printf("./programe options \n");    
    printf("USED:");
    printf("    -help: show help info\n");
    
}

int checkParam(int argc,char **argv)
{
    int i = 0;
    for(i = 1;i < argc;i++)
    {
        if( strcmp("-help", argv[i]) == 0 )
        {
            showHelp();
            return -1;
        }
        else
        {
            printf("param %s is not support \n\n", argv[i]);
            showHelp();
            return -1;
        }
    }
    return 0;
}


int main ( int argc, char *argv[] )  
{  
    FrameBufferInfo fbInfo;
    int bps = 0;
    char str[256];
    struct timeval now;
    int uart_fb;

    if ( checkParam(argc, argv) )
    {
        printf("checkParam Err\n");
        return 0;
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

    bps = fbInfo.vinfo.bits_per_pixel;
 
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
    
    uart_fb = open("/dev/ttymxc0", O_RDWR|O_NOCTTY|O_NONBLOCK);
    while(1)
    {
        write(uart_fb, str, strlen(str));
        gettimeofday(&now, NULL);
        sprintf(str, "%lu:%06d\n", now.tv_sec, now.tv_usec);
        showString(fbInfo, 100, 100, str, bps);
    }


    /*释放缓冲区，关闭设备*/
    munmap(fbInfo.fbp, fbInfo.finfo.smem_len);  
    close(fbInfo.fd);  
    close(uart_fb);  
    return 0;  
}
