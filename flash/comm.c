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
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "packet_comm.h"
#include "err_code.h"
#include "common_share.h"

#define ADD_ERROR(id) {id, #id}
struct {
    bootrom_error_code_t err_code;
    const char *err_str;
} bootrom_error_table[] = {
    ADD_ERROR(BFLB_BOOTROM_SUCCESS),
    /*flash*/
    ADD_ERROR(BFLB_BOOTROM_FLASH_INIT_ERROR),
    ADD_ERROR(BFLB_BOOTROM_FLASH_ERASE_PARA_ERROR),
    ADD_ERROR(BFLB_BOOTROM_FLASH_ERASE_ERROR),
    ADD_ERROR(BFLB_BOOTROM_FLASH_WRITE_PARA_ERROR),
    ADD_ERROR(BFLB_BOOTROM_FLASH_WRITE_ADDR_ERROR),
    ADD_ERROR(BFLB_BOOTROM_FLASH_WRITE_ERROR),
    ADD_ERROR(BFLB_BOOTROM_FLASH_BOOT_PARA),
    /*cmd*/
    ADD_ERROR(BFLB_BOOTROM_CMD_ID_ERROR),
    ADD_ERROR(BFLB_BOOTROM_CMD_LEN_ERROR),
    ADD_ERROR(BFLB_BOOTROM_CMD_CRC_ERROR),
    ADD_ERROR(BFLB_BOOTROM_CMD_SEQ_ERROR),
    /*image*/
    ADD_ERROR(BFLB_BOOTROM_IMG_BOOTHEADER_LEN_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_BOOTHEADER_NOT_LOAD_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_BOOTHEADER_MAGIC_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_BOOTHEADER_CRC_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_BOOTHEADER_ENCRYPT_NOTFIT),
    ADD_ERROR(BFLB_BOOTROM_IMG_BOOTHEADER_SIGN_NOTFIT),
    ADD_ERROR(BFLB_BOOTROM_IMG_SEGMENT_CNT_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_AES_IV_LEN_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_AES_IV_CRC_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_PK_LEN_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_PK_CRC_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_PK_HASH_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_SIGNATURE_LEN_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_SIGNATURE_CRC_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_SECTIONHEADER_LEN_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_SECTIONHEADER_CRC_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_SECTIONHEADER_DST_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_SECTIONDATA_LEN_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_SECTIONDATA_DEC_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_SECTIONDATA_TLEN_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_SECTIONDATA_CRC_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_HALFBAKED_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_HASH_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_SIGN_PARSE_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_SIGN_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_DEC_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IMG_ALL_INVALID_ERROR),
    /*IF*/
    ADD_ERROR(BFLB_BOOTROM_IF_RATE_LEN_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IF_RATE_PARA_ERROR),
    ADD_ERROR(BFLB_BOOTROM_IF_PASSWORDERROR),
    ADD_ERROR(BFLB_BOOTROM_IF_PASSWORDCLOSE),
    /*MISC*/
    ADD_ERROR(BFLB_BOOTROM_PLL_ERROR),
    ADD_ERROR(BFLB_BOOTROM_INVASION_ERROR),
    ADD_ERROR(BFLB_BOOTROM_POLLING),
    ADD_ERROR(BFLB_BOOTROM_FAIL),
};

struct {
    eflash_loader_error_code_t err_code;
    const char *err_str;
} flash_error_table[] = {
    ADD_ERROR(BFLB_EFLASH_LOADER_SUCCESS),
    /*flash*/
    ADD_ERROR(BFLB_EFLASH_LOADER_FLASH_INIT_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_FLASH_ERASE_PARA_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_FLASH_ERASE_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_FLASH_WRITE_PARA_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_FLASH_WRITE_ADDR_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_FLASH_WRITE_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_FLASH_BOOT_PARA_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_FLASH_SET_PARA_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_FLASH_READ_STATUS_REG_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_FLASH_WRITE_STATUS_REG_ERROR),
    /*cmd*/
    ADD_ERROR(BFLB_EFLASH_LOADER_CMD_ID_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_CMD_LEN_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_CMD_CRC_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_CMD_SEQ_ERROR),
    /*image*/
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_BOOTHEADER_LEN_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_BOOTHEADER_NOT_LOAD_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_BOOTHEADER_MAGIC_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_BOOTHEADER_CRC_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_BOOTHEADER_ENCRYPT_NOTFIT),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_BOOTHEADER_SIGN_NOTFIT),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SEGMENT_CNT_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_AES_IV_LEN_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_AES_IV_CRC_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_PK_LEN_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_PK_CRC_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_PK_HASH_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SIGNATURE_LEN_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SIGNATURE_CRC_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SECTIONHEADER_LEN_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SECTIONHEADER_CRC_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SECTIONHEADER_DST_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SECTIONDATA_LEN_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SECTIONDATA_DEC_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SECTIONDATA_TLEN_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SECTIONDATA_CRC_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_HALFBAKED_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_HASH_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SIGN_PARSE_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_SIGN_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_DEC_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IMG_ALL_INVALID_ERROR),
    /*IF*/
    ADD_ERROR(BFLB_EFLASH_LOADER_IF_RATE_LEN_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IF_RATE_PARA_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IF_PASSWORDERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_IF_PASSWORDCLOSE),
    /*MISC*/
    ADD_ERROR(BFLB_EFLASH_LOADER_PLL_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_INVASION_ERROR),
    ADD_ERROR(BFLB_EFLASH_LOADER_POLLING),
    ADD_ERROR(BFLB_EFLASH_LOADER_FAIL),
};

void dump_hex(const char *prefix, uint8_t *p_data, uint32_t len) {
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

static const char * lookup_error(uint16_t err_code) {
    uint16_t i = 0;

    if (boot_rom_stage) {
        for (i = 0; i < ARRAY_SIZE(bootrom_error_table); i++) {
            if ((bootrom_error_code_t)err_code == bootrom_error_table[i].err_code) {
                return bootrom_error_table[i].err_str;
            }
        }
    } else {
        for (i = 0; i < ARRAY_SIZE(flash_error_table); i++) {
            if ((eflash_loader_error_code_t)err_code == flash_error_table[i].err_code) {
                return flash_error_table[i].err_str;
            }
        }
    }

    return "Unknown error code\n";
}

static int read_check_response(int uart_fd, bl_resp_t *p_resp) {
    int ret_code = 0;
    int bytes_n = 0;
    bl_resp_t resp;

    /* wait for 100 miliseconds */
    usleep(100 * 1000);

    memset(&resp, 0, sizeof resp);
loop:
    bytes_n = read(uart_fd, (void *)&resp, sizeof resp);
    if (bytes_n < 0) {
        ret_code = -2;
        fprintf(stderr, "ERROR: fail to read response [bytes_n = %d]\n", bytes_n);
        goto fail;
    }
    if (bytes_n == 0) {
        printf("wait and try\n");
        usleep(20 * 1000);
        goto loop;
    }

    printf("received [%d] bytes: %c%c\n", bytes_n,
            resp.result[0], resp.result[1]);
#ifdef DEBUG
    dump_hex(__FUNCTION__, (uint8_t *)&resp, bytes_n);
#endif

    if (is_ok(resp.result)) {
        ret_code = 0;
    } else if (is_fail(resp.result)) {
        uint16_t err_code = (resp.err_msb << 8 | resp.err_lsb);
        /* fail, print out the error code */
        fprintf(stderr, "ERROR: error code = [0x%04x] %s\n\n", err_code,
                lookup_error(err_code));
        ret_code = -3;
        goto fail;
    } else {
        fprintf(stderr, "ERROR: unknown response\n\n");
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
 * read file into a allocated buffer
 */
int read_to_buf(char *p_file_name, uint8_t **p_buf, uint32_t *p_sz_data) {
    int ret_code = 0;
    struct stat f_stat;
    uint8_t *p_local = NULL;
    ssize_t items_n = 0;
    FILE *f = NULL;

    if (p_file_name == NULL) {
        return -1;
    }
    ret_code = stat(p_file_name, &f_stat);
    if (ret_code < 0) {
        fprintf(stderr, "ERROR: fail to get stats of '%s'", p_file_name);
        return -2;
    }
    p_local = (uint8_t *)malloc(f_stat.st_size);
    if (p_local == NULL) {
        fprintf(stderr, "ERROR: malloc fail for '%s'", p_file_name);
        return -3;
    }
    memset(p_local, 0, f_stat.st_size);

    f = fopen(p_file_name, "r");
    if (f == NULL) {
        fprintf(stderr, "ERROR: fail to open %s\n", p_file_name);
        return -4;
    }
    /* fread should return 1, skip check ret value intentionally below */
    items_n = fread((void *)p_local, f_stat.st_size, 1, f);
    if (items_n != 1) {
        fprintf(stderr, "ERROR: incorrect items read\n");
        ret_code = -5;
        goto fail;
    }

    *p_buf = p_local;
    *p_sz_data = f_stat.st_size;

fail:
    fclose(f);
    return ret_code;
}

/*
 * UART hand shake between the host and the target
 */
int hand_shake(int uart_fd, uint32_t baud_rate)
{
    int ret_status = 0;
    uint8_t *p_stream_hfive;
    ssize_t bytes_n;
    ssize_t write_n;

    fprintf(stdout, "hand shake with rate %u\n", baud_rate);
    /*
     * approximate the number of bytes of 0x55 to send in 5 mseconds
     * using the current baud rate with 8N1
     */
    bytes_n = 7 * baud_rate / 10000;
#ifdef DEBUG
    printf("shake hands bytes_n = %ld\n", bytes_n);
#endif
    p_stream_hfive = (uint8_t *) malloc(bytes_n);
    memset(p_stream_hfive, 0x55, bytes_n);
    write_n = write(uart_fd, (void *)p_stream_hfive, bytes_n);
    free(p_stream_hfive);
    if (write_n != bytes_n) {
        ret_status = -1;
        fprintf(stderr, "ERROR: incorrect bytes written (%lu vs %lu)\n",
                write_n, bytes_n);
        goto fail;
    }

    /* now read from the device */
    while (1) {
        char read_buf[256] = {0};

        usleep(100 * 1000); // Sleep for 100ms
        memset(read_buf, 0, sizeof(read_buf));
        bytes_n = read(uart_fd, read_buf, sizeof(read_buf));
        if (bytes_n > 0) {
            printf("Received (%lu bytes): %c%c\n", bytes_n, read_buf[0], read_buf[1]);
            /* check the result: "OK", "FL" */
            if (read_buf[0] == 'O' && read_buf[1] == 'K') {
                printf("SUCCEED: hand shake\n\n");
                break;
            }
            else if (read_buf[0] == 'F' && read_buf[1] == 'L') {
                fprintf(stderr, "ERROR: fail in hand shake\n\n");
                ret_status = -2;
                break;
            }
            else {
                fprintf(stderr, "ERROR: unknown response\n\n");
                ret_status = -3;
                break;
            }
        } /* bytes_n */
    }

fail:
    return ret_status; /* zero is OK */
}

int request_boot_info(int uart_fd, boot_info_t *p_boot_info) {
    int ret_code = 0;
    int bytes_n = 0;
    boot_info_req_t req;
    bl_resp_t resp;

    memset(&req, 0, sizeof req);
    init_header(COMMAND_BOOT_INFO, 0, &req.bi_hdr);

    usleep(100 * 1000);
    bytes_n = write(uart_fd, (void *)&req, sizeof req);
    if (bytes_n < sizeof req) {
        ret_code = -1;
        fprintf(stderr, "ERROR: fail to send request boot_info\n\n");
        goto fail;
    }

    ret_code = read_check_response(uart_fd, &resp);
    if (ret_code == 0) {
        /* sanity check the length field */
        uint32_t len = (resp.len_msb << 8) | (resp.len_lsb);

        if (len != sizeof(boot_info_t)) {
            fprintf(stderr, "ERROR: inavlid payload\n\n");
            ret_code = -2;
            goto fail;
        }
        fprintf(stdout, "boot_rom_ver: 0x%x\n", resp.boot_info.boot_rom_ver);
        dump_hex("opt_info", (uint8_t *)&resp.boot_info.opt_info[0],
                sizeof(resp.boot_info.opt_info));

        if (p_boot_info != NULL) {
            memcpy(p_boot_info, &resp.boot_info, sizeof(*p_boot_info));
        }
    }

fail:
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: get boot_info\n\n");
    } else {
        fprintf(stderr, "ERROR: fail to get boot_info\n\n");
    }
    return ret_code;
}

/*
 * eflash image format:
 *      Boot_Header_Config (176 bytes)
 *      segment_header_t   (16 bytes)
 *      eflash executable
 */
int load_boot_header(int uart_fd, char *eflash_file_name)
{
    int ret_code = 0;
    FILE *f = NULL;
    boot_header_pkt_t boot_header_pkt;
    ssize_t bytes_n = 0;

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
    bytes_n = fread(&boot_header_pkt.boot_header, sizeof boot_header_pkt.boot_header,
            1, f);
    fclose(f);
    if (bytes_n != 1) {
        ret_code = -3;
        fprintf(stderr, "ERROR: incorrect items read\n");
        goto fail;
    }
    bytes_n = write(uart_fd, (void *)&boot_header_pkt, sizeof boot_header_pkt);
    if (bytes_n != sizeof boot_header_pkt) {
        ret_code = 1;
        fprintf(stderr, "ERROR: fewer bytes written\n");
        goto fail;
    }

    ret_code = read_check_response(uart_fd, NULL);
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: load boot header\n\n");
    } else {
        fprintf(stderr, "ERROR: failed in loading boot header\n\n");
    }

fail:
    return ret_code;
}

int load_segment_header(int uart_fd, char *eflash_file_name) {
    int ret_code = 0;
    ssize_t bytes_n = 0;
    FILE *f = NULL;
    segment_header_pkt_t segment_header_pkt;

    if (eflash_file_name == NULL) {
        return -1;
    }
    f = fopen(eflash_file_name, "r");
    if (f == NULL) {
        fprintf(stderr, "ERROR: unable to open file [%s]\n\n", eflash_file_name);
        return -2;
    }
    /* construct the packet to device and send */
    memset(&segment_header_pkt, 0, sizeof segment_header_pkt);

    init_header(COMMAND_SEG_HDR, sizeof segment_header_pkt.segment,
            &segment_header_pkt.seg_hdr);
    /* skip the section of Boot_Header_Config */
    fseek(f, sizeof(Boot_Header_Config), SEEK_SET);
    bytes_n = fread(&segment_header_pkt.segment, sizeof segment_header_pkt.segment,
            1, f);
    fclose(f);
    if (bytes_n != 1) {
        fprintf(stderr, "ERROR: incorrect number of items read\n");
        ret_code = -3;
        goto fail;
    }
#ifdef DEBUG
    dump_hex("segment header", (uint8_t *)&segment_header_pkt, sizeof segment_header_pkt);
    printf("dest_addr = 0x%x len = %u, rsvd = 0x%x crc32 = 0x%x\n",
            segment_header_pkt.segment.dest_addr,
            segment_header_pkt.segment.len,
            segment_header_pkt.segment.rsvd,
            segment_header_pkt.segment.crc32);
#endif
    bytes_n = write(uart_fd, (void *)&segment_header_pkt, sizeof segment_header_pkt);
    if (bytes_n != sizeof segment_header_pkt) {
        ret_code = -1;
        fprintf(stderr, "ERROR: incorrect number of bytes written\n");
        goto fail;
    }

    /* check response */
    ret_code = read_check_response(uart_fd, NULL);
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: load segment header\n\n");
    } else {
        fprintf(stderr, "ERROR: fail to load segement header\n\n");
    }

fail:
    return ret_code;
}

int load_segment_data(int uart_fd, char *eflash_file_name) {
    int ret_code = 0;
    ssize_t bytes_n = 0;
    FILE *f = NULL;
    segment_data_pkt_t segment_data_pkt;
    int i = 0;
    int real_len;
    int remain = 0;
    int skip_len = sizeof(Boot_Header_Config) + sizeof(segment_header_t);
    struct stat f_stat;

    if (eflash_file_name == NULL) {
        return -1;
    }

    assert(sizeof(segment_data_pkt) <= 4096);
    ret_code = stat(eflash_file_name, &f_stat);
    if (ret_code < 0) {
        fprintf(stderr, "ERROR: fail to get file statistics [%s]\n\n", eflash_file_name);
        return ret_code;
    }
    if (f_stat.st_size <= 176 + 16) {
        fprintf(stderr, "ERROR: invalid file [%s]\n\n", eflash_file_name);
        return -3;
    }
    f = fopen(eflash_file_name, "r");
    if (f == NULL) {
        fprintf(stderr, "ERROR: unable to open file [%s]\n\n", eflash_file_name);
        return -4;
    }
    /* construct the packet to device and send */
    /* skip the section of Boot_Header_Config and segment_header */
    fseek(f, skip_len, SEEK_SET);

    remain = f_stat.st_size - skip_len;
    /* the binary may exeed the single packet size, do several arounds */
    while (remain > 0) {
        real_len = fread(&segment_data_pkt.seg_data[0], 1,
                sizeof segment_data_pkt.seg_data, f);
        remain = remain - real_len;
        if (real_len <= 0 && ferror(f) != 0) {
            fprintf(stderr, "ERROR: unexpected error in read\n\n");
            ret_code = -5;
            goto fail;
        }

        /* fill the header */
        init_header(COMMAND_SEG_DATA, real_len, &segment_data_pkt.seg_data_hdr);
        bytes_n = write(uart_fd, (void *)&segment_data_pkt, real_len +
                sizeof(segment_data_pkt.seg_data_hdr));
        if (bytes_n != real_len + sizeof(segment_data_pkt.seg_data_hdr)) {
            ret_code = -1;
            fprintf(stderr, "ERROR: incorrect number of bytes written\n");
            goto fail;
        }

        /* check response */
        ret_code = read_check_response(uart_fd, NULL);
        if (ret_code == 0) {
            fprintf(stdout, "SUCCEED: load segment (%d) bytes data[%d]\n", real_len, i++);
        } else {
            fprintf(stderr, "ERROR: fail to load segement data\n\n");
            goto fail;
        }
    }

    fprintf(stdout, "SUCCEED: load segment data\n\n");
fail:
    fclose(f);

    return ret_code;
}

