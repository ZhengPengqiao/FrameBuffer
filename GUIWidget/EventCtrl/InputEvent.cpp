#include "InputEvent.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <sys/epoll.h>
#include <string.h>
#include <sys/vfs.h>
#include <dirent.h>

#define MAX_EVENTS 20
#define DVE_PATH "/dev/input"
struct epoll_event ev;
struct epoll_event events[MAX_EVENTS];
char buffer[sizeof(input_event)];

InputEvent::InputEvent()
{
    inputEventDeal = NULL;
    exitflag = false;
    dealParam = NULL;
}

InputEvent::~InputEvent()
{

}


int InputEvent::scan_dir(const char *dirname)
{
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);

    *filename++ = '/';
    while((de = readdir(dir))) 
    {
        if(de->d_name[0] == '.' && (de->d_name[1] == '\0' ||
            (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        //将文件添加到epoll中监听中
        addAepollFile(devname);
    }
    closedir(dir);
    return 0;
}


bool InputEvent::addAepollFile(const char* filename)
{
    bool re = false;
    int fd = open(filename, O_RDONLY | O_NONBLOCK);
    if (fd == -1 ) 
    {
        printf("open kd device or mouse fail!\n");
    }
    else 
    {
        ev.data.fd = fd;
        ev.events = EPOLLIN | EPOLLET;
        int i = epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev);
        if (i == -1 ) 
        {
            printf("add %s device in epoll fail!\n", filename);
        }
        else 
        {
            printf("add %s device in epoll sucess!\n", filename);
            re = true;
        }
    }
    return re;
}

bool InputEvent::init()
{

    bool re = true;
    m_epfd = epoll_create(1);
    if (m_epfd == -1) 
    {
        printf("epoll create fail!\n");
    }
    else 
    {
        scan_dir(DVE_PATH);
    }

    return re;

}


void InputEvent::doInput(const epoll_event* ev)
{
    ssize_t resize = 0;
    ssize_t n = 0;
    struct input_event input_ev;
    while ((resize = read(ev->data.fd, buffer + n, sizeof(struct input_event))) > 0) 
    {
        n += resize;
        if (n == sizeof(input_event)) 
        {
             memcpy((void*)(&input_ev), buffer, sizeof(input_event));
             if( inputEventDeal != NULL )
             {
                inputEventDeal( dealParam, (int)input_ev.type, (int)input_ev.code, (int)input_ev.value);
             }

             n = 0;
         }
    }
}


//发送点击事件
void InputEvent::PostClickEvent()
{
    inputEventDeal( dealParam, (int)EV_KEY, 272, 1);
    inputEventDeal( dealParam, (int)EV_KEY, 272, 0);    
}


void InputEvent::PostPosEvent()
{
    inputEventDeal( dealParam, (int)EV_ABS, 24, 29);
}


void InputEvent::run()
{
    int re = 0;
    while (!exitflag) 
    {
        re = epoll_wait(m_epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < re; ++i) 
        {
             if (events[i].events & EPOLLIN) 
             {
                doInput(events + i);
             }
        }
        re = 0;
    }
}


void InputEvent::stop()
{
    exitflag = true; 
}


