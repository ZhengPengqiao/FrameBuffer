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
#include <signal.h>
#include "./BMP/bmp.h"

#define DEV_NAME "/dev/fb0"


typedef struct FrameBufferInfo
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    char *fbp;
    int fd;  
}FrameBufferInfo;

static int r = 0;
static int g = 0;
static int b = 0;
static char alpha = 0x00;
static int uc = 0;
static int swapbr = 0;
static int l_num = 8;
#define NO_MOVE 0
#define LEFTTORIGHT 1
#define RIGHTTOLEFT 2
#define TOPTOBUTTON 3
#define BUTTONTOTOP 4
static int x_o = 0;

static int is_move = false;
static int move_offset = 0;
static int screen_offset = 0;
static bool exitflag = false;
static char data[4096*4096*4];
static char *bmp_filename=(char*)"./assert/bmp.bmp";

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
        case 24:
            fbw = fbInfo.finfo.line_length/3;
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

    for( int i = 0; i < drawH; i++ )
    {
        for( int j = 0; j < drawW; j++ )
        {
            switch( bps )
            {
                case 32:
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4]   = b&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+1] = g&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+2] = r&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+3] = alpha;
                break;
                case 24:
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3]   = b&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+1] = g&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+2] = r&0xFF;
                break;
                case 16:

                    rgb = (((unsigned(r) << 8) & 0xF800) | 
                            ((unsigned(g) << 3) & 0x7E0) | 
                            ((unsigned(b) >> 3)));
                    data[screen_offset+fbw*2*(i+y)+(j+x)*2] = (rgb)&0xFF;
                    data[screen_offset+fbw*2*(i+y)+(j+x)*2+1] = (rgb>>8)&0xFF;
                break;
            }
        }
    }
    return 0;
}


/*
 * 函数名称 : ClearRightCol
 * 函数介绍 : 将最后一列画黑
 * 参数介绍 : fbInfo:FrameBuffer相关信息，r,g,b:使用的颜色,
 *           bps:framebuffer的bps
 * 返回值   : -1:失败，  0：成功
 */
int ClearRightCol(FrameBufferInfo fbInfo, int r, int g, int b, int bps)
{
    unsigned int rgb = 0;
    int drawW = 0;
    int drawH = 0;
    int fbw = 0;
    int fbh = fbInfo.vinfo.yres_virtual;
  
    switch( bps )
    {
        case 32:
            fbw = fbInfo.finfo.line_length/4;
        break;
        case 24:
            fbw = fbInfo.finfo.line_length/3;
        break;
        case 16:
            fbw = fbInfo.finfo.line_length/2;
        break;
    }
    drawW = fbw;
    drawH = fbh;

    for( int i = 0; i < drawH; i++ )
    {
        switch( bps )
        {
            case 32:
                fbInfo.fbp[fbw*4*i+(drawW-1)*4]   = b&0xFF;
                fbInfo.fbp[fbw*4*i+(drawW-1)*4+1] = g&0xFF;
                fbInfo.fbp[fbw*4*i+(drawW-1)*4+2] = r&0xFF;
                fbInfo.fbp[fbw*4*i+(drawW-1)*4+3] = alpha;
            break;
            case 24:
                fbInfo.fbp[fbw*3*i+(drawW-1)*3]   = b&0xFF;
                fbInfo.fbp[fbw*3*i+(drawW-1)*3+1] = g&0xFF;
                fbInfo.fbp[fbw*3*i+(drawW-1)*3+2] = r&0xFF;
            break;
            case 16:

                rgb = (((unsigned(r) << 8) & 0xF800) | 
                        ((unsigned(g) << 3) & 0x7E0) | 
                        ((unsigned(b) >> 3)));
                fbInfo.fbp[fbw*2*i+(drawW-1)*2] = (rgb)&0xFF;
                fbInfo.fbp[fbw*2*i+(drawW-1)*2+1] = (rgb>>8)&0xFF;
            break;
        }
    }
    return 0;
}


/*
 * 函数名称 : TestColor
 * 函数介绍 : 测试颜色FrameBuffer
 * 参数介绍 : fbInfo:FrameBuffer相关信息， x,y,w,h：清空的矩形，  br,bg,bb:使用的颜色,
 *           bps:framebuffer的bps
 * 返回值   : -1:失败，  0：成功
 */
int TestColor(FrameBufferInfo fbInfo, int x, int y, int w, int h, int br, int bg, int bb, int bps)
{
    unsigned int rgb = 0;
    int drawW = 0;
    int drawH = 0;
    int fbw = 0;
    int fbh = fbInfo.vinfo.yres;
    int r = br, g = bg, b = bb;
    int rgFlag = 0;
    
    switch( bps )
    {
        case 32:
            fbw = fbInfo.finfo.line_length/4;
        break;
        case 24:
            fbw = fbInfo.finfo.line_length/3;
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

    for( int i = 0; i < drawH; i++ )
    {
        for( int j = 0; j < drawW; j++ )
        {

            r = (unsigned int)(br + (float)i*255/drawH)%255;
            g = (unsigned int)(bg + (float)j*255/drawW)%255;
            b = bb % 255;
            switch( bps )
            {
                case 32:
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4]   = b&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+1] = g&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+2] = r&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+3] = alpha;
                break;
                case 24:
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3]   = b&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+1] = g&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+2] = r&0xFF;
                break;
                case 16:
                    rgb = (((r << 8) & 0xF800) | 
                        ((g << 3) & 0x7E0) | 
                        ((b >> 3)));
                    data[screen_offset+fbw*2*(i+y)+(j+x)*2] = (rgb)&0xFF;
                    data[screen_offset+fbw*2*(i+y)+(j+x)*2+1] = (rgb>>8)&0xFF;
                break;
            }
        }
    }
    return 0;
}



/*
 * 函数名称 : TestSolidColor
 * 函数介绍 : 测试单颜色FrameBuffer
 * 参数介绍 : fbInfo:FrameBuffer相关信息， x,y,w,h：清空的矩形，  br,bg,bb:使用的颜色,
 *           bps:framebuffer的bps
 * 返回值   : -1:失败，  0：成功
 */
int TestSolidColor(FrameBufferInfo fbInfo, int x, int y, int w, int h, int br, int bg, int bb, int bps)
{
    unsigned int rgb = 0;
    int drawW = 0;
    int drawH = 0;
    int fbw = 0;
    int fbh = fbInfo.vinfo.yres;
    int r = br, g = bg, b = bb;
    int rgFlag = 0;
    
    switch( bps )
    {
        case 32:
            fbw = fbInfo.finfo.line_length/4;
        break;
        case 24:
            fbw = fbInfo.finfo.line_length/3;
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

    for( int i = 0; i < drawH; i++ )
    {
        r = br;
        g = bg;
        b = bb;
        for( int j = 0; j < drawW; j++ )
        {
            if ( i <= drawH/3 )
            {
                r = (unsigned int)(br + j*255/drawW)%255;
            }
            else if ( drawH/3 && i <= drawH*2/3 )
            {
                g = (unsigned int)(bg + j*255/drawW)%255;
            }
            else if ( drawH*2/3 < i )
            {
                b = (unsigned int)(bb + j*255/drawW)%255;
            }
            
            switch( bps )
            {
                case 32:
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4]   = b&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+1] = g&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+2] = r&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+3] = alpha;
                break;
                case 24:
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3]   = b&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+1] = g&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+2] = r&0xFF;
                break;
                case 16:
                    rgb = (((r << 8) & 0xF800) | 
                        ((g << 3) & 0x7E0) | 
                        ((b >> 3)));
                    data[screen_offset+fbw*2*(i+y)+(j+x)*2] = (rgb)&0xFF;
                    data[screen_offset+fbw*2*(i+y)+(j+x)*2+1] = (rgb>>8)&0xFF;
                break;
            }
        }
    }
    return 0;
}



/*
 * 函数名称 : TestUrandom
 * 函数介绍 : 测试颜色FrameBuffer
 * 参数介绍 : fbInfo:FrameBuffer相关信息， x,y,w,h：清空的矩形
 *           bps:framebuffer的bps
 * 返回值   : -1:失败，  0：成功
 */
int TestUrandom(FrameBufferInfo fbInfo, int x, int y, int w, int h, int bps)
{
    unsigned int rgb = 0;
    int drawW = 0;
    int drawH = 0;
    int fbw = 0;
    int fbh = fbInfo.vinfo.yres;
    int rgFlag = 0;
    int r = 0;
    int g = 0;
    int b = 0;
    int a = 0;

    switch( bps )
    {
        case 32:
            fbw = fbInfo.finfo.line_length/4;
        break;
        case 24:
            fbw = fbInfo.finfo.line_length/3;
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

    for( int i = 0; i < drawH; i++ )
    {
        for( int j = 0; j < drawW; j++ )
        {
            r = rand()/255;
            g = rand()/255;
            b = rand()/255;
            a = rand()/255;
            switch( bps )
            {
                case 32:
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4]   = b&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+1] = g&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+2] = r&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+3] = a;
                    
                break;
                case 24:
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3]   = b&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+1] = g&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+2] = r&0xFF;
                break;
                case 16:
                    rgb = (((r << 8) & 0xF800) | 
                        ((g << 3) & 0x7E0) | 
                        ((b >> 3)));
                    data[screen_offset+fbw*2*(i+y)+(j+x)*2] = (rgb)&0xFF;
                    data[screen_offset+fbw*2*(i+y)+(j+x)*2+1] = (rgb>>8)&0xFF;
                break;
            }
        }
    }
    return 0;
}


/*
 * 函数名称 : CheckerBoard
 * 函数介绍 : 测试颜色FrameBuffer
 * 参数介绍 : fbInfo:FrameBuffer相关信息， x,y,w,h：清空的矩形
 *           bps:framebuffer的bps
 * 返回值   : -1:失败，  0：成功
 */
int CheckerBoard(FrameBufferInfo fbInfo, int x, int y, int w, int h, int bps)
{
    unsigned int rgb = 0;
    int drawW = 0;
    int drawH = 0;
    int fbw = 0;
    int fbh = fbInfo.vinfo.yres;
    int r, g, b;
    int rgFlag = 0;
    
    switch( bps )
    {
        case 32:
            fbw = fbInfo.finfo.line_length/4;
        break;
        case 24:
            fbw = fbInfo.finfo.line_length/3;
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

    for( int i = 0; i < drawH; i++ )
    {
        for( int j = 0; j < drawW; j++ )
        {

            r = (i/(drawH/10))*70%255;
            g = (j/(drawW/10))*40%255;
            b = (r+g)%255;
            switch( bps )
            {
                case 32:
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4]   = b&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+1] = g&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+2] = r&0xFF;
                    data[screen_offset+fbw*4*(i+y)+(j+x)*4+3] = alpha;
                break;
                case 24:
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3]   = b&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+1] = g&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+2] = r&0xFF;
                break;
                case 16:
                    rgb = (((r << 8) & 0xF800) | 
                        ((g << 3) & 0x7E0) | 
                        ((b >> 3)));
                    data[screen_offset+fbw*2*(i+y)+(j+x)*2] = (rgb)&0xFF;
                    data[screen_offset+fbw*2*(i+y)+(j+x)*2+1] = (rgb>>8)&0xFF;
                break;
            }
        }
    }
    return 0;
}


/*
 * 函数名称 : CheckerBoardCycleFrameBuffer
 * 函数介绍 : 测试颜色FrameBuffer, 按照w,h扩至款宽高大小，如果超出屏幕，就绘制到屏幕开始
 * 参数介绍 : fbInfo:FrameBuffer相关信息， x,y,w,h：清空的矩形
 *           bps:framebuffer的bps
 * 返回值   : -1:失败，  0：成功
 */
