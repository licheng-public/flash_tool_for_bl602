/*
 * flash BL 60x
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "uart.h"
#include "comm.h"

#define COMM_OK		0
#define COMM_FAIL	1

/*
 * UART hand shake between the host and the target
 */
uint32_t hand_shake(int uart_fd, uint32_t baud_rate)
{
    char read_buf[256];
	int i = 0;
    ssize_t bytes_n;
	uint32_t ret_status = COMM_OK;
	uint8_t stream_hfive[512];
#if 1
	ssize_t left;
	ssize_t write_n;

	/* 
	 * approximate the number of bytes of 0x55 to send in 5 mseconds
	 * using the current baud rate with 8N1
	 */
	bytes_n = 5 * (baud_rate * 10 / 1000 / 8 + 1);

	if (bytes_n <= sizeof stream_hfive) {
		left = sizeof stream_hfive;
	}
	else {
		left = bytes_n;
	}
	for (i = 0; i < sizeof left; i++) {
		stream_hfive[i] = 0x55;
	}

	while (left > 0) {
		write_n = write(uart_fd, stream_hfive, left);
		if (write_n <= 0) {
			fprintf(stderr, "error in write\n");
			ret_status = COMM_FAIL;
			goto fail;
		}
		left = bytes_n - write_n;
	};
#else
	uint64_t elapse = 0;
	/* send a stream of 0x55 to device for 5 msec */
	gettimeofday(&start_time, NULL);
	do {
		write(uart_fd, stream_hfive, 1);
		gettimeofday(&curr_time, NULL);
		elapse = (curr_time.tv_sec - start_time.tv_sec) * 1000 * 1000 +
			(curr_time.tv_usec - start_time.tv_usec);
	} while(elapse < 5 * 10000);
#endif

	/* now read from the device */
    while (1) {
        usleep(100000); // Sleep for 100ms
        memset(read_buf, 0, sizeof(read_buf));
        bytes_n = read(uart_fd, read_buf, sizeof(read_buf));
        if (bytes_n > 0) {
            printf("Received (%ld bytes): %s\n", bytes_n, read_buf);
			/* check the result: "OK", "FL" */
			if (read_buf[0] == 'O' && read_buf[1] == 'K') {
				printf("hand shake succeeds\n");
				break;
			}
			else if (read_buf[0] == 'F' && read_buf[1] == 'L') {
				fprintf(stderr, "fail in comm\n");
				ret_status = COMM_FAIL;
				break;
			}
			else {
				fprintf(stderr, "Unknown result\n");
				ret_status = COMM_FAIL;
				break;
			}
        } /* bytes_n */
    }

fail:
	return ret_status; /* zero is OK */
}

void print_help(const char *p_app_name)
{
	printf("USAGE: %s uart_port baud_rate\n", p_app_name);
	return;
}

int main(int argc, char *argv[])
{
	const char *p_uart_port;
	int ret_code;
    int uart_fd;
	uint32_t baud_rate;

	if (argc < 3) {
		print_help(argv[0]);
		return -1;
	}
	p_uart_port = argv[1];
	baud_rate = atoi(argv[2]);

	uart_fd = uart_open(p_uart_port, baud_rate);
	if (uart_fd < 0) {
		fprintf(stderr, "failed to open UART\n");
		return -2;
	}

	if (COMM_OK != hand_shake(uart_fd, baud_rate)) {
		fprintf(stderr, "failed in talk to BL602\n");
		ret_code = -3;
		goto fail;
	}

	/* connection is established now */
	(void) usleep(10 * 1000);
	/* read_boot_info */
    ret_code = request_boot_info(uart_fd);
    if (0 != ret_code) {
        goto fail;
    }

	/* load_boot_header */

	/* if signed, load_public_key */

	/* if signed, load signature */

	/* if encrypted, load AES IV */

	/* load segment header */
	/* load segment data */

	/* check image */

	/* run image */

    // Close UART
fail:
    uart_close(uart_fd);
	return ret_code;
}
