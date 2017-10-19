#include <stdio.h>  
#include <stdlib.h>  
#include <fcntl.h>  
#include <string.h> 
#include <unistd.h> 
#include <linux/fb.h>  
#include <sys/mman.h>  
#include <sys/ioctl.h>  
#include <arpa/inet.h>  
  
#define DEV_NAME "/dev/fb0"
#define SAV_NAME "./save.data"

int main ( int argc, char *argv[] )  
{  
    char *buffer;
    int fd = 0;  
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    struct fb_bitfield red;
    struct fb_bitfield green;
    struct fb_bitfield blue;
    char *fbp = 0;
    char * pDstBuf;
    int savefb = 0;
    int ret;

    /*打开设备文件*/  
    fd = open(DEV_NAME, O_RDWR|O_CREAT|O_TRUNC, 0777);  
    if (!fd)  
    {
        perror("open fb device ERR:"); 
        return 0; 
    }
  
    savefb = open(SAV_NAME, O_RDWR);
    if (!savefb)  
    {
        perror("open SAV_NAME ERR:"); 
        return 0; 
    }

    /*取得屏幕相关参数*/

  
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo))  
    {
        perror("reading variable information ERR:\n"); 
        close(fd);  
        return 0; 
    }
 
    /* finfo 它描述显示卡的属性，并且系统运行时不能被修改*/
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo))  
    {
        perror("reading fixed information ERR:"); 
        close(fd); 
        return 0; 
    }

    printf("vinfo.red   offset=%d   length=%d   msb_right=%d\n",  vinfo.red.offset,  vinfo.red.length,  vinfo.red.msb_right);
    printf("vinfo.green   offset=%d   length=%d   msb_right=%d\n",  vinfo.green.offset,  vinfo.green.length,  vinfo.green.msb_right);
    printf("vinfo.blue   offset=%d   length=%d   msb_right=%d\n",  vinfo.blue.offset,  vinfo.blue.length,  vinfo.blue.msb_right);
    printf("vinfo.transp   offset=%d   length=%d   msb_right=%d\n",  vinfo.transp.offset,  vinfo.transp.length,  vinfo.transp.msb_right);
    printf("vinfo.bits_per_pixel=%d\n", vinfo.bits_per_pixel);
    printf("vinfo.xres_virtual=%d vinfo.yres_virtual=%d\n", vinfo.xres_virtual, vinfo.yres_virtual);
    printf("vinfo.xoffset=%d  vinfo.yoffset=%d\n", vinfo.xoffset, vinfo.yoffset);
    printf("vinfo.xresX vinfo.yres=%dX%d    finfo.line_length=%d  finfo.smem_len=%d\n", vinfo.xres, vinfo.yres,finfo.line_length/4, finfo.smem_len);  
  
    buffer = (char*)malloc(finfo.smem_len);
    if( buffer != NULL )
    {
        printf("buffer = %p\n", buffer);
    }
    
    ret = read(fd, buffer, finfo.smem_len);
    printf("read %d bytes\n", ret);

    for (int i = 0; i < vinfo.yres; ++i)
    {
        for (int j = 0; j < finfo.line_length/4; ++j)
        {
            buffer[i*finfo.line_length+ j*4 + 3] = 255;
        }
    }

    ret = write(savefb, buffer,  finfo.smem_len);
    printf("write %d bytes\n", ret);

    sleep(1);

    /*释放缓冲区，关闭设备*/
    munmap(fbp, screensize);  
    close(fd);  
    close(savefb);
    return 0;  
}  
