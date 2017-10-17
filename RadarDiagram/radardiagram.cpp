#include "radardiagram.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <list>

RadarDiagrame::RadarDiagrame()
{
    memset(status,0, sizeof(status));
}

RadarDiagrame::~RadarDiagrame()
{
    if( pixeldata != NULL )
    {
        free(pixeldata);
    }
}


int RadarDiagrame::mallocPixelData(int width, int height)
{
    //计算像素数据长度
    int pixellength=width*3;
    while(pixellength%4 != 0)
    {
        pixellength++;
    }

    pixellength *= height;

    //读取像素数据
    pixeldata = (char *)malloc(pixellength);
    if(pixeldata == NULL)
    {
        this->pixelwidth = 0;
        this->pixelheight = 0;
        this->pixellength = 0;
        return -1;
    }

    this->pixellength = pixellength;
    this->pixelwidth = width;
    this->pixelheight = height;


    for(int i = 0; i < height; i++ )
    {
        for( int j = 0; j < width; j++ )
        {
            pixeldata[ (i*width+j)*3 ] = 20;
            pixeldata[ (i*width+j)*3+1 ] = 20;
            pixeldata[ (i*width+j)*3+2 ] = 20;
        }

    }
    
    updatePixeldata();
    return 0;
}

int RadarDiagrame::setShowStatus(int x, int y, int sta)
{
    status[x][y] = sta;
    updatePixeldata();
    return 0;
}


void RadarDiagrame::updatePixeldata()
{
    int i = 0;
    int j = 0;
    for(i = 0; i < 7; i++)
    {
        for(j = 0; j < 3; j++)
        {
            if(status[j][i] == 0)
            {
                    addArcArea(-50,100,  450+j*300,200, 100+i*15,10, 60,30*i,0  ,30,30*(i+1),0);
            }
        }
    }
}

void RadarDiagrame::addArcArea(int centerX, int centerY, int degreeS, int degreeL,\
                               int diameterS, int diameterL,int csr,int csg,int csb,\
                               int cer,int ceg,int ceb)
{
    int rStep = (cer-csr) / diameterL;
    int gStep = (ceg-csg) / diameterL;
    int bStep = (ceb-csb) / diameterL;
    
    for ( int i = diameterS; i < (diameterS+diameterL); i++ )
    {
        setColor(csr+rStep*(i-diameterS),csg+gStep*(i-diameterS),csb+bStep*(i-diameterS));
        addArc(centerX,centerY,degreeS,degreeL,i);
    }
}

void RadarDiagrame::setColor(int r, int g, int b)
{
    this->r = r;
    this->g = g;
    this->b = b;
}

void RadarDiagrame::drawPoint(int x, int y)
{
    if( (pixeldata != NULL) && ( x < pixelwidth ) && ( x >= 0 ) && ( y < pixelheight ) && ( y >= 0 ))
    {
        pixeldata[ (y*pixelwidth+x)*3 ] = b;
        pixeldata[ (y*pixelwidth+x)*3+1 ] = g;
        pixeldata[ (y*pixelwidth+x)*3+2 ] = r;
    }
}


void RadarDiagrame::addArc(int centerX, int centerY, int degreeS, int degreeL, int diameter)
{
    for( int i = degreeS; i < (degreeS+degreeL); i++ )
    {
        int x = (int)( centerX+(int)(diameter*sin(i*PIUNIT/10)) );
        int y = (int)( centerY+(int)(diameter*cos(i*PIUNIT/10)) );

        drawPoint(x,y);
    }
}
