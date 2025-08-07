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

int request_boot_info(int uart_fd);

int load_boot_header(int uart_fd);

int load_pub_key(int uart_fd);

int load_signature(int uart_fd);

int load_aes_iv(int uart_fd);

int load_segment_header(int uart_fd);

int load_segment_data(int uart_fd);

int check_image(int uart_fd);

int run_image(int uart_fd);

#endif /* _COMM_H */
