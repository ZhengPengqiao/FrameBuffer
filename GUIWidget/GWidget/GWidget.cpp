#include "GWidget.h"

GWidget::GWidget()
{
	x=y=width=height=0;
    r=g=b=56;
	bmpInfo = NULL;
	bmpInfoOn = NULL;
	bmpInfoOff = NULL;
	bmpInfo = bmpInfoOn;
}


GWidget::~GWidget()
{
    delete bmpInfoOn;
    delete bmpInfoOff;
}

int GWidget::setClickStatus(bool status)
{
	if(status)
	{
	    bmpInfo = bmpInfoOn;
	}
	else
	{
    	bmpInfo = bmpInfoOff;
	}
}

int GWidget::onPaint()
{
	if( bmpInfo != NULL )
	{
    	graphic2D.G2DShowBmp(framebuffer->pnowBackBuffer, framebuffer->width, framebuffer->height, \
        	x, y, width, height, bmpInfo->pixeldata, bmpInfo->imagewidth, bmpInfo->imageheight);
    }
    else
    {
	    graphic2D.G2DClearColor(framebuffer->pnowBackBuffer, framebuffer->width, framebuffer->height, \
	        x, y, width, height, r, g, b);	
	}
}

void GWidget::setPosition(int x, int y, int width, int height)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
}

int GWidget::setColor(int r, int g, int b)
{
	this->r = r;
	this->g = g;
	this->b = b;
}

int GWidget::setOnBackground(char *fileName)
{
	if( bmpInfoOn != NULL )
	{
		delete bmpInfoOn;
	}

	bmpInfoOn = new BmpInfo(fileName);
    /*BMP的上下是翻转的，这里将图像数据翻转过来*/
    bmpInfoOn->MirrorByV();
}

int GWidget::setOffBackground(char *fileName)
{
	if( bmpInfoOff != NULL )
	{
		delete bmpInfoOff;
	}
    bmpInfoOff = new BmpInfo(fileName);
    /*BMP的上下是翻转的，这里将图像数据翻转过来*/
    bmpInfoOff->MirrorByV();
}
