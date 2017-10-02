#ifndef _FRAMEBUFFER_H__
#define _FRAMEBUFFER_H__
#include <linux/fb.h>  

typedef struct FrameBufferInfo
{
    
}FrameBufferInfo;

class FrameBuffer
{
public:
	/* 构造函数 */
	FrameBuffer();
	/* 析构函数 */
	~FrameBuffer();
	/* 函数介绍 : 使用指定颜色刷新FrameBuffer*/
	int ClearColor(int x, int y, int w, int h, int r, int g, int b);
	/* 函数介绍 : 将指定的图像显示在指定的位置*/
	int ShowBmp( int x, int y, char *bmpBuff, int bw, int bh);
	/* 函数介绍 : 将指定的图像显示在指定的位置 */
	int BufferShowBmp( char* fbBuffer, int fbw, int fbh, int x, int y, char *bmpBuff, int bw, int bh);
	/* 函数介绍 : 使用指定颜色刷新FrameBuffer */
	int BufferClearColor(char* fbBuffer, int fbw, int fbh, int x, int y, int w, int h, int r, int g, int b);
	/* 函数介绍 : 初始化FrameBuffer */
	int Init(char * fileName);
	/* 函数介绍 : 交换绘制图像的buffer */
	int swapDisplayBuffer();
	/* 函数介绍 : 释放分配的资源*/
	int Release();
	/* 函数介绍 : 开启绘制的线程 */
	int startDisplayThread();
	/* 显示线程函数 */
	static void * displayFun(void *arg);


	struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int bufferCount;
    int nowDrawBuffer;		//供给FrameBuffer显示的缓冲区
    int nowBackBuffer;		//后台绘制的缓冲区
	char *pSwapBuffer;   	//为了实现双缓冲所分配的内存
	char *pnowDrawBuffer;  	//供给FrameBuffer显示的缓冲区
	char *pnowBackBuffer;  	//后台绘制的缓冲区
	int width;				//FrameBuffer的宽
	int height;				//FrameBuffer的高
	int frameRateTime;      //刷新间隔us
	char *fbp; 				//FrameBuffer的缓冲地址
private:
    int fd;  	
    
};

#endif //_FRAMEBUFFER_H__