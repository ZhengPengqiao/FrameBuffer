#ifndef __GRAPHIC2D_H__
#define __GRAPHIC2D_H__


class Graphic2D
{
public:
	Graphic2D();
	~Graphic2D();
	/* 函数介绍 : 将指定的图像显示在指定的位置 */
	int G2DShowBmp( char* fbBuffer, int fbw, int fbh, int x, int y, char *bmpBuff, int bw, int bh);
	/* 函数介绍 : 使用指定颜色刷新FrameBuffer */
	int G2DClearColor(char* fbBuffer, int fbw, int fbh, int x, int y, int w, int h, int r, int g, int b);
};


#endif  //__GRAPHIC2D_H__