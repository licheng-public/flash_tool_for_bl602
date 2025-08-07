/*
 * common share header
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

#ifndef _COMMON__SHARE_H
#define _COMMON_SHARE_H

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define SSIZE(type, field)  sizeof(((type *)0)->field)
#define SSIZE_A(type, field)  sizeof(((type *)0)->field[0])

#endif /* _COMMON_SHARE_H */
