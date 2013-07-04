/*
 * Copyright (C) 2013 Hisense Co.,LTD
 *
 * Author wangbaoyun@hisense.com
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include "keys.h"

#define die(str, args...) do { \
        perror(str); \
        exit(EXIT_FAILURE); \
    } while(0)

#define DEVPATH "/dev/uinput"
#define LINESIZE 32
#define WORDSIZE 8
#define HELPTEXT "usage: \n \
    input help; print this message \n \
    input event-file interval \n \
    event-file content like this: \n \
    mouse rel_x rel_y \n \
    key keycode \n \
    touch abs_x abs_y \n \
    interval like 1 that represents 1 second between two event \n \
    "

int openInputDev(char *devPath)
{
    struct uinput_user_dev uidev;
    
    int fd = open(devPath, O_WRONLY | O_NONBLOCK);
    if(fd < 0)
        die("error: open");

    return fd;
}

void createUinputDev(int fd)
{
    struct uinput_user_dev uidev;
    
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-dev");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    if(write(fd, &uidev, sizeof(uidev)) < 0)
        die("error: write");

    if(ioctl(fd, UI_DEV_CREATE) < 0)
        die("error: ioctl");

    sleep(5);
}

void destroyUinputDev(int fd)
{
    if(ioctl(fd, UI_DEV_DESTROY) < 0)
        die("error: ioctl");
}

void closeInputDev(int fd)
{
    close(fd);
}

void setMouseParameter(int mouseFd)
{
    if(ioctl(mouseFd, UI_SET_EVBIT, EV_REL) < 0)
        die("error: ioctl");
    if(ioctl(mouseFd, UI_SET_RELBIT, REL_X) < 0)
        die("error: ioctl");
    if(ioctl(mouseFd, UI_SET_RELBIT, REL_Y) < 0)
        die("error: ioctl");
}

void setKeysParameter(int keysFd)
{
    int index; 

    if(ioctl(keysFd, UI_SET_EVBIT, EV_KEY) < 0)
        die("error: ioctl");
    if(ioctl(keysFd, UI_SET_KEYBIT, BTN_LEFT)<0)
        die("error: ioctl");
    if(ioctl(keysFd, UI_SET_KEYBIT, BTN_TOUCH)<0)
        die("error: ioctl");
    for(index = 0; index < 99; index++)
        if(ioctl(keysFd, UI_SET_KEYBIT, hisKeycode[index])<0)
            die("error: ioctl");
}

void setTouchParameter(int touchFd)
{
    if(ioctl(touchFd, UI_SET_EVBIT, EV_ABS) < 0)
        die("error: ioctl");
    if(ioctl(touchFd, UI_SET_ABSBIT, ABS_X) < 0)
        die("error: ioctl");
    if(ioctl(touchFd, UI_SET_ABSBIT, ABS_Y) < 0)
        die("error: ioctl");

}

void sendKeysEvent(int keysFd, int keycode)
{
    struct input_event ev;

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = 1;
    if(write(keysFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = 0;
    if(write(keysFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;
    if(write(keysFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");
}

void sendMouseEvent(int mouseFd, int dx, int dy)
{
    struct input_event ev;

    printf("mouse event: %d, %d\n", dx, dy);
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_REL;
    ev.code = REL_X;
    ev.value = dx;
    if(write(mouseFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_REL;
    ev.code = REL_Y;
    ev.value = dy;
    if(write(mouseFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;
    if(write(mouseFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");
}

void sendTouchEvent(int touchFd, int x, int y)
{
    struct input_event ev;
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_KEY;
    ev.code = BTN_TOUCH;
    ev.value = 0;
    if(write(touchFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_ABS;
    ev.code = ABS_X;
    ev.value = x;
    if(write(touchFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_ABS;
    ev.code = ABS_Y;
    ev.value = y;
    if(write(touchFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_KEY;
    ev.code = BTN_TOUCH;
    ev.value = 1;
    if(write(touchFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;
    if(write(touchFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");
}

void getEventInfo(char eventBuf[], int lineSize, char eventType[], char eventParaX[], char eventParaY[])
{
    int eventTypePos = 0;
    int eventParaXPos = 0;
    int eventParaYPos = 0;
    int i = 0;

    for(i = 0; i<lineSize; i++){
        if(eventBuf[i] != ' ') {
            eventTypePos = i;
            break;
        }
    }

    for(i = eventTypePos; i<lineSize; i++){
        if( eventBuf[i] != ' ' )
            eventType[i-eventTypePos] = eventBuf[i];
        else {
            eventType[i] = NULL;
            break;
        }
    }

    for(; i<lineSize; i++){
        if(eventBuf[i] != ' ') {
            eventParaXPos = i;
            break;
        }
    }

    for(i = eventParaXPos; i<lineSize; i++){
        if( eventBuf[i] != ' ' )
            eventParaX[i-eventParaXPos] = eventBuf[i];
        else {
            eventParaX[i-eventParaXPos] = NULL;
            break;
        }
    }

    for(; i<lineSize; i++) {
        if(eventBuf[i] != ' ') {
            eventParaYPos = i;
            break;
        }
    }

    for(i = eventParaYPos; i<lineSize; i++){
        if( eventBuf[i] != ' ' )
            eventParaY[i-eventParaYPos] = eventBuf[i];
        else {
            eventParaY[i-eventParaYPos] = NULL;
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    int fd = 0;
    FILE * dataFile = NULL;
    char eventBuf[LINESIZE];
    char eventType[WORDSIZE];
    char eventParaX[WORDSIZE];
    char eventParaY[WORDSIZE];

    if(argc == 1) {
        printf("%s", HELPTEXT);
    }

    fd = openInputDev(DEVPATH);

    setKeysParameter(fd);
    setMouseParameter(fd);
    setTouchParameter(fd);

    createUinputDev(fd);

    dataFile = fopen(argv[1], "r");
    if(dataFile == NULL)
        die("open data file");
    while(fgets(eventBuf, LINESIZE, dataFile) != NULL) { 
        getEventInfo(eventBuf, strlen(eventBuf), eventType, eventParaX, eventParaY);

        if(strlen(eventType) != 0) {
            if(!strcmp(eventType, "key")) {
                if(strlen(eventParaX) != 0)
                    sendKeysEvent(fd, atoi(eventParaX));
            } 
            else if(!strcmp(eventType, "mouse")) {
                if((strlen(eventParaX) != 0)&&(strlen(eventParaY) != 0))
                    sendMouseEvent(fd, atoi(eventParaX), atoi(eventParaY));
            }
            else if(!strcmp(eventType, "touch")) {
                if((strlen(eventParaX) != 0)&&(strlen(eventParaY) != 0))
                    sendTouchEvent(fd, atoi(eventParaX), atoi(eventParaY));
            }
            else
                printf("unkown input device type\n");
        }

        sleep(atoi(argv[2]));
    }

    fclose(dataFile);
    destroyUinputDev(fd);
    closeInputDev(fd);

    return 0;
}
