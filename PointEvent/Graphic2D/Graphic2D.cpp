#include "Graphic2D.h"


Graphic2D::Graphic2D()
{

}

Graphic2D::~Graphic2D()
{

}



/*
 * 函数名称 : G2DShowBmp
 * 函数介绍 : 将指定的图像显示在指定的位置
 * 参数介绍 : disBuffer:disBuffer缓存，dw,dh:disBuffer宽高， x,y:显示的位置， bmpBuff:图片数据， bw,bh:图片宽高
 * 返回值   : -1:失败，  0：成功, 
 */
int Graphic2D::G2DShowBmp( char* disBuffer, int dw, int dh, int x, int y, char *bmpBuff, int bw, int bh)
{
    int drawW = 0;
    int drawH = 0;
    if( x < 0 )
    {
        x = 0;
    }

    if( y < 0 )
    {
        y = 0;
    }


    if ( x + bw > dw )
    {
        drawW = dw-x;
    }
    else
    {
        drawW = bw;
    }

    if ( y + bh > dh )
    {
        drawH = dh-y;
    }
    else
    {
        drawH = bh;
    }

    for( int i = 0; i < drawH; i++ )
    {
        for( int j = 0; j < drawW; j++ )
        {
            disBuffer[dw*4*(i+y)+(j+x)*4] 	= bmpBuff[i*bw*3+j*3+0];
            disBuffer[dw*4*(i+y)+(j+x)*4+1] = bmpBuff[i*bw*3+j*3+1];
            disBuffer[dw*4*(i+y)+(j+x)*4+2] = bmpBuff[i*bw*3+j*3+2];
            disBuffer[dw*4*(i+y)+(j+x)*4+3] = 255;
        }
    } 
}


/*
 * 函数名称 : G2DClearColor
 * 函数介绍 : 使用指定颜色刷新FrameBuffer
 * 参数介绍 : disBuffer:disBuffer x,y,w,h：清空的矩形， r,g,b:使用的颜色
 * 返回值   : -1:失败，  0：成功
 */
int Graphic2D::G2DClearColor(char* disBuffer, int dw, int dh, int x, int y, int w, int h, int r, int g, int b)
{
    int drawW = 0;
    int drawH = 0;
    if ( x + w > dw )
    {
        drawW = dw-x;
    }
    else
    {
        drawW = w;
    }

    if ( y + h > dh )
    {
        drawH = dh-y;
    }
    else
    {
        drawH = h;
    }

    for( int i = 0; i < drawH; i++ )
    {
        for( int j = 0; j < drawW; j++ )
        {
            disBuffer[dw*4*(i+y)+(j+x)*4] = b;
            disBuffer[dw*4*(i+y)+(j+x)*4+1] = g;
            disBuffer[dw*4*(i+y)+(j+x)*4+2] = r;
            disBuffer[dw*4*(i+y)+(j+x)*4+3] = 0;
        }
    }
    return 0;
}
