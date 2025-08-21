/*
 * uart utility
 *
 * Copyright (C) 2025, Liang Cheng
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>

static int get_baud_rate(uint32_t baud_rate, speed_t *speed)
{
    int ret_status = 0;
    switch(baud_rate) {
        case 9600:
            *speed = B9600;
            break;
        case 19200:
            *speed = B19200;
            break;
        case 38400:
            *speed = B38400;
            break;
        case 57600:
            *speed = B57600;
            break;
        case 115200:
            *speed = B115200;
            break;
        case 230400:
            *speed = B230400;
            break;
        default:
            ret_status = -1;
            break;
    }

    return ret_status;
}

#if 0
/*
 * NOTE: checked on Ubuntu 18.04.6, this code seems not work.
 * set the baud rate to customer value, e.g. 200000, and check
 * against with the rate from:
 *    $stty -F /dev/ttyUSB0
 * #include <asm/termbits.h>
 *
 * directly including the asm/termios.h results in compiler error
 * due to conflict definition.
 */
/*
 * https://www.downtowndougbrown.com/2013/11/linux-custom-serial-baud-rates/
 */
int set_custom_baud_rate(int fd, uint32_t custom_baud) {
    struct termios2 tio;

    /* Get current settings */
    if (ioctl(fd, TCGETS2, &tio) < 0) {
        perror("TCGETS2");
        return -1;
    }

    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;       /* Allow custom baud */
    tio.c_ispeed = custom_baud;  /* Input speed */
    tio.c_ospeed = custom_baud;  /* Output speed */

    /* Apply settings */
    if (ioctl(fd, TCSETS2, &tio) < 0) {
        perror("TCSETS2");
        return -1;
    }

    return 0;
}
#endif

int uart_open(const char *p_uart_port, uint32_t baud_rate)
{
    int uart_fd;
    struct termios options;
    int ret_status;
    speed_t speed;

    // Open the UART device file
    uart_fd = open(p_uart_port, O_RDWR);
    if (uart_fd == -1) {
        fprintf(stderr, "ERROR: Unable to open %s", p_uart_port);
        return -1;
    }

    // Configure UART settings
    tcgetattr(uart_fd, &options);

    // Set baud rate
    ret_status = get_baud_rate(baud_rate, &speed);
    if (ret_status < 0) {
        fprintf(stderr, "ERROR: baud_rate not supported \n");
        uart_fd = -2;
        goto fail;
    }
    else {
        cfsetispeed(&options, speed);
        cfsetospeed(&options, speed);
    }

    // 8N1: 8 data bits, no parity, 1 stop bit
    options.c_cflag &= ~PARENB; // No parity
    options.c_cflag &= ~CSTOPB; // 1 stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;     // 8 data bits

    // Raw input mode
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    // Apply the settings
    tcsetattr(uart_fd, TCSANOW, &options);

fail:
    return uart_fd;
}

int uart_close(int uart_fd)
{
    close(uart_fd);
    return 0;
}
