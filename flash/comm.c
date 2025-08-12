/*
 * communication between host and BL 60x
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
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "packet_comm.h"

static void dump_hex(const char *prefix, uint8_t *p_data, uint32_t len) {
    uint32_t i = 0;

    if (prefix != NULL) {
        fprintf(stdout, "%s dump:", prefix);
    }
    for (i = 0; i < len; i++) {
        if ((i % 8) == 0) {
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "0x%02x ", p_data[i]);
    }
    fprintf(stdout, "\n\n");
}

static bool is_ok(uint8_t *result) {
    return (result[0] == 'O' && result[1] == 'K');
}

static bool is_fail(uint8_t *result) {
    return (result[0] == 'F' && result[1] == 'L');
}

static void init_header(COMMAND_ID id, uint32_t len, packet_hdr_t *p_hdr) {
    if (NULL != p_hdr) {
        p_hdr->cmd_id = id;
        p_hdr->rsvd_08 = 0;
        p_hdr->len_lsb = len & 0xFF;
        p_hdr->len_msb = (len & 0xFF00) >> 8;
    }
}

static int read_check_response(int uart_fd, bl_resp_t *p_resp) {
    int ret_code = 0;
    int bytes_n = 0;
    bl_resp_t resp;

    /* wait for 100 miliseconds */
    usleep(100 * 1000);

    memset(&resp, 0, sizeof resp);
    bytes_n = read(uart_fd, (void *)&resp, sizeof resp);
    if (bytes_n <= 0) {
        ret_code = -2;
        fprintf(stderr, "ERROR: fail to read response\n");
        goto fail;
    }

    printf("\nreceived [%d] bytes: %c%c\n", bytes_n,
            resp.result[0], resp.result[1]);
    dump_hex(__FUNCTION__, (uint8_t *)&resp, bytes_n);

    if (is_ok(resp.result)) {
        ret_code = 0;
    } else if (is_fail(resp.result)) {
        /* fail, print out the error code */
        printf("0x%x 0x%x\n", resp.err_msb, resp.err_lsb);
        fprintf(stderr, "ERROR: error code = [0x%04x]\n", (resp.err_msb << 8 | resp.err_lsb));
        ret_code = -3;
        goto fail;
    } else {
        fprintf(stderr, "ERROR: unknown response\n");
        ret_code = -4;
        goto fail;
    }

    if (p_resp != NULL) {
        memcpy(p_resp, &resp, sizeof(*p_resp));
    }
fail:
    return ret_code;
}

/*
 * UART hand shake between the host and the target
 */
int hand_shake(int uart_fd, uint32_t baud_rate)
{
	int i = 0;
	int ret_status = 0;
	uint8_t stream_hfive[512];
    ssize_t bytes_n;
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
			ret_status = -1;
			goto fail;
		}
		left = bytes_n - write_n;
	};

	/* now read from the device */
    ret_status = read_check_response(uart_fd, NULL);
    if (ret_status == 0) {
        fprintf(stdout, "SUCCEED: hand shake\n");
    } else {
        fprintf(stderr, "ERROR: failed in hand shake\n");
    }

fail:
	return ret_status; /* zero is OK */
}

int load_boot_header(int uart_fd, char *eflash_file_name)
{
    int ret_code = 0;
    FILE *f = NULL;
    boot_header_pkt_t boot_header_pkt;

    if (eflash_file_name == NULL) {
        return -1;
    }
    f = fopen(eflash_file_name, "r");
    if (f == NULL) {
        return -2;
    }
    /* construct the packet to device and send */
    init_header(COMMAND_BOOT_HDR, sizeof boot_header_pkt.boot_header,
            &boot_header_pkt.bh_hdr);
    (void) fread(&boot_header_pkt.boot_header, sizeof boot_header_pkt.boot_header,
            1, f);
    fclose(f);
    write(uart_fd, (void *)&boot_header_pkt, sizeof boot_header_pkt);

    ret_code = read_check_response(uart_fd, NULL);

    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: load boot header\n");
    } else {
        fprintf(stderr, "ERROR: failed in loading boot header\n");
    }

    return ret_code;
}

int request_boot_info(int uart_fd, boot_info_t *p_boot_info) {
    int ret_code = 0;
    int bytes_n = 0;
    boot_info_req_t req;
    bl_resp_t resp;

    memset(&req, 0, sizeof req);
    init_header(COMMAND_BOOT_INFO, 0, &req.bi_hdr);
    bytes_n = write(uart_fd, (void *)&req, sizeof req);
    if (bytes_n < sizeof req) {
        ret_code = -1;
        fprintf(stderr, "ERROR: fail to send request boot_info\n");
        goto fail;
    }

    ret_code = read_check_response(uart_fd, &resp);
    if (ret_code == 0) {
        /* sanity check the length field */
        uint32_t len = (resp.len_msb << 8) | (resp.len_lsb);

        if (len != sizeof(boot_info_t)) {
            fprintf(stderr, "ERROR: inavlid payload\n");
            ret_code = -2;
            goto fail;
        }
        fprintf(stdout, "boot_rom_ver: 0x%u\n", resp.boot_info.boot_rom_ver);
        dump_hex("opt_info", (uint8_t *)&resp.boot_info.opt_info[0],
                sizeof(resp.boot_info.opt_info));

        if (p_boot_info != NULL) {
            memcpy(p_boot_info, &resp.boot_info, sizeof(*p_boot_info));
        }
    }

fail:
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: get boot_info\n");
    } else {
        fprintf(stderr, "ERROR: fail to get boot_info\n");
    }
    return ret_code;
}

int load_segment_header(int uart_fd, char *eflash_file_name) {
    int ret_code = 0;
    FILE *f = NULL;
    segment_header_pkt_t segment_header_pkt;

    if (eflash_file_name == NULL) {
        return -1;
    }
    f = fopen(eflash_file_name, "r");
    if (f == NULL) {
        return -2;
    }
    /* construct the packet to device and send */
    init_header(COMMAND_BOOT_HDR, sizeof segment_header_pkt.segment,
            &segment_header_pkt.seg_hdr);
    /* XXX: replace the hard code 176 */
    fseek(f, 176, SEEK_SET);
    (void) fread(&segment_header_pkt.segment, sizeof segment_header_pkt.segment,
            1, f);
    fclose(f);
    write(uart_fd, (void *)&segment_header_pkt, sizeof segment_header_pkt);

    /* check response */
    ret_code = read_check_response(uart_fd, NULL);
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: load segment header\n");
    } else {
        fprintf(stderr, "ERROR: fail to load segement header\n");
    }

    return ret_code;
}

int load_segment_data(int uart_fd, char *eflash_file_name) {
    int ret_code = 0;

    FILE *f = NULL;
    segment_data_pkt_t segment_data_pkt;
    size_t real_len;

    if (eflash_file_name == NULL) {
        return -1;
    }
    f = fopen(eflash_file_name, "r");
    if (f == NULL) {
        return -2;
    }
    /* construct the packet to device and send */
    /* XXX: replace the hard code 176 + 16 */
    fseek(f, 176 + 16, SEEK_SET);

    /* TODO: the binary may exeed the size, how to handle this? */
    real_len = fread(&segment_data_pkt.seg_data[0], 1,
            sizeof segment_data_pkt.seg_data, f);
    fclose(f);

    /* fill the header */
    init_header(COMMAND_BOOT_HDR, real_len, &segment_data_pkt.seg_data_hdr);
    write(uart_fd, (void *)&segment_data_pkt, real_len);

    /* check response */
    ret_code = read_check_response(uart_fd, NULL);
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: load segment header\n");
    } else {
        fprintf(stderr, "ERROR: fail to load segement data\n");
    }

    return ret_code;
}

int check_image(int uart_fd) {
    int ret_code = 0;
    image_check_pkt_t img_check_pkt;

    init_header(COMMAND_IMG_CHECK, 0, &img_check_pkt.img_check_hdr);
    write(uart_fd, &img_check_pkt, sizeof img_check_pkt);

    ret_code = read_check_response(uart_fd, NULL);
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: check image\n");
    } else {
        fprintf(stderr, "ERROR: fail to check image\n");
    }

    return ret_code;
}

int run_image(int uart_fd) {
    int ret_code = 0;
    image_run_pkt_t img_run_pkt;

    init_header(COMMAND_IMG_RUN, 0, &img_run_pkt.img_run_hdr);
    write(uart_fd, &img_run_pkt, sizeof img_run_pkt);

    ret_code = read_check_response(uart_fd, NULL);
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: run image\n");
    } else {
        fprintf(stderr, "ERROR: fail to run image\n");
    }

    return ret_code;
}

int load_pub_key(int uart_fd) {
    int ret_code = 0;

    return ret_code;
}

int load_signature(int uart_fd) {
    int ret_code = 0;

    return ret_code;
}

int load_aes_iv(int uart_fd) {
    int ret_code = 0;

    return ret_code;
}