int CheckerBoardCycleFrameBuffer(FrameBufferInfo fbInfo, int x, int y, int w, int h, int bps)
{
    unsigned int rgb = 0;
    int drawW = 0;
    int drawH = 0;
    int fbw = 0;
    int fbh = fbInfo.vinfo.yres;
    int r, g, b;
    int rgFlag = 0;
    
    switch( bps )
    {
        case 32:
            fbw = fbInfo.finfo.line_length/4;
        break;
        case 24:
            fbw = fbInfo.finfo.line_length/3;
        break;
        case 16:
            fbw = fbInfo.finfo.line_length/2;
        break;
    }

    drawW = w;
    drawH = h;

    for( int i = 0; i < drawH; i++ )
    {
        for( int j = 0; j < drawW; j++ )
        {

            r = (i/(drawH/10))*70%255;
            g = (j/(drawW/10))*40%255;
            b = (r+g)%255;
            switch( bps )
            {
                case 32:
                    data[screen_offset+fbw*4*( (i+y)%drawH )+( (j+x)%drawW )*4]   = b&0xFF;
                    data[screen_offset+fbw*4*( (i+y)%drawH )+( (j+x)%drawW )*4+1] = g&0xFF;
                    data[screen_offset+fbw*4*( (i+y)%drawH )+( (j+x)%drawW )*4+2] = r&0xFF;
                    data[screen_offset+fbw*4*( (i+y)%drawH )+( (j+x)%drawW )*4+3] = alpha;
                break;
                case 24:
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3]   = b&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+1] = g&0xFF;
                    data[screen_offset+fbw*3*(i+y)+(j+x)*3+2] = r&0xFF;
                break;
                case 16:
                    rgb = (((r << 8) & 0xF800) | 
                        ((g << 3) & 0x7E0) | 
                        ((b >> 3)));
                    data[screen_offset+fbw*2*( (i+y)%drawH )+( (j+x)%drawW )*2] = (rgb)&0xFF;
                    data[screen_offset+fbw*2*( (i+y)%drawH )+( (j+x)%drawW )*2+1] = (rgb>>8)&0xFF;
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
    return 0;
}

void showHelp()
{
    printf("./programe options \n");    
    printf("USED:");
    printf("    -help: show help info\n");
    printf("    -uc value: 0:gradual change 1:linear leaf 2:user color 3:checkerboard 4:usr Line Num(Black/White) 5:urandom 6:SolidColor 8:bmp image  (default %d)\n", uc);
    printf("    -r value: r value (default %d)\n", r);
    printf("    -g value: g value (default %d)\n", g);
    printf("    -b value: b value (default %d)\n", b);
    printf("    -alpha value: alpha value (default %d)\n", alpha);
    printf("    -l value: line num <-uc 4>(default %d)\n", l_num);
    printf("    -move val: run move 0:NO 1:LTR 2:RTL 3:TTB 4:BTT(default %d)\n", is_move);
    printf("    -x_o val: x_o to clear(default 0)\n");
    printf("    -swapbr: will swap red blue offset\n");
    printf("    -so val: screen offset\n");
    printf("    -bmp filename: bmp_filename(default:%s)\n", bmp_filename);
    
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
        else if( strcmp("-uc", argv[i]) == 0 )
        {
            uc = atoi(argv[i+1]);
            i++;
        }
        else if( strcmp("-r", argv[i]) == 0 )
        {
            r = atoi(argv[i+1]);
            i++;
        }
        else if( strcmp("-g", argv[i]) ==0 )
        {
            g = atoi(argv[i+1]);
            i++;
        }
        else if( strcmp("-b", argv[i]) ==0 )
        {
            b = atoi(argv[i+1]);
            i++;
        }
        else if( strcmp("-alpha", argv[i]) ==0 )
        {
            alpha = atoi(argv[i+1]);
            i++;
        }
        else if( strcmp("-l", argv[i]) ==0 )
        {
            l_num = atoi(argv[i+1]);
            i++;
        }
        else if( strcmp("-so", argv[i]) ==0 )
        {
            screen_offset = atoi(argv[i+1]);
            i++;
        }
        else if( strcmp("-xo", argv[i]) ==0 )
        {
            x_o = atoi(argv[i+1]);
            i++;
        }
        else if( strcmp("-bmp", argv[i]) ==0 )
        {
            bmp_filename = argv[i+1];
            i++;
        }
        else if( strcmp("-swapbr", argv[i]) ==0 )
        {
            swapbr = 1;
        }
        else if( strcmp("-move", argv[i]) ==0 )
        {
            is_move = atoi(argv[i+1]);
            if( is_move > 4 )
            {
                printf("param -move %s is not support \n\n", argv[i]);
                showHelp();
                return -1;
            }
            i++;
            printf("set -move=%d\n", is_move);
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


static void exit_signal(int signo)
{
    printf("receive signal exit, will exit\n");
	exitflag = true;
}

int main ( int argc, char *argv[] )  
{  
    FrameBufferInfo fbInfo;
    int topx = 0;
    int topy = 0;
    int frameCount = 0;
    int bps = 0;
    unsigned int rgb = 0;
    float len = 10;
    BmpInfo *bmpInfo = NULL;

    if ( checkParam(argc, argv) )
    {
        printf("checkParam Err\n");
        return 0;
    }
    
	signal(SIGQUIT, exit_signal);
	signal(SIGINT,  exit_signal);
	signal(SIGPIPE, SIG_IGN);

    if( uc == 8 )
    {
        bmpInfo = new BmpInfo(bmp_filename);
        /*BMP的上下是翻转的，这里将图像数据翻转过来*/
        bmpInfo->MirrorByV();
        printf("fileName=%s  width=%d height=%d length=%d\n", bmpInfo->fileName, bmpInfo->imagewidth, bmpInfo->imageheight, bmpInfo->pixellength);
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

    if( swapbr == 1 )
    {
        printf("============= NOTE ==============\n");
        printf("THIS OPERATION WILL EFFECT SYSTEM\n");
        printf("=================================\n");
        printf("will swap red's offset and blue's offset\n");
        int tmp = fbInfo.vinfo.red.offset;
        fbInfo.vinfo.red.offset = fbInfo.vinfo.blue.offset;
        fbInfo.vinfo.blue.offset=tmp;
  
        if (ioctl(fbInfo.fd, FBIOPUT_VSCREENINFO, &fbInfo.vinfo))  
        {
            perror("reading variable information ERR:\n"); 
            close(fbInfo.fd);
            printf("ERR----> OPERATION ERR, SWAP NOT EFFECT\n"); 
            return 0; 
        }
        close(fbInfo.fd);
        printf("OK----> OPERATION OK, SWAP EFFECT\n"); 
        printf("=================================\n");
        return 0;

    }

    /*映射屏幕缓冲区到用户地址空间*/  
    fbInfo.fbp = (char *)mmap(0, fbInfo.finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbInfo.fd, 0);  
    if (fbInfo.fbp == MAP_FAILED)  
    {  
        perror("map framebuffer device to memory ERR:\n");  
        return 0;
    }  
    
    printf("clear uc=%d r=%d g=%d b=%d a=%d\n", uc, r, g, b, alpha);

    while(!exitflag)
    {
        if ( uc == 0 )
        {
            TestColor(fbInfo, 0, 0, fbInfo.vinfo.xres, fbInfo.vinfo.yres, r, g, b, bps);
        }
        else if(uc == 1)
        {
            if( is_move == LEFTTORIGHT )
            {
                move_offset += 5;
                len = fbInfo.vinfo.xres/8;
                ClearFrameBuff(fbInfo, (int)(0*len+move_offset)%fbInfo.vinfo.xres, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0xff, 0xff, 0xff, bps);
                ClearFrameBuff(fbInfo, (int)(1*len+move_offset)%fbInfo.vinfo.xres, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0xb8, 0xc6, 0x00, bps);
                ClearFrameBuff(fbInfo, (int)(2*len+move_offset)%fbInfo.vinfo.xres, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0x00, 0xc2, 0xca, bps);
                ClearFrameBuff(fbInfo, (int)(3*len+move_offset)%fbInfo.vinfo.xres, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0x00, 0xc6, 0x00, bps);
                ClearFrameBuff(fbInfo, (int)(4*len+move_offset)%fbInfo.vinfo.xres, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0xc7, 0x00, 0xb9, bps);
                ClearFrameBuff(fbInfo, (int)(5*len+move_offset)%fbInfo.vinfo.xres, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0xc4, 0x00, 0x00, bps);
                ClearFrameBuff(fbInfo, (int)(6*len+move_offset)%fbInfo.vinfo.xres, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0x00, 0x00, 0xcb, bps);
                ClearFrameBuff(fbInfo, (int)(7*len+move_offset)%fbInfo.vinfo.xres, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0x00, 0x00, 0x00, bps); 
            }
            else
            {
                len = fbInfo.vinfo.xres/8;
                ClearFrameBuff(fbInfo, 0*len, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0xff, 0xff, 0xff, bps);
                ClearFrameBuff(fbInfo, 1*len, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0xb8, 0xc6, 0x00, bps);
                ClearFrameBuff(fbInfo, 2*len, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0x00, 0xc2, 0xca, bps);
                ClearFrameBuff(fbInfo, 3*len, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0x00, 0xc6, 0x00, bps);
                ClearFrameBuff(fbInfo, 4*len, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0xc7, 0x00, 0xb9, bps);
                ClearFrameBuff(fbInfo, 5*len, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0xc4, 0x00, 0x00, bps);
                ClearFrameBuff(fbInfo, 6*len, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0x00, 0x00, 0xcb, bps);
                ClearFrameBuff(fbInfo, 7*len, 0, fbInfo.vinfo.xres/8, fbInfo.vinfo.yres,  0x00, 0x00, 0x00, bps); 
            }
            

        }
        else if(uc == 2)
        {
            ClearFrameBuff(fbInfo, 0, 0, fbInfo.vinfo.xres, fbInfo.vinfo.yres, r, g, b, bps);
        }
        else if(uc == 3)
        {
            if( is_move == LEFTTORIGHT )
            {
                move_offset += 5;
                CheckerBoardCycleFrameBuffer(fbInfo, move_offset, 0, fbInfo.vinfo.xres, fbInfo.vinfo.yres, bps);
            }
            else
            {
                CheckerBoard(fbInfo, move_offset, 0, fbInfo.vinfo.xres, fbInfo.vinfo.yres, bps);
            }
        }
        else if(uc == 4)
        {
            len = fbInfo.vinfo.xres/l_num;
            for( int i = 0; i < l_num; i++ )
            {
                if( i%2 == 0 )
                {
                    ClearFrameBuff(fbInfo, i*len, 0, fbInfo.vinfo.xres/l_num, fbInfo.vinfo.yres,  0xff, 0xff, 0xff, bps);
                }
                else
                {
                    ClearFrameBuff(fbInfo, i*len, 0, fbInfo.vinfo.xres/l_num, fbInfo.vinfo.yres,  0x00, 0x00, 0x00, bps);
                }
            }
        }
        else if(uc == 5)
        {
            TestUrandom(fbInfo, 0, 0, fbInfo.vinfo.xres, fbInfo.vinfo.yres, bps);
        }
        else if(uc == 6)
        {
            TestSolidColor(fbInfo, 0, 0, fbInfo.vinfo.xres, fbInfo.vinfo.yres, r, g, b, bps);
        }
        else if( uc == 7 )
        {
            ClearRightCol(fbInfo, 0x00, 0x00, 0x00, bps); 
        }
        else if( uc == 8 )
        {
            ShowBmp(fbInfo, 0, 0,\
                bmpInfo->pixeldata, bmpInfo->imagewidth, bmpInfo->imageheight, bps);
        }

        if( uc != 7 )
        {
            ClearFrameBuff(fbInfo, fbInfo.vinfo.xres-x_o, 0, x_o, fbInfo.vinfo.yres,  0x00, 0x00, 0x00, bps); 

            write(fbInfo.fd, data, fbInfo.finfo.smem_len);
            usleep(33000);
        }
        else
        {
            usleep(10000);
        }
    }

    /*释放缓冲区，关闭设备*/
    munmap(fbInfo.fbp, fbInfo.finfo.smem_len);  
    close(fbInfo.fd);  
    if ( bmpInfo != NULL )
    {
        delete bmpInfo;
    }
    printf("exit success\n");
    return 0;  
}  