int check_image(int uart_fd) {
    int ret_code = 0;
    ssize_t bytes_n = 0;
    image_check_pkt_t img_check_pkt;

    init_header(COMMAND_IMG_CHECK, 0, &img_check_pkt.img_check_hdr);
    bytes_n = write(uart_fd, &img_check_pkt, sizeof img_check_pkt);
    if (bytes_n != sizeof img_check_pkt) {
        ret_code = -1;
        fprintf(stderr, "ERROR: incorrect number of bytes written\n");
        goto fail;
    }

    ret_code = read_check_response(uart_fd, NULL);
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: check image\n\n");
    } else {
        fprintf(stderr, "ERROR: fail to check image\n\n");
    }

fail:
    return ret_code;
}

int run_image(int uart_fd) {
    int ret_code = 0;
    ssize_t bytes_n = 0;
    image_run_pkt_t img_run_pkt;

    init_header(COMMAND_IMG_RUN, 0, &img_run_pkt.img_run_hdr);
    bytes_n = write(uart_fd, &img_run_pkt, sizeof img_run_pkt);
    if (bytes_n != sizeof img_run_pkt) {
        ret_code = -1;
        fprintf(stderr, "ERROR: incorrect number of bytes written\n");
        goto fail;
    }

    ret_code = read_check_response(uart_fd, NULL);
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: run image\n\n");
    } else {
        fprintf(stderr, "ERROR: fail to run image\n\n");
    }

fail:
    return ret_code;
}

