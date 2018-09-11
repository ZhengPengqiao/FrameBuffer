#ifndef _GMOUSEICON_H__
#define _GMOUSEICON_H__

#include "Graphic2D.h"
#include "bmp.h"
#include <stdio.h>
#include "FrameBuffer.h"

class GMouseIcon
{
public:
	GMouseIcon();
	~GMouseIcon();
	
	int onPaint();
	int setClickStatus(bool status);
	int setColor(int r, int g, int b);
	int setOnBackground(char *fileName);
	int setOffBackground(char *fileName);
	void setPosition(int x, int y, int width, int height);
	void setRelPosition(int x, int y);
	bool isActivity(int x, int y);
    FrameBuffer *framebuffer;

private:
	BmpInfo *bmpInfo;
	BmpInfo *bmpInfoOn;
	BmpInfo *bmpInfoOff;
    Graphic2D graphic2D;
    int x;
    int y;
    int width;
    int height;

    int r,g,b;
};

#endif  //_GMOUSEICON_H__