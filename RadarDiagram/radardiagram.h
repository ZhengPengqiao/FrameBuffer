#ifndef RADARDIAGRAM_H
#define RADARDIAGRAM_H

#include <list>     
#include <algorithm> 

using namespace std;

#define PIUNIT ((float)3.141/180)


class RadarDiagrame
{
public:
    RadarDiagrame();
    ~RadarDiagrame();
    int mallocPixelData(int width, int height);
    int setShowStatus(int x, int y, int sta);
    void updatePixeldata();
    void addArcArea(int centerX, int centerY, int degreeS, int degreeL,\
                               int diameterS, int diameterL,int csr,int csg,int csb,\
                               int cer,int ceg,int ceb);
    void setColor(int r, int g, int b);
    void drawPoint(int x, int y);
    void addArc(int centerX, int centerY, int degreeS, int degreeL, int diameter);

    int pixelwidth;
    int pixelheight;
    int pixellength;
    char* pixeldata;

private:
    int r;
    int g;
    int b;
    int status[3][7];
};

#endif // RADARDIAGRAM_H
