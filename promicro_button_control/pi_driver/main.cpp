#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/uinput.h>
#include <cstring>
#include <vector>
#include <cstdint>

// ---------- UART SETUP ----------
int setup_uart(const char* device) {
    int fd = open(device, O_RDONLY | O_NOCTTY);

    if (fd < 0) {
        perror("UART open failed");
        exit(1);
    }

    struct termios tty{};
    tcgetattr(fd, &tty);

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;

    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;

    tcsetattr(fd, TCSANOW, &tty);

    return fd;
}

// ---------- UINPUT ----------
void emit(int fd, int type, int code, int val) {
    struct input_event ev{};
    ev.type = type;
    ev.code = code;
    ev.value = val;
    write(fd, &ev, sizeof(ev));
}

int setup_uinput() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open /dev/uinput");
        exit(1);
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_ABS);

    ioctl(fd, UI_SET_KEYBIT, BTN_SOUTH);
    ioctl(fd, UI_SET_KEYBIT, BTN_EAST);
    ioctl(fd, UI_SET_KEYBIT, BTN_NORTH);
    ioctl(fd, UI_SET_KEYBIT, BTN_WEST);
    ioctl(fd, UI_SET_KEYBIT, BTN_START);
    ioctl(fd, UI_SET_KEYBIT, BTN_SELECT);
    ioctl(fd, UI_SET_KEYBIT, BTN_MODE);

    ioctl(fd, UI_SET_ABSBIT, ABS_HAT0X);
    ioctl(fd, UI_SET_ABSBIT, ABS_HAT0Y);

    struct uinput_abs_setup abs{};
    abs.code = ABS_HAT0X;
    abs.absinfo.minimum = -1;
    abs.absinfo.maximum = 1;
    abs.absinfo.flat = 0;
    ioctl(fd, UI_ABS_SETUP, &abs);

    abs.code = ABS_HAT0Y;
    abs.absinfo.minimum = -1;
    abs.absinfo.maximum = 1;
    abs.absinfo.flat = 0;
    ioctl(fd, UI_ABS_SETUP, &abs);

    struct uinput_setup usetup{};
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 0x1234;
    usetup.id.product = 0x5678;
    usetup.id.version = 1;
    strcpy(usetup.name, "NiceNano Controller");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    sleep(1);
    return fd;
}


void print_state(uint16_t state) {
    std::cout << "Pressed: ";

    if (state & 1)   std::cout << "LEFT ";
    if (state & 2)   std::cout << "UP ";
    if (state & 4)   std::cout << "DOWN ";
    if (state & 8)   std::cout << "RIGHT ";

    if (state & 16)  std::cout << "X ";
    if (state & 32)  std::cout << "Y ";
    if (state & 64)  std::cout << "A ";
    if (state & 128) std::cout << "B ";

    if (state & 256) std::cout << "SELECT ";
    if (state & 512) std::cout << "START ";

    if ((state & 1792) == 1792) std::cout << "HOME ";

    if (state == 0) std::cout << "NONE";

    std::cout << std::endl;
}

// ---------- MAIN ----------
int main() {
    int serial_fd = setup_uart("/dev/ttyAMA0");
    int uinput_fd = setup_uinput();

    uint16_t state = 0;
    uint16_t last_state = 0;

    while (true) {
        uint8_t buf[2];

        int n = read(serial_fd, buf, 2);
        if (n != 2) continue;

        state = buf[0] | (buf[1] << 8);

        if (state == last_state) continue;

        // ---------- Decode ----------
        bool left   = state & 1;
        bool up     = state & 2;
        bool down   = state & 4;
        bool right  = state & 8;

        bool x_btn  = state & 16;
        bool y_btn  = state & 32;
        bool a_btn  = state & 64;
        bool b_btn  = state & 128;

        bool select = state & 256;
        bool start  = state & 512;
        bool home   = (state & 1792) == 1792;

        // ---------- DEBUG PRINT ----------
        std::cout << "Pressed: ";

        if (left)   std::cout << "LEFT ";
        if (right)  std::cout << "RIGHT ";
        if (up)     std::cout << "UP ";
        if (down)   std::cout << "DOWN ";
        if (a_btn)  std::cout << "A ";
        if (b_btn)  std::cout << "B ";
        if (x_btn)  std::cout << "X ";
        if (y_btn)  std::cout << "Y ";
        if (start)  std::cout << "START ";
        if (select) std::cout << "SELECT ";
        if (home)   std::cout << "HOME ";

        if (state == 0) std::cout << "NONE";

        std::cout << std::endl;

        // ---------- D-PAD ----------
        int dx = 0, dy = 0;
        if (left)  dx = -1;
        if (right) dx = 1;
        if (up)    dy = -1;
        if (down)  dy = 1;

        emit(uinput_fd, EV_ABS, ABS_HAT0X, dx);
        emit(uinput_fd, EV_ABS, ABS_HAT0Y, dy);

        // ---------- BUTTONS ----------
        emit(uinput_fd, EV_KEY, BTN_SOUTH, a_btn);
        emit(uinput_fd, EV_KEY, BTN_EAST,  b_btn);
        emit(uinput_fd, EV_KEY, BTN_NORTH, x_btn);
        emit(uinput_fd, EV_KEY, BTN_WEST,  y_btn);

        emit(uinput_fd, EV_KEY, BTN_START, start);
        emit(uinput_fd, EV_KEY, BTN_SELECT, select);
        emit(uinput_fd, EV_KEY, BTN_MODE, home);

        emit(uinput_fd, EV_SYN, SYN_REPORT, 0);

        last_state = state;
    }

    close(serial_fd);
    close(uinput_fd);
}