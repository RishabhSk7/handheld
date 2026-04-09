#include <gpiod.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define SPI_DEVICE "/dev/spidev0.1"
#define SPI_SPEED 1000000
#define GPIOCHIP_NAME "gpiochip0"
#define GPIO_LINE 17      // RST pin

// PN532 commands
#define PN532_COMMAND_GETFIRMWAREVERSION 0x02
#define PN532_COMMAND_INLISTPASSIVETARGET 0x4A

void spi_transfer(int spi_fd, uint8_t *tx, uint8_t *rx, int len) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .speed_hz = SPI_SPEED,
        .bits_per_word = 8,
    };
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("SPI transfer failed");
    }
}

int main() {
    // --- Reset PN532 ---
    struct gpiod_chip *chip = gpiod_chip_open_by_name(GPIOCHIP_NAME);
    struct gpiod_line *line = gpiod_chip_get_line(chip, GPIO_LINE);
    gpiod_line_request_output(line, "pn532-reset", 0);
    gpiod_line_set_value(line, 0);
    usleep(200000); // 200 ms low
    gpiod_line_set_value(line, 1);
    usleep(50000);  // 50 ms high
    printf("PN532 reset done\n");

    // --- Open SPI ---
    int spi_fd = open(SPI_DEVICE, O_RDWR);
    uint8_t mode = SPI_MODE_0;
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    uint32_t speed = SPI_SPEED;
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    // --- Wakeup + Get Firmware Version ---
    uint8_t wakeup[] = {0x00};  // dummy wakeup byte
    uint8_t fw_cmd[] = {0x00,0x00,0xFF,0x02,0xFE,0xD4,PN532_COMMAND_GETFIRMWAREVERSION,0x2A,0x00};
    uint8_t rx[32] = {0};

    spi_transfer(spi_fd, wakeup, rx, 1);
    spi_transfer(spi_fd, fw_cmd, rx, sizeof(fw_cmd));
    printf("FW response: ");
    for(int i=0;i<sizeof(rx);i++) printf("%02X ", rx[i]);
    printf("\n");

    // --- Poll for cards ---
    while(1) {
        uint8_t poll_cmd[] = {0x00,0x00,0xFF,0x04,0xFC,0xD4,PN532_COMMAND_INLISTPASSIVETARGET,0x01,0x00,0xE1,0x00};
        uint8_t poll_rx[32] = {0};

        spi_transfer(spi_fd, poll_cmd, poll_rx, sizeof(poll_cmd));

        // Check if a card is detected
        if (poll_rx[7] == 0x01) {  // 1 card detected
            printf("Card UID: ");
            for(int i=0;i<poll_rx[12];i++) {  // UID length in poll_rx[12]
                printf("%02X ", poll_rx[13+i]);
            }
            printf("\n");
        } else {
            printf("No card detected\n");
        }
        usleep(500000); // poll every 0.5 sec
    }

    close(spi_fd);
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}
