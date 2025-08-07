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

    fprintf(stdout, "\n");
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

static void build_header(COMMAND_ID id, uint32_t len, packet_hdr_t *p_hdr) {
    if (NULL != p_hdr) {
        p_hdr->cmd_id = id;
        p_hdr->rsvd_08 = 0;
        p_hdr->len_lsb = len & 0xFF;
        p_hdr->len_msb = (len & 0xFFFF) >> 8;
    }
}

int request_boot_info(int uart_fd) {
    int ret_code = 0;
    int bytes_n = 0;
    boot_info_req_t req;
    bl_resp_t resp;

    build_header(COMMAND_BOOT_INFO, 0, &req.bi_hdr);
    bytes_n = write(uart_fd, (void *)&req, sizeof req);
    if (bytes_n < sizeof req) {
        ret_code = -1;
        fprintf(stderr, "ERROR: fail to send request boot_info\n");
        goto fail;
    }
    /* wait for 100 miliseconds */
    usleep(100 * 1000);

    memset(&resp, 0, sizeof resp);
    bytes_n = read(uart_fd, (void *)&resp, sizeof resp);
    if (bytes_n <= 0) {
        ret_code = -2;
        fprintf(stderr, "ERROR: fail to read boot_info\n");
        goto fail;
    }

    printf("received [%d] bytes\n", bytes_n);
    dump_hex(__FUNCTION__, (uint8_t *)&resp, bytes_n);

    fprintf(stdout, "%c%c\n", resp.result[0], resp.result[1]);
    if (is_ok(resp.result)) {
        /* sanity check the length field */
        uint32_t len = (resp.len_msb << 8) | (resp.len_lsb);

        printf("msb = 0x%x lsb = 0x%x\n", resp.len_msb, resp.len_lsb);
        if (len != sizeof(boot_info_t)) {
            fprintf(stderr, "ERROR: inavlid payload\n");
            ret_code = -2;
            goto fail;
        }
        fprintf(stdout, "boot_rom_ver: 0x%u\n", resp.boot_info.boot_rom_ver);
        dump_hex("opt_info", (uint8_t *)&resp.boot_info.opt_info[0],
                sizeof(resp.boot_info.opt_info));

    } else if (is_fail(resp.result)) {
        /* fail, print out the error code */
        fprintf(stderr, "ERROR: error code = [0x%u]\n", (resp.err_msb << 8 | resp.err_lsb));
        ret_code = -3;
        goto fail;
    } else {
        fprintf(stderr, "ERROR: unknown response\n");
        ret_code = -4;
        goto fail;
    }

    fprintf(stdout, "SUCCEED: get boot_info\n");

fail:
    return ret_code;
}

int load_boot_header(int uart_fd) {
    int ret_code = 0;

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

int load_segment_header(int uart_fd) {
    int ret_code = 0;

    return ret_code;
}

int load_segment_data(int uart_fd) {
    int ret_code = 0;

    return ret_code;
}

int check_image(int uart_fd) {
    int ret_code = 0;

    return ret_code;
}

int run_image(int uart_fd) {
    int ret_code = 0;

    return ret_code;
}