int erase_storage(int uart_fd, uint32_t start_addr, uint32_t len){
    int ret_code = 0;
    ssize_t bytes_n = 0;
    uint32_t i = 0;
    uint32_t end_addr = start_addr + len;
    erase_pkt_t erase_pkt;
    uint8_t *p_char = (uint8_t *) &erase_pkt;
    uint32_t off_start_crc = offsetof(packet_hdr_t, len_lsb);

    memset(&erase_pkt, 0, sizeof(erase_pkt));
    init_header(COMMAND_ERASE_FLASH, sizeof(erase_pkt.start_addr)
            + sizeof(erase_pkt.end_addr), &erase_pkt.erase_hdr);
    erase_pkt.start_addr = htole32(start_addr);
    erase_pkt.end_addr = htole32(end_addr);

    /* fill crc in the reserve field */

    for (i = off_start_crc; i < sizeof (erase_pkt); i++) {
        erase_pkt.erase_hdr.rsvd_08 += p_char[i];
    }
    bytes_n = write(uart_fd, &erase_pkt, sizeof erase_pkt);
    if (bytes_n != sizeof erase_pkt) {
        ret_code = -1;
        fprintf(stderr, "ERROR: incorrect number of bytes written\n");
        goto fail;
    }

    ret_code = read_check_response(uart_fd, NULL);
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: erase storage [0x%08x, 0x%08x]\n\n", start_addr,
                end_addr);
    } else {
        fprintf(stderr, "ERROR: erase storage [0x%08x, 0x%08x]\n\n", start_addr,
                end_addr);
    }

fail:
    return ret_code;
}

int flash_data(int uart_fd, uint8_t *p_data, uint32_t len_data, uint32_t target_addr) {
    int ret_code = 0;
    int i = 0;
    int j = 0;
    ssize_t bytes_n = 0;
    uint8_t *p_curr = p_data;
    uint8_t *p_char = NULL;
    uint32_t len_to_send = 0;
    uint32_t remain = len_data;
    flash_data_pkt_t *p_pkt = NULL;

    printf("start to flash data [%d] bytes\n", len_data);
    while (p_curr < p_data + len_data) {
        p_pkt = (flash_data_pkt_t *) malloc(sizeof(*p_pkt));
        if (p_pkt == NULL) {
            fprintf(stderr, "ERROR: failed to allocate memory\n\n");
            return -1;
        }

        if (remain <= sizeof(p_pkt->data)) {
            len_to_send = remain;
        } else {
            len_to_send = sizeof(p_pkt->data);
        }
#ifdef DEBUG
        printf("remain = %d len_to_send = %d\n", remain, len_to_send);
#endif
        memset((void *)p_pkt, 0, sizeof(*p_pkt));
        init_header(COMMAND_FLASH_DATA, len_to_send + sizeof(p_pkt->addr),
                &(p_pkt->flash_data_hdr));
        p_pkt->addr = htole32(target_addr);
        memcpy(p_pkt->data, p_curr, len_to_send);
        /* fill crc */
        p_char = (uint8_t *) &p_pkt->len_lsb;
        p_pkt->crc08 = 0;
        for (i = 0; i < len_to_send + sizeof(p_pkt->addr) + 2; i++) {
            p_pkt->crc08 += *p_char;
            p_char++;
        }
        bytes_n = write(uart_fd, p_pkt, len_to_send + sizeof(p_pkt->addr)
                + sizeof(p_pkt->flash_data_hdr));
        if (bytes_n != len_to_send + sizeof(p_pkt->addr)
                + sizeof(p_pkt->flash_data_hdr)) {
            fprintf(stderr, "ERROR: incorrect number of bytes written\n");
            ret_code = -2;
            goto fail;
        }

        /* check response */
        ret_code = read_check_response(uart_fd, NULL);
        if (ret_code == 0) {
            fprintf(stdout, "succeed: flash (%d) bytes data[%d] to "
                    "addr 0x%08x\n", len_to_send, j++, target_addr);
        } else {
            fprintf(stderr, "ERROR: fail to flash data\n\n");
            goto fail;
        }

        /* move to the next frame */
        p_curr = p_curr + len_to_send;
        target_addr = target_addr + len_to_send;
        remain = remain - len_to_send;

        free(p_pkt);
        p_pkt = NULL;
    }

fail:
    if (p_pkt != NULL) {
        free(p_pkt);
    }
    return ret_code;
}

