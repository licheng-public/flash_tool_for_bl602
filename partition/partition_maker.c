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
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include "common_share.h"
#include "partition.h"

struct offset_table_t {
    const char *field_name;
    uint32_t offset;
    uint32_t size;
} offset_table_pt_entry[] = {
    {"type", offsetof(pt_table_entry_config_t, type), SSIZE(pt_table_entry_config_t, type)},
    {"device", offsetof(pt_table_entry_config_t, device), SSIZE(pt_table_entry_config_t, device)},
    {"active_index", offsetof(pt_table_entry_config_t, active_index), SSIZE(pt_table_entry_config_t, active_index)},
    {"name", offsetof(pt_table_entry_config_t, name), SSIZE(pt_table_entry_config_t, name)},
    {"address0", offsetof(pt_table_entry_config_t, address[0]), SSIZE_A(pt_table_entry_config_t, address)},
    {"address1", offsetof(pt_table_entry_config_t, address[1]), SSIZE_A(pt_table_entry_config_t, address)},
    {"size0", offsetof(pt_table_entry_config_t, max_len[0]), SSIZE_A(pt_table_entry_config_t, max_len)},
    {"size1", offsetof(pt_table_entry_config_t, max_len[1]), SSIZE_A(pt_table_entry_config_t, max_len)},
    {"len", offsetof(pt_table_entry_config_t, len), SSIZE(pt_table_entry_config_t, len)},
    {"age", offsetof(pt_table_entry_config_t, age), SSIZE(pt_table_entry_config_t, age)},
};


static void print_help(const char *p_app)
{
    fprintf(stderr, "Usage: %s -i partition_cfg_file -o output_partition_bin\n",
            p_app);
    return;
}

static bool is_empty_line(char *buf) {
    int i = 0;
    bool has_nothing = true;

    while(buf[i] != '\0') {
        if (!isspace(buf[i])) {
            has_nothing = false;
            break;
         }
        i++;
    }

    return has_nothing;
}

/*
 * https://zlib.net/crc_v3.txt
 * for fun:
 * https://stackoverflow.com/questions/2587766/how-is-a-crc32-checksum-calculated
 */
static uint32_t calc_crc32(const char *src, uint32_t sz)
{
    char ch;
    uint32_t crc = ~0;
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t b = 0;

    printf("crc = 0x%x\n", crc);
    for(i = 0; i < sz; i++) {
        ch = src[i];
        for(j = 0; j < 8; j++) {
            b = (ch ^ crc) & 1;
            crc >>= 1;
            if (b) {
                crc=crc^0xEDB88320;
            }
            ch >>= 1;
        }
    }

    printf("crc = %x\n", ~crc);
    return ~crc;
}

/*
 * The customized parser to handle the partition configuration
 * in toml format. Intended to fullfill the need without dependency.
 */
