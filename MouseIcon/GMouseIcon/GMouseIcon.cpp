#include "GMouseIcon.h"

GMouseIcon::GMouseIcon()
{
	x=y=width=height=0;
    r=g=b=56;
	bmpInfo = NULL;
	bmpInfoOn = NULL;
	bmpInfoOff = NULL;
	bmpInfo = bmpInfoOn;
}


GMouseIcon::~GMouseIcon()
{
    delete bmpInfoOn;
    delete bmpInfoOff;
}

int GMouseIcon::setClickStatus(bool status)
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

int GMouseIcon::onPaint()
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

void GMouseIcon::setPosition(int x, int y, int width, int height)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
}

void GMouseIcon::setRelPosition(int x, int y)
{
	this->x += x;
	this->y += y;
}


int GMouseIcon::setColor(int r, int g, int b)
{
	this->r = r;
	this->g = g;
	this->b = b;
}

int GMouseIcon::setOnBackground(char *fileName)
{
	if( bmpInfoOn != NULL )
	{
		delete bmpInfoOn;
	}

	bmpInfoOn = new BmpInfo(fileName);
    /*BMP的上下是翻转的，这里将图像数据翻转过来*/
    bmpInfoOn->MirrorByV();
}

int GMouseIcon::setOffBackground(char *fileName)
{
	if( bmpInfoOff != NULL )
	{
		delete bmpInfoOff;
	}
    bmpInfoOff = new BmpInfo(fileName);
    /*BMP的上下是翻转的，这里将图像数据翻转过来*/
    bmpInfoOff->MirrorByV();
}