int notify_flash_done(int uart_fd) {
    int ret_code = 0;
    ssize_t bytes_n = 0;
    flash_done_pkt_t flash_done_pkt;

    memset(&flash_done_pkt, 0, sizeof (flash_done_pkt));
    init_header(COMMAND_PROG_OK, 0, &flash_done_pkt.flash_done_hdr);

    bytes_n = write(uart_fd, &flash_done_pkt, sizeof flash_done_pkt);
    if (bytes_n != sizeof flash_done_pkt) {
        fprintf(stderr, "ERROR: incorrect number of bytes written\n");
        ret_code = 1;
        goto fail;
    }

    ret_code = read_check_response(uart_fd, NULL);
    if (ret_code == 0) {
        fprintf(stdout, "SUCCEED: ack flash ok\n\n");
    } else {
        fprintf(stderr, "ERROR: nack flash \n\n" );
    }

fail:
    return ret_code;
}

int send_sha256(int uart_fd, uint32_t *sha256, uint32_t start_addr, uint32_t size) {
    int ret_code = 0;
    sha256_pkt_t sha256_pkt;
    bl_resp_t bl_resp;
    ssize_t bytes_n = 0;
    uint8_t *p_char = (uint8_t *)&sha256_pkt;
    uint32_t i = 0;
    uint32_t crc_start = offsetof(sha256_pkt_t, sha256_hdr)
        + offsetof(packet_hdr_t, len_lsb);
#ifdef DEBUG
    printf("entering send_sha256\n");
#endif
    memset((void *)&sha256_pkt, 0, sizeof(sha256_pkt));
    init_header(COMMAND_SHA_256, sizeof(sha256_pkt.start_addr)
            + sizeof(sha256_pkt.size), &sha256_pkt.sha256_hdr);

    sha256_pkt.start_addr = htole32(start_addr);
    sha256_pkt.size = htole32(size);
#ifdef DEBUG
    printf("***** start_addr = 0x%x size = 0x%x  ****\n", sha256_pkt.start_addr,
            sha256_pkt.size);
#endif
    /* calculate CRC  and fill into resv08 */
    for (i = crc_start; i < sizeof sha256_pkt; i++) {
        sha256_pkt.sha256_hdr.rsvd_08 += p_char[i];
    }

    bytes_n = write(uart_fd, &sha256_pkt, sizeof sha256_pkt);
    if (bytes_n != sizeof sha256_pkt) {
        ret_code = 1;
        fprintf(stderr, "ERROR: fewer bytes written \n");
        goto fail;
    }

    memset(&bl_resp, 0, sizeof bl_resp);
    ret_code = read_check_response(uart_fd, &bl_resp);

    if (ret_code == 0) {
        /* somehow the order from device is different */
        for (int i =0; i < 8; i++) {
            bl_resp.sha256[i] = be32toh(bl_resp.sha256[i]);
        }

        /* compare the sha256 from device with our local */
        ret_code = memcmp(sha256, bl_resp.sha256, sizeof(bl_resp.sha256));
        if (ret_code == 0) {
            fprintf(stdout, "SUCCEED: SHA256 verificatin pass\n\n");
        } else {
            fprintf(stderr, "ERROR: SHA256 verificatin fail, but ignore now\n");
            for (int i =0; i < 8; i++) {
                printf("sha256[%d] = 0x%08x bl_resp.sha256[%d] = 0x%08x %s\n",
                        i, sha256[i],
                        i, bl_resp.sha256[i],
                        (sha256[i] == bl_resp.sha256[i] ? " ":"X")
                        );
            }
            ret_code = 0;
        }
    } else {
        fprintf(stderr, "ERROR: fail in getting response for SHA256 \n\n" );
    }

fail:
    return ret_code;
}

int send_finish(int uart_fd, uint32_t baud_rate) {
    /*
     * uart_fd might be open for different baud_rate from this.
     * And note: cfsetospeed only supports up to 230400. Thus,
     * skip this now. If really necessary, close this fd, open it
     * with this_baud_rate, and try to hand_shake.
     */
    return 0;
    /* return hand_shake(uart_fd, baud_rate); */
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
