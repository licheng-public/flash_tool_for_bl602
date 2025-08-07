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
#include <stdint.h>

#include "partition.h"

union {
    uint8_t bin[sizeof(pt_table_stuff_config_t)];
    pt_table_stuff_config_t pt_table_config;
} partition_bin;

void break_point(void) {
}

int main(int argc, char *argv[]) {
    FILE *p_file = NULL;
    if (argc < 2) {
        fprintf(stderr, "ERROR: nothing to dump\n");
        return -1;
    }
    p_file = fopen(argv[1], "r");
    if (p_file == NULL) {
    }
    fread(&partition_bin, sizeof partition_bin, 1, p_file);

    fclose(p_file);
    break_point();

    return 0;
}