static int parse_partition_cfg(const char *p_cfg_file,
    pt_table_stuff_config_t *p_partition,
    uint32_t *p_partition_address, uint32_t *addr_cnt)
{
    int ret_code = 0;
    int idx = -1; /* strange */
    int i = 0;
    FILE *p_file = fopen(p_cfg_file, "r");
    char buf[512];
    char *p_val = NULL;
    bool in_pt_table = false;

    if (p_file == NULL) {
        ret_code = -1;
        fprintf(stderr, "ERROR: fail to open file %s\n", p_cfg_file);
        return ret_code;
    }

    *addr_cnt = 0;
    memset(buf, 0, sizeof buf);
    while (NULL != fgets(buf, sizeof buf, p_file)) {
        if (buf[0] == '#' || is_empty_line(buf)) {
            /* ignore comment */
            memset(buf, 0, sizeof buf);
            continue;
        }
        if (strncmp(buf, "[pt_table]", strlen("[pt_table]")) == 0) {
            in_pt_table = true;
            memset(buf, 0, sizeof buf);
            continue;
        } else if (strncmp(buf, "[[pt_entry]]", strlen("[[pt_entry]]")) == 0) {
            in_pt_table = false;
            memset(buf, 0, sizeof buf);
            idx++;
            if (idx >= PT_ENTRY_MAX) {
                fprintf(stderr, "ERROR: reach maximum partition entries\n");
                ret_code = -3;
                break;
            }
            continue;
        }

        p_val = strchr(buf, '=');
        if (p_val == NULL) {
            /* ignore */
            fprintf(stderr, "ERROR: [%s] not a 'name = value' pair\n", buf);
            ret_code = -2;
            goto fail;
        }
        *p_val = '\0';
        p_val++;

        if (in_pt_table) {
            /* field parsed filled in pt_table */
            if (strncmp(buf, "address0", 8) == 0) {
                p_partition_address[0] = strtoul(p_val, NULL, 16);
                *addr_cnt = *addr_cnt + 1;
             } else if (strncmp(buf, "address1", 8) == 0) {
                 p_partition_address[1] = strtoul(p_val, NULL, 16);
                 *addr_cnt = *addr_cnt + 1;
             }
        } else {
            /* field value filled in pt_entry */
            int o_cnt = ARRAY_SIZE(offset_table_pt_entry);
            for (i = 0; i < o_cnt; i++) {
                if (strncmp(buf, offset_table_pt_entry[i].field_name,
                            strlen(offset_table_pt_entry[i].field_name)) == 0) {
                    /* name or value */
                    if (strncmp(buf,"name", 4) == 0) {
                        char *p_left = strchr(p_val, '"');
                        char *p_right = strrchr(p_val, '"');
                        int j = 0;
                        p_left = p_left + 1;
                        p_right = p_right - 1;
                        while(p_left <= p_right) {
                            p_partition->pt_entries[idx].name[j++] = *p_left;
                            p_left++;
                            if (j > offset_table_pt_entry[i].size) {
                                break;
                            }
                        }
                    } else {
                        uint32_t val = strtoul(p_val, NULL, 16);
                        memcpy((void *)((char *)&p_partition->pt_entries[idx]
                                    + offset_table_pt_entry[i].offset), &val,
                                offset_table_pt_entry[i].size);
                    }
                    break;
                } /* field matched */
            } /* for loop */
        } /* is_pt_table */
    } /* while(fgets) */

    p_partition->pt_table.magic = BFLB_PT_MAGIC_CODE;
    p_partition->pt_table.entry_cnt = idx + 1;
    /*
     * TODO: what is the crc32 in p_partition->pt_table.crc32?
     * unused?
     */
    p_partition->crc32 = calc_crc32((char *)&p_partition->pt_entries,
            sizeof p_partition->pt_entries);

fail:
    fclose(p_file);
    return ret_code;
}

static int write_file(char *file_name, void *contents, uint32_t size)
{
    FILE *f = fopen(file_name, "w");
    size_t item_cnt = 0;

    if (f == NULL) {
        fprintf(stderr, "ERROR: failed to open %s\n", file_name);
        return -1;
    }
    item_cnt = fwrite(contents, size, 1, f);
    if (item_cnt != 1) {
        fprintf(stderr, "ERROR: failed to write\n");
        return -2;
    }

    fclose(f);
    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    int ret_code = 0;
    char *in_filename = NULL;
    char *out_filename = NULL;
    pt_table_stuff_config_t partition;
    uint32_t partition_addr[16] = {0};
    uint32_t addr_cnt;

    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch (opt) {
        case 'i':
            in_filename = optarg;
            break;
        case 'o':
            out_filename = optarg;
            break;
        default: /* '?' */
            print_help(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (in_filename == NULL || out_filename == NULL) {
        print_help(argv[0]);
        ret_code = -1;
        goto fail;
    }

    memset(&partition, 0, sizeof partition);
    ret_code = parse_partition_cfg(in_filename, &partition, partition_addr, &addr_cnt);
    if (ret_code == 0) {
        /* write partition bin */
        char bin_file[256] = {0};
        int i = 0;
        if (addr_cnt != 0) {
            for (i = 0; i < addr_cnt; i++) {
                sprintf(bin_file, "%s@0x%x", out_filename, partition_addr[i]);
                ret_code = write_file(bin_file, (void *)&partition, sizeof partition);
                if (ret_code != 0) {
                    goto fail;
                }
            }
        } else {
            ret_code = write_file(out_filename, (void *)&partition, sizeof partition);
            if (ret_code != 0) {
                goto fail;
            }
        }
    }

fail:
    return ret_code;
}

