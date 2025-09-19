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

#ifndef _COMM_H
#define _COMM_H

#include "packet_comm.h"

/*XXX: move read_to_buf and dump_hex  out of common.* */
int read_to_buf(char *p_file_name, uint8_t **p_buf, uint32_t *p_sz_data);

void dump_hex(const char *prefix, uint8_t *p_data, uint32_t len);

int hand_shake(int uart_fd, uint32_t baud_rate);

int load_boot_header(int uart_fd, char *eflash_file_name);

int request_boot_info(int uart_fd, boot_info_t *p_boot_info);

int load_pub_key(int uart_fd);

int load_signature(int uart_fd);

int load_aes_iv(int uart_fd);

int load_segment_header(int uart_fd, char *eflash_file_name);

int load_segment_data(int uart_fd, char *eflash_file_name);

int check_image(int uart_fd);

int run_image(int uart_fd);

int erase_storage(int uart_fd, uint32_t start_addr, uint32_t len);

int flash_data(int uart_fd, uint8_t *data, uint32_t len_data, uint32_t target_addr);

int notify_flash_done(int uart_fd);

int send_sha256(int uart_fd, uint32_t *sha256, uint32_t start_addr, uint32_t len);

int send_finish(int uart_fd, uint32_t baud_rate);

#endif /* _COMM_H */
