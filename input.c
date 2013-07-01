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
#define HELPTEXT "usage: \
    input-report help; print this message \
    input-report event-file \
    event-file content like this: \
    mouse rel_x rel_y \
    key keycode \
    touch abs_x abs_y \
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
    for(index = 0; index < 99; index++)
        ioctl(keysFd, UI_SET_KEYBIT, hisKeycode[index]);
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

    printf("sendKeys down\n");
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = 0;
    if(write(keysFd, &ev, sizeof(struct input_event)) < 0)
        die("error: write");
    
    printf("sendKeys up\n");
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

    printf("linesize is %d\n", lineSize);
    for(i = 0; i<lineSize; i++){
        if(eventBuf[i] != ' ') {
            eventTypePos = i;
            break;
        }
    }

    printf("eventTypePos is %d\n", eventTypePos);
    for(i = eventTypePos; i<lineSize; i++){
        if( eventBuf[i] != ' ' )
            eventType[i-eventTypePos] = eventBuf[i];
        else {
            eventType[i] = NULL;
            break;
        }
    }
    printf("eventType is %s\n", eventType);

    for(; i<lineSize; i++){
        if(eventBuf[i] != ' ') {
            eventParaXPos = i;
            break;
        }
    }

    printf("eventParaXPos is %d\n", eventParaXPos);
    for(i = eventParaXPos; i<lineSize; i++){
        if( eventBuf[i] != ' ' )
            eventParaX[i-eventParaXPos] = eventBuf[i];
        else {
            eventParaX[i-eventParaXPos] = NULL;
            break;
        }
    }

    printf("eventParaX is %s\n", eventParaX);
    for(; i<lineSize; i++) {
        if(eventBuf[i] != ' ') {
            eventParaYPos = i;
            break;
        }
    }

    printf("eventParaYPos is %d\n", eventParaYPos);
    for(i = eventParaYPos; i<lineSize; i++){
        if( eventBuf[i] != ' ' )
            eventParaY[i-eventParaYPos] = eventBuf[i];
        else {
            eventParaY[i-eventParaYPos] = NULL;
            break;
        }
    }
    printf("eventParaY is %s\n", eventParaY);
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

    printf("argc is %d\n", argc);
    printf("dataFile path is %s\n", argv[1]);

    fd = openInputDev(DEVPATH);

    dataFile = fopen(argv[1], "r");
    if(dataFile == NULL)
        die("open data file");
    while(fgets(eventBuf, LINESIZE, dataFile) != NULL) { 
        printf("%s\n", eventBuf);
        getEventInfo(eventBuf, LINESIZE, eventType, eventParaX, eventParaY);

        if(strlen(eventType) != 0) {
            if(!strcmp(eventType, "key")) {
                printf("in key event\n");
                printf("eventParaX is %d\n", atoi(eventParaX));
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
    //srand(time(NULL));

    return 0;
}
