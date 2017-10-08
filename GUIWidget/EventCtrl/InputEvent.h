#ifndef _INPUTEVENT_H_
#define _INPUTEVENT_H_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <sys/epoll.h>
#include <string.h>


struct epoll_event;


class InputEvent 
{
public:
    InputEvent();
    ~InputEvent();

    bool init();
    void run();
    void stop();
    void (*inputEventDeal)(void *param, int type, int code, int value);
    bool addAepollFile(const char* filename);
    void PostPosEvent();
    void PostClickEvent();



    void *dealParam;

private:
    int m_epfd;
    int m_kdfd;
    int m_msfd;

    void doInput(const epoll_event* ev);
	int scan_dir(const char *dirname);
    bool exitflag;
};


#endif // _INPUTEVENT_H_