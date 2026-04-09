#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <gpiod.h>
#include <string.h>

#define CHIP "gpiochip0"

#define UP 17
#define DOWN 27
#define LEFT 22
#define RIGHT 23

void emit(int fd, int type, int code, int val)
{
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;

    write(fd, &ie, sizeof(ie));
}

int main()
{
    struct gpiod_chip *chip;
    struct gpiod_line *up, *down, *left, *right;

    int fd;

    chip = gpiod_chip_open_by_name(CHIP);

    up = gpiod_chip_get_line(chip, UP);
    down = gpiod_chip_get_line(chip, DOWN);
    left = gpiod_chip_get_line(chip, LEFT);
    right = gpiod_chip_get_line(chip, RIGHT);

    gpiod_line_request_input_flags(up,"btn",GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP);
    gpiod_line_request_input_flags(down,"btn",GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP);
    gpiod_line_request_input_flags(left,"btn",GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP);
    gpiod_line_request_input_flags(right,"btn",GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP);

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, KEY_UP);
    ioctl(fd, UI_SET_KEYBIT, KEY_DOWN);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFT);
    ioctl(fd, UI_SET_KEYBIT, KEY_RIGHT);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));

    snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "GPIO Gamepad");
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    usetup.id.version = 1;

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    sleep(1);

    int last_u=1,last_d=1,last_l=1,last_r=1;

    while (1)
    {
        int u = gpiod_line_get_value(up);
        int d = gpiod_line_get_value(down);
        int l = gpiod_line_get_value(left);
        int r = gpiod_line_get_value(right);

        if(u != last_u){
            emit(fd, EV_KEY, KEY_UP, !u);
            emit(fd, EV_SYN, SYN_REPORT, 0);
            last_u = u;
        }

        if(d != last_d){
            emit(fd, EV_KEY, KEY_DOWN, !d);
            emit(fd, EV_SYN, SYN_REPORT, 0);
            last_d = d;
        }

        if(l != last_l){
            emit(fd, EV_KEY, KEY_LEFT, !l);
            emit(fd, EV_SYN, SYN_REPORT, 0);
            last_l = l;
        }

        if(r != last_r){
            emit(fd, EV_KEY, KEY_RIGHT, !r);
            emit(fd, EV_SYN, SYN_REPORT, 0);
            last_r = r;
        }

        usleep(5000);
    }

    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}
