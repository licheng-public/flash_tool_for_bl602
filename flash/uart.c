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
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

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

int uart_open(const char *p_uart_port, uint32_t baud_rate)
{
	int uart_fd;
    struct termios options;
	int ret_status;
	speed_t speed;

    // Open the UART device file
    uart_fd = open(p_uart_port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1) {
        fprintf(stderr, "Unable to open %s", p_uart_port);
        return -1;
    }

    // Configure UART settings
    tcgetattr(uart_fd, &options);

    // Set baud rate
	ret_status = get_baud_rate(baud_rate, &speed);
	if (ret_status < 0) {
		fprintf(stderr, "baud_rate not supported \n");
#if 0
		struct termios2 tio;
		/*
		 * https://www.downtowndougbrown.com/2013/11/linux-custom-serial-baud-rates/
		 */
#include <asm/termios.h>
		ioctl(fd, TCGETS2, &tio);
		tio.c_cflag &= ~CBAUD;
		tio.c_cflag |= BOTHER;
		tio.c_ispeed = baud_rate;
		tio.c_ospeed = baud_rate;
		/* do other miscellaneous setup options with the flags here */
		ioctl(fd, TCSETS2, &tio);
#endif
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

    // Set non-blocking read
    fcntl(uart_fd, F_SETFL, FNDELAY);

fail:
	return uart_fd;
}

int uart_close(int uart_fd)
{
	close(uart_fd);
	return 0;
}
