#ifndef _GBUTTON_H__
#define _GBUTTON_H__

#include "Graphic2D.h"
#include "bmp.h"
#include <stdio.h>
#include "FrameBuffer.h"

class GWidget
{
public:
	GWidget();
	~GWidget();
	
	int onPaint();
	int setClickStatus(bool status);
	int setColor(int r, int g, int b);
	int setOnBackground(char *fileName);
	int setOffBackground(char *fileName);
	void setPosition(int x, int y, int width, int height);
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

#endif  //_GBUTTON_H__