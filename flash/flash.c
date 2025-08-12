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
#include "common_share.h"
#include "packet_comm.h"

void print_help(const char *p_app_name)
{
    printf("USAGE: %s --uart uart_device --rate baud_rate --partition part1.bin part2.bin"
            "  --fw firmware.bin --dtb ro_param.dtb --eflash eflash_loader\n", p_app_name);
	return;
}

/*
 * The usage 
 * ./flash --uart uart_device --rate baud_rate --partition part1.bin part2.bin
 *   --fw firmware.bin --dtb ro_param.dtb
 */
int main(int argc, char *argv[])
{
	int ret_code;
    int uart_fd = -1;
	uint32_t baud_rate;
    boot_info_t boot_info;
    int i = 1;
    int j = 0;
	char *p_uart_port;
    char *fw_file = NULL;
    char *eflash_loader_file = NULL;
    char *dtb_file = NULL;
    char *p_part[4] = {NULL, NULL, NULL, NULL};

	if (argc < 11) {
        fprintf(stderr, "ERROR: missing operand\n");
		print_help(argv[0]);
		return -1;
	}

    i = 1;
#define CHECK_BOUND {\
    if (++i >= argc || (argv[i][0] == '-' && argv[i][1] == '-')) { \
        fprintf(stderr, "ERROR: missing an argument for %s\n", argv[i-1]);\
        goto fail2;\
    }\
}

    while (i < argc) {
        if (strcmp(argv[i], "--uart") == 0) {
            CHECK_BOUND;
            p_uart_port = argv[i++];
        } else if (strcmp(argv[i], "--rate") == 0) {
            CHECK_BOUND;
	        baud_rate = atoi(argv[i++]);
        } else if (strcmp(argv[i], "--fw") == 0) {
            CHECK_BOUND;
            fw_file = argv[i++];
        } else if (strcmp(argv[i], "--eflash") == 0) {
            CHECK_BOUND;
            eflash_loader_file = argv[i++];
        } else if (strcmp(argv[i], "--dtb") == 0) {
            CHECK_BOUND;
            dtb_file = argv[i++];
        } else if (strcmp(argv[i], "--partition") == 0) {
            j = i + 1;
            while (j < argc && argv[j][0] != '-' && argv[j][1] != '=') {
                if ( j - i - 1 < ARRAY_SIZE(p_part)) {
                    p_part[j -i -1] = argv[j];
                }
                j++;
            }
            i = j;
        } else {
            fprintf(stderr, "ERROR: unkwown options [%s]\n", argv[i]);
            return -2;
        }
    }

    /* check arguments */
    if (p_uart_port == NULL || dtb_file == NULL || fw_file == NULL
            || p_part[0] == NULL || eflash_loader_file == NULL) {
        fprintf(stderr, "ERROR: missing arguments for flashing\n");
        goto fail2;
    }

	uart_fd = uart_open(p_uart_port, baud_rate);
	if (uart_fd < 0) {
		fprintf(stderr, "ERROR: failed to open UART\n");
		return -2;
	}

#define CHECK_ERROR(ret_code)  {\
    if (0 != (ret_code)) {      \
        goto fail;              \
    }                           \
}

	ret_code = hand_shake(uart_fd, baud_rate);
    CHECK_ERROR(ret_code);

	/* connection is established now */
	(void) usleep(10 * 1000);
	/* read_boot_info */
    ret_code = request_boot_info(uart_fd, &boot_info);
    CHECK_ERROR(ret_code);

    /*
     * Before flashing the images, eflash image has to program to device. eflash is the
     * program executed on device to handle all flash operations, including erase, program,
     * etc.
     *
     * Though the protocol doc says eflash is not signed and encrypted in Chapter 2, the
     * below source code is still programed based on protocol in Chapter 1 for completenecess.
     *
     * Also note: this boot header might be different from the one generated from
     * efuse_bootheader_cfg.conf, just in case you are curious.
     */
	/* load_boot_header */
    ret_code = load_boot_header(uart_fd, eflash_loader_file);
    CHECK_ERROR(ret_code);

    if (boot_info.sign != 0) {
	    /* if signed, load_public_key */
        ret_code = load_pub_key(uart_fd);
        CHECK_ERROR(ret_code);

	    /* if signed, load signature */
        ret_code = load_signature(uart_fd);
        CHECK_ERROR(ret_code);
    }

    if (boot_info.encrypted) {
	    /* if encrypted, load AES IV */
        ret_code = load_aes_iv(uart_fd);
        CHECK_ERROR(ret_code);
    }

	/* load segment header */
    ret_code = load_segment_header(uart_fd, eflash_loader_file);
    CHECK_ERROR(ret_code);

	/* load segment data */
    ret_code = load_segment_data(uart_fd, eflash_loader_file);
    CHECK_ERROR(ret_code);

	/* check image */
    ret_code = check_image(uart_fd);
    CHECK_ERROR(ret_code);

	/* run image */
    ret_code = run_image(uart_fd);
    CHECK_ERROR(ret_code);
    /*
     * At this point, the eflash image should be running, and ready to serve the flashing
     * jobs. Shake hands to make sure it is OK.
     */
    ret_code = hand_shake(uart_fd, baud_rate);
    CHECK_ERROR(ret_code);

    /* TODO: fill the rest */

fail:
    /* Close UART */
    uart_close(uart_fd);

fail2:
	return ret_code;
}
