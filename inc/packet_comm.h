/*
 * communication packet format between host and BL 60x
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

#ifndef _PACKET_COMMON_H_
#define _PACKET_COMMON_H_

#include <stdint.h>
#include "boot_header_info.h"

#define BFLB_BOOT2_SIGN_MAXSIZE                 2048/8

typedef enum {
    COMMAND_BOOT_INFO   = 0x10,
    COMMAND_BOOT_HDR    = 0x11,
    COMMAND_PUB_KEY     = 0x12,
    COMMAND_SIGNATURE   = 0x14,
    COMMAND_AES_IV      = 0x16,
    COMMAND_SEG_HDR     = 0x17,
    COMMAND_SEG_DATA    = 0x18,
    COMMAND_IMG_CHECK   = 0x19,
    COMMAND_IMG_RUN     = 0x1A,
    COMMAND_ERASE_FLASH = 0x30,
    COMMAND_FLASH_DATA  = 0x31,
    COMMAND_PROG_OK     = 0x3A,
    COMMAND_SHA_256     = 0x2D

} COMMAND_ID;

typedef struct {
    uint8_t cmd_id;
    uint8_t rsvd_08;
    uint8_t len_lsb;
    uint8_t len_msb;
} packet_hdr_t;

/* boot info */
typedef struct {
    packet_hdr_t bi_hdr;
} boot_info_req_t;

/* boot header */
typedef struct {
    packet_hdr_t bh_hdr;
    Boot_Header_Config boot_header;
} boot_header_pkt_t;

/* public key */
typedef struct {
    uint8_t eckeyx[32]; /* ec key in boot info */
    uint8_t eckeyy[32]; /* ec key in boot info */
    uint32_t crc32;
} pkey_cfg_t;

typedef struct {
    packet_hdr_t pkey_hdr;
    pkey_cfg_t pkey_cfg;
} pub_key_load_t;

/* signature  */
typedef struct {
    uint32_t sig_len;
    uint8_t signature[BFLB_BOOT2_SIGN_MAXSIZE];
    uint32_t crc32;
}sign_cfg_t;

typedef struct {
    packet_hdr_t sig_hdr;
    sign_cfg_t sig;
} sig_pkt_t;

/* aes_iv */
typedef struct {
    uint8_t aesiv[16];
    uint32_t crc32;
} aesiv_cfg_t;

typedef struct {
    packet_hdr_t aes_iv_hdr;
    aesiv_cfg_t aes_iv;
} aes_iv_pkt_t;

/* segment header */
typedef struct {
    uint32_t dest_addr;
    uint32_t len;
    uint32_t rsvd;
    uint32_t crc32;
} segment_header_t;

typedef struct {
    packet_hdr_t seg_hdr;
    segment_header_t segment;
} segment_header_pkt_t;

/* segment data */
/* 
 * BL602_ISP_protocol says: 4096 is the limitation of protocol frame size.
 * If larger, send multiple data to send
 * NOTE: test shows that above is not true.
 */
typedef struct {
    packet_hdr_t seg_data_hdr;
    uint8_t seg_data[2048];
} segment_data_pkt_t;

/* image check */
typedef struct {
    packet_hdr_t img_check_hdr;
} image_check_pkt_t;

/* image run */
typedef struct {
    packet_hdr_t img_run_hdr;
} image_run_pkt_t;

/* erase command */
typedef struct {
    packet_hdr_t erase_hdr;
    uint32_t start_addr;
    uint32_t end_addr;
} erase_pkt_t;

/* program data */
typedef struct {
    union {
        packet_hdr_t flash_data_hdr;
        struct {
            uint8_t cmd_id;
            uint8_t crc08;
            uint8_t len_lsb;
            uint8_t len_msb;
        };
    };
    uint32_t addr;
    uint8_t data[8 * 1024];
} flash_data_pkt_t;

/* flash done */
typedef struct {
    packet_hdr_t flash_done_hdr;
} flash_done_pkt_t;

/* send SHA256 */
typedef struct {
    packet_hdr_t sha256_hdr;
    uint32_t start_addr;
    uint32_t size;
} sha256_pkt_t;

typedef struct {
    uint32_t boot_rom_ver;
    union {
        uint8_t opt_info[16];
        /* XXX: faked, not published by Buffalolab */
        struct __attribute__ ((__packed__)){
            uint8_t sign              :  2;   /* [1: 0]      for sign*/
            uint8_t encrypted         :  2;   /* [3: 2]      for encrypt */
            uint8_t encrypt_type      :  2;   /* [5: 4]      for encrypt*/
            uint8_t keySel            :  2;   /* [7: 6]      for key sel in boot interface*/
            uint8_t unknown[15];
        };
    };
} boot_info_t;

typedef struct __attribute__ ((__packed__)){
    union {
        struct {
            uint8_t result[2]; /* 'O''K' or 'F''L' */
        };
        struct {
            uint8_t result_e[2]; /* place holder. */
            uint8_t err_lsb;
            uint8_t err_msb;
        };
        struct {
            uint8_t result_i[2]; /* place holder. */
            uint8_t len_lsb;
            uint8_t len_msb;
            boot_info_t boot_info;
        };
        struct {
            uint8_t result_s[2]; /* place holder */
            uint8_t len_lsb_s;
            uint8_t len_msb_s;
            uint8_t sha256[32];
        };
    };
} bl_resp_t;

#endif /* _PACKET_COMMON_H_ */
