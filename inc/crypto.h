/*
 * crypto function to calculate SHA256
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

/*
 * direct implement of SHA256 based on
 *      publication number: FIPS PUB 180-4
 *      title: Secure Hash Standard (SHS)
 */
#ifndef _CRYPTO_H
#define _CRYPTO_H

#include <stdint.h>

void calc_sha256(uint8_t *p_msg, uint32_t len, uint32_t *p_sha);

uint32_t calc_crc32(const char *src, uint32_t sz);
#endif /* _CRYPTO_H */
