/*
 * build image with boot_header_info
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
/* for fstat */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "packet_comm.h"
#include "common_share.h"
#include "crypto.h"


#define SFC_TABLE_E(var, alias, type) {#var, #alias, offsetof(type, var) \
    + offsetof(Boot_Flash_Config, cfg) \
    + offsetof(Boot_Header_Config, flashCfg), SSIZE(type, var)}

/* Boot_Flash_Config */
#define BFC_TABLE_E(var, alias, type) {#var, #alias, offsetof(type, var) \
    + offsetof(Boot_Header_Config, flashCfg), SSIZE(type, var)}
/* Boot_Sys_Clk_Config */
#define BSCC_TABLE_E(var, alias, type) {#var, #alias, offsetof(type, var) \
    + offsetof(Boot_Clk_Config, cfg) \
    + offsetof(Boot_Header_Config, clkCfg), SSIZE(type, var)}

/* Boot_Clk_Config */
#define BCC_TABLE_E(var, alias, type) {#var, #alias, offsetof(type, var) \
    + offsetof(Boot_Header_Config, clkCfg), SSIZE(type, var)}

/* Boot_Header_Config */
#define BHC_TABLE_E(var, alias, type) {#var, #alias, offsetof(type, var), SSIZE(type, var)}

/* XXX:
 * The config file is flat, and the name in config does not match
 * with the name of the field in structure.
 *
 * CRC and hash fields should not be parsed/filled.
 * */
struct offset_table_t {
    char *name;
    char *alias; /* used for indexing */
    uint32_t offset;
    uint32_t size; /* in bytes */
} offset_table [] = {
    SFC_TABLE_E(ioMode,             io_mode,                SPI_Flash_Cfg_Type),
    SFC_TABLE_E(cReadSupport,       cont_read_support,      SPI_Flash_Cfg_Type),
    SFC_TABLE_E(clkDelay,           sfctrl_clk_delay,       SPI_Flash_Cfg_Type),
    SFC_TABLE_E(clkInvert,          sfctrl_clk_invert,      SPI_Flash_Cfg_Type),
    SFC_TABLE_E(resetEnCmd,         reset_en_cmd,           SPI_Flash_Cfg_Type),
    SFC_TABLE_E(resetCmd,           reset_cmd,              SPI_Flash_Cfg_Type),
    SFC_TABLE_E(resetCreadCmd,      exit_contread_cmd,      SPI_Flash_Cfg_Type),
    SFC_TABLE_E(resetCreadCmdSize,  exit_contread_cmd_size, SPI_Flash_Cfg_Type),
    SFC_TABLE_E(jedecIdCmd,         jedecid_cmd,            SPI_Flash_Cfg_Type),
    SFC_TABLE_E(jedecIdCmdDmyClk,   jedecid_cmd_dmy_clk,    SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qpiJedecIdCmd,      qpi_jedecid_cmd,        SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qpiJedecIdCmdDmyClk, qpi_jedecid_dmy_clk,   SPI_Flash_Cfg_Type),
    SFC_TABLE_E(sectorSize,         sector_size,            SPI_Flash_Cfg_Type),
    SFC_TABLE_E(mid,                mfg_id,                 SPI_Flash_Cfg_Type),
    SFC_TABLE_E(pageSize,           page_size,              SPI_Flash_Cfg_Type),
    SFC_TABLE_E(chipEraseCmd,       chip_erase_cmd,         SPI_Flash_Cfg_Type),
    SFC_TABLE_E(sectorEraseCmd,     sector_erase_cmd,       SPI_Flash_Cfg_Type),
    SFC_TABLE_E(blk32EraseCmd,      blk32k_erase_cmd,       SPI_Flash_Cfg_Type),
    SFC_TABLE_E(blk64EraseCmd,      blk64k_erase_cmd,       SPI_Flash_Cfg_Type),
    SFC_TABLE_E(writeEnableCmd,     write_enable_cmd,       SPI_Flash_Cfg_Type),
    SFC_TABLE_E(pageProgramCmd,     page_prog_cmd,          SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qpageProgramCmd,    qpage_prog_cmd,         SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qppAddrMode,        qual_page_prog_addr_mode, SPI_Flash_Cfg_Type),
    SFC_TABLE_E(fastReadCmd,        fast_read_cmd,          SPI_Flash_Cfg_Type),
    SFC_TABLE_E(frDmyClk,           fast_read_dmy_clk,      SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qpiFastReadCmd,     qpi_fast_read_cmd,      SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qpiFrDmyClk,        qpi_fast_read_dmy_clk,  SPI_Flash_Cfg_Type),
    SFC_TABLE_E(fastReadDoCmd,      fast_read_do_cmd,       SPI_Flash_Cfg_Type),
    SFC_TABLE_E(frDoDmyClk,         fast_read_do_dmy_clk,   SPI_Flash_Cfg_Type),
    SFC_TABLE_E(fastReadDioCmd,     fast_read_dio_cmd,      SPI_Flash_Cfg_Type),
    SFC_TABLE_E(frDioDmyClk,        fast_read_dio_dmy_clk,  SPI_Flash_Cfg_Type),
    SFC_TABLE_E(fastReadQoCmd,      fast_read_qo_cmd,       SPI_Flash_Cfg_Type),
    SFC_TABLE_E(frQoDmyClk,         fast_read_qo_dmy_clk,   SPI_Flash_Cfg_Type),
    SFC_TABLE_E(fastReadQioCmd,     fast_read_qio_cmd,      SPI_Flash_Cfg_Type),
    SFC_TABLE_E(frQioDmyClk,        fast_read_qio_dmy_clk,  SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qpiFastReadQioCmd,  qpi_fast_read_qio_cmd,  SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qpiFrQioDmyClk,     qpi_fast_read_qio_dmy_clk, SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qpiPageProgramCmd,  qpi_page_prog_cmd,      SPI_Flash_Cfg_Type),
    SFC_TABLE_E(writeVregEnableCmd, write_vreg_enable_cmd,  SPI_Flash_Cfg_Type),
    SFC_TABLE_E(wrEnableIndex,      wel_reg_index,          SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qeIndex,            qe_reg_index,           SPI_Flash_Cfg_Type),
    SFC_TABLE_E(busyIndex,          busy_reg_index,         SPI_Flash_Cfg_Type),
    SFC_TABLE_E(wrEnableBit,        wel_bit_pos,            SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qeBit,              qe_bit_pos,             SPI_Flash_Cfg_Type),
    SFC_TABLE_E(busyBit,            busy_bit_pos,           SPI_Flash_Cfg_Type),
    SFC_TABLE_E(wrEnableWriteRegLen, wel_reg_write_len,     SPI_Flash_Cfg_Type),
    SFC_TABLE_E(wrEnableReadRegLen, wel_reg_read_len,       SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qeWriteRegLen,      qe_reg_write_len,       SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qeReadRegLen,       qe_reg_read_len,        SPI_Flash_Cfg_Type),
    SFC_TABLE_E(releasePowerDown,   release_power_down,     SPI_Flash_Cfg_Type),
    SFC_TABLE_E(busyReadRegLen,     busy_reg_read_len,      SPI_Flash_Cfg_Type),
    /* outliers: readRegCmd[4], and writeRegCmd[4] */
    {"readRegCmd[0]", "reg_read_cmd0", offsetof(SPI_Flash_Cfg_Type, readRegCmd[0])
        + offsetof(Boot_Flash_Config, cfg)
        + offsetof(Boot_Header_Config, flashCfg),
            SSIZE_A(SPI_Flash_Cfg_Type,readRegCmd)},
    {"readRegCmd[1]","reg_read_cmd1", offsetof(SPI_Flash_Cfg_Type, readRegCmd[1])
        + offsetof(Boot_Flash_Config, cfg)
        + offsetof(Boot_Header_Config, flashCfg),
            SSIZE_A(SPI_Flash_Cfg_Type,readRegCmd)},
    {"readRegCmd[2]","reg_reg_cmd2", offsetof(SPI_Flash_Cfg_Type, readRegCmd[2])
        + offsetof(Boot_Flash_Config, cfg)
        + offsetof(Boot_Header_Config, flashCfg),
            SSIZE_A(SPI_Flash_Cfg_Type,readRegCmd)},
    {"readRegCmd[3]","reg_read_cmd3", offsetof(SPI_Flash_Cfg_Type, readRegCmd[3])
        + offsetof(Boot_Flash_Config, cfg)
        + offsetof(Boot_Header_Config, flashCfg),
            SSIZE_A(SPI_Flash_Cfg_Type,readRegCmd)},
    {"writeRegCmd[0]", "reg_write_cmd0", offsetof(SPI_Flash_Cfg_Type, writeRegCmd[0])
        + offsetof(Boot_Flash_Config, cfg)
        + offsetof(Boot_Header_Config, flashCfg),
            SSIZE_A(SPI_Flash_Cfg_Type,writeRegCmd)},
    {"writeRegCmd[1]", "reg_write_cmd1", offsetof(SPI_Flash_Cfg_Type, writeRegCmd[1])
        + offsetof(Boot_Flash_Config, cfg)
        + offsetof(Boot_Header_Config, flashCfg),
            SSIZE_A(SPI_Flash_Cfg_Type, writeRegCmd)},
    {"writeRegCmd[2]", "reg_write_cmd2", offsetof(SPI_Flash_Cfg_Type, writeRegCmd[2])
        + offsetof(Boot_Flash_Config, cfg)
        + offsetof(Boot_Header_Config, flashCfg),
            SSIZE_A(SPI_Flash_Cfg_Type, writeRegCmd)},
    {"writeRegCmd[3]", "reg_write_cmd3", offsetof(SPI_Flash_Cfg_Type, writeRegCmd[3])
        + offsetof(Boot_Flash_Config, cfg)
        + offsetof(Boot_Header_Config, flashCfg),
            SSIZE_A(SPI_Flash_Cfg_Type,writeRegCmd)},
    SFC_TABLE_E(enterQpi,           enter_qpi_cmd,          SPI_Flash_Cfg_Type),
    SFC_TABLE_E(exitQpi,            exit_qpi_cmd,           SPI_Flash_Cfg_Type),
    SFC_TABLE_E(cReadMode,          cont_read_code,         SPI_Flash_Cfg_Type),
    SFC_TABLE_E(cRExit,             cont_read_exit_code,    SPI_Flash_Cfg_Type),
    SFC_TABLE_E(burstWrapCmd,       burst_wrap_cmd,         SPI_Flash_Cfg_Type),
    SFC_TABLE_E(burstWrapCmdDmyClk, burst_wrap_dmy_clk,     SPI_Flash_Cfg_Type),
    SFC_TABLE_E(burstWrapDataMode,  burst_wrap_data_mode,   SPI_Flash_Cfg_Type),
    SFC_TABLE_E(burstWrapData,      burst_wrap_code,        SPI_Flash_Cfg_Type),
    SFC_TABLE_E(deBurstWrapCmd,     de_burst_wrap_cmd,      SPI_Flash_Cfg_Type),
    SFC_TABLE_E(deBurstWrapCmdDmyClk, de_burst_wrap_cmd_dmy_clk, SPI_Flash_Cfg_Type),
    SFC_TABLE_E(deBurstWrapDataMode, de_burst_wrap_code_mode, SPI_Flash_Cfg_Type),
    SFC_TABLE_E(deBurstWrapData,    de_burst_wrap_code,     SPI_Flash_Cfg_Type),
    SFC_TABLE_E(timeEsector,        sector_erase_time,      SPI_Flash_Cfg_Type),
    SFC_TABLE_E(timeE32k,           blk32k_erase_time,      SPI_Flash_Cfg_Type),
    SFC_TABLE_E(timeE64k,           blk64k_erase_time,       SPI_Flash_Cfg_Type),
    SFC_TABLE_E(timePagePgm,        page_prog_time,         SPI_Flash_Cfg_Type),
    SFC_TABLE_E(timeCe,             chip_erase_time,        SPI_Flash_Cfg_Type),
    SFC_TABLE_E(pdDelay,            power_down_delay,       SPI_Flash_Cfg_Type),
    SFC_TABLE_E(qeData,             qe_data,                SPI_Flash_Cfg_Type),
    /* BFC_TABLE_E(crc32,              flashcfg_crc32,         Boot_Flash_Config), */
    BFC_TABLE_E(magicCode,          flashcfg_magic_code,    Boot_Flash_Config),
    // boot_sys_clock_cfg
    BSCC_TABLE_E(xtalType,          xtal_type,              Boot_Sys_Clk_Config),
    BSCC_TABLE_E(pllClk,            pll_clk,                Boot_Sys_Clk_Config),
    BSCC_TABLE_E(hclkDiv,           hclk_div,               Boot_Sys_Clk_Config),
    BSCC_TABLE_E(bclkDiv,           bclk_div,               Boot_Sys_Clk_Config),
    BSCC_TABLE_E(flashClkType,      flash_clk_type,         Boot_Sys_Clk_Config),
    BSCC_TABLE_E(flashClkDiv,       flash_clk_div,          Boot_Sys_Clk_Config),
    // boot_clk_config
    BCC_TABLE_E(magicCode,          clkcfg_magic_code,      Boot_Clk_Config),
    BCC_TABLE_E(crc32,              clkcfg_crc32,           Boot_Clk_Config),
    // boot_header_config
    BHC_TABLE_E(magicCode,          magic_code,             Boot_Header_Config),
    BHC_TABLE_E(rivison,            revision,               Boot_Header_Config),
    BHC_TABLE_E(imgSegmentInfo.segmentCnt,  segment_cnt,    Boot_Header_Config),
    BHC_TABLE_E(imgSegmentInfo.imgLen,      img_len,        Boot_Header_Config),
    BHC_TABLE_E(bootEntry,                  bootentry,      Boot_Header_Config),
    BHC_TABLE_E(imgStart.ramAddr,           img_start,      Boot_Header_Config),
    BHC_TABLE_E(imgStart.flashOffset,       flash_offset,   Boot_Header_Config),
    /*
    BHC_TABLE_E(hash[BFLB_BOOT2_HASH_SIZE], hash,           Boot_Header_Config),
    BHC_TABLE_E(crc32,                      crc32,          Boot_Header_Config),
    */
};


#define OFFSET_BOOTCFG offsetof(Boot_Header_Config, bootCfg)
struct offset_table_wt_bits_t {
    char *name;
    char *alias;
    uint32_t bit_len;
    uint32_t pos;
    uint32_t offset;
} offset_table_wt[] = {
     {"sign",           "sign",             2,   0,     OFFSET_BOOTCFG},
     {"encryptType",    "encrypt_type",     2,   2,     OFFSET_BOOTCFG},
     {"keySel",         "key_sel",          2,   4,     OFFSET_BOOTCFG},
     {"rsvd6_7",        "rsvd6_7",          2,   6,     OFFSET_BOOTCFG},
     {"noSegment",      "no_segment",       1,   8,     OFFSET_BOOTCFG},
     {"cacheEnable",    "cache_enable",     1,   9,     OFFSET_BOOTCFG},
     {"notLoadInBoot",  "notload_in_bootrom", 1, 10,    OFFSET_BOOTCFG},
     {"aesRegionLock",  "aes_region_lock",  1,   11,    OFFSET_BOOTCFG},
     {"cacheWayDisable", "cache_way_disable", 4, 12,    OFFSET_BOOTCFG},
     {"crcIgnore",      "crc_ignore",       1,  16,     OFFSET_BOOTCFG},
     {"hashIgnore",     "hash_ignore",      1,  17,     OFFSET_BOOTCFG},
     {"haltCPU1",       "halt_cpu1",        1,  18,     OFFSET_BOOTCFG},
     {"rsvd19_31",      "rsvd19_31",        13, 19,     OFFSET_BOOTCFG},
};

#if DEBUG
static void print_offset(void) {
    uint32_t i = 0;

    for (i = 0; i < ARRAY_SIZE(offset_table); i++) {
        printf("%64s \t%u \t%u\n",
                offset_table[i].alias,
                offset_table[i].offset, offset_table[i].size);
    }
    for (i = 0; i < ARRAY_SIZE(offset_table_wt); i++) {
        printf("%64s \t%u \t%u \t%u\n",
                offset_table_wt[i].alias,
                offset_table_wt[i].pos,
                offset_table_wt[i].bit_len,
                offset_table_wt[i].offset);
    }
    return;
}
#endif

static void print_help(const char *p_app)
{
    fprintf(stderr, "Usage: %s -i boot_cfg_file -b src_bin -o output_bin -s offset\n",
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
 * get token and value pointer from a string.
 *
 * if the string is without =, the pointer to the first non-space
 * is returned. The token would be *p_token. *p_val = NULL
 * if the string is with '=', the poiner to the first non-space
 * is returned. The token would be *p_token, and *p_val points to
 * the first character after '='.
 *
 * NOTE: p_src string is MODIFIED
 */
static void get_token_and_value(char *p_src, char **p_token, char **p_val) {
    char *p_tmp = p_src;

    *p_token = NULL;
    *p_val = NULL;

    while (isspace(*p_tmp) && *p_tmp != '\0') {
        p_tmp++;
    }
    *p_token = p_tmp;
    if (*p_tmp == '\0') {
        *p_val = NULL;
        return;
    }
    /* now the case is "abc    " or "abc   = 3   " or "=" */
    p_tmp = strchr(*p_token, '=');
    if (p_tmp == NULL) {
        /* standardlone token, like the line with "abc\n" */
        *p_val = NULL;
        p_tmp = *p_token + strlen(*p_token) - 1; /* point to the last char */
        while (isspace(*p_tmp)) {
            *p_tmp = '\0';
            p_tmp--;
        }
    } else {
        *p_val = p_tmp + 1;
        do {
            *p_tmp = '\0';
            p_tmp--;
        } while (isspace(*p_tmp) && p_tmp > (*p_token));
    }

    return;
}

/*
 * The customized parser to handle the bhc configuration
 * in toml format. Intended to fullfill the need without dependency.
 */
static int parse_boot_header_cfg(const char *p_cfg_file,
    Boot_Header_Config *p_bhc, uint32_t img_len)
{
    int ret_code = 0;
    int i = 0;
    FILE *p_file = fopen(p_cfg_file, "r");
    char buf[256];
    char *p_val = NULL;
    bool in_efuse_cfg = false;

    if (p_file == NULL) {
        ret_code = -1;
        fprintf(stderr, "ERROR: fail to open file %s\n", p_cfg_file);
        return ret_code;
    }

    memset(buf, 0, sizeof buf);
    while (NULL != fgets(buf, sizeof buf, p_file)) {
        char *p_token = NULL;

        if (buf[0] == '#' || is_empty_line(buf)) {
            /* ignore comment */
            memset(buf, 0, sizeof buf);
            continue;
        }
        /* get pointer to token and value */
        get_token_and_value(buf, &p_token, &p_val);
        if (strcmp(p_token, "[EFUSE_CFG]") == 0) {
            in_efuse_cfg = true;
            memset(buf, 0, sizeof buf);
            continue;
        } else if (strcmp(p_token, "[BOOTHEADER_CFG]") == 0) {
            in_efuse_cfg = false;
            memset(buf, 0, sizeof buf);
            continue;
        }

        if (in_efuse_cfg) {
            /* bypass efuse config */
            continue;
        } else {
            /* field value filled in boot_header_cfg */
            bool found = false;
            int o_cnt = ARRAY_SIZE(offset_table);
            for (i = 0; i < o_cnt; i++) {
                if (strcmp(p_token, offset_table[i].alias) == 0) {
                    /* value */
                    uint32_t val = strtoul(p_val, NULL, 0);
                    memcpy((void *)((char *)p_bhc + offset_table[i].offset), &val,
                            offset_table[i].size);
                    found = true;
                    break;
                } /* field matched */
            } /* for loop */

            /* could be bits field */
            if (!found) {
                int o_cnt = ARRAY_SIZE(offset_table_wt);
                for (i = 0; i < o_cnt; i++) {
                    if (strcmp(p_token, offset_table_wt[i].alias) == 0) {
                        /* value */
                        uint32_t val = strtoul(p_val, NULL, 0);
                        val = val & (~(0x1 << offset_table_wt[i].bit_len));
                        val = val << offset_table_wt[i].pos;
                        p_bhc->bootCfg.wval = p_bhc->bootCfg.wval | val;
                        found = true;
                        break;
                    } /* field matched */
                } /* for loop */
            } /* !found */

            if (!found) {
                fprintf(stderr, "WARNING: unknown field %s\n", p_token);
                memset(buf, 0, sizeof buf);
                continue;
            }
            memset(buf, 0, sizeof buf);
        } /* is_efuse_cfg */
    } /* while(fgets) */

    /* overwrite the imgLen field with the real */
    p_bhc->imgSegmentInfo.imgLen = img_len;

    /* fill crc fields in the order */
    p_bhc->flashCfg.crc32 = calc_crc32((char *)&p_bhc->flashCfg.cfg,
            offsetof(Boot_Flash_Config, crc32) - offsetof(Boot_Flash_Config, cfg));
    p_bhc->clkCfg.crc32 = calc_crc32((char *)&p_bhc->clkCfg.cfg,
            offsetof(Boot_Clk_Config, crc32) - offsetof(Boot_Clk_Config, cfg));
    p_bhc->crc32 = calc_crc32((char *)p_bhc, offsetof(Boot_Header_Config, crc32));

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
    uint32_t offset = 0;
    char *cfg_filename = NULL;
    char *bin_filename = NULL;
    char *out_filename = NULL;
    Boot_Header_Config bhc;
    uint32_t len;
    struct stat bin_stats;
    char *p_buf = NULL;
    FILE *p_file_bin = NULL;
    uint32_t hash[8] = {0};


    while ((opt = getopt(argc, argv, "i:b:o:s:")) != -1) {
        switch (opt) {
        case 'i':
            cfg_filename = optarg;
            break;
        case 'b':
            bin_filename = optarg;
            break;
        case 'o':
            out_filename = optarg;
            break;
        case 's':
            offset = strtoul(optarg, NULL, 16);
            break;
        default: /* '?' */
            print_help(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (cfg_filename == NULL || bin_filename == NULL || out_filename == NULL
            || offset < sizeof(Boot_Header_Config)) {
        print_help(argv[0]);
        exit(EXIT_SUCCESS);
    }

    memset(&bhc, 0, sizeof bhc);

    /*
     * step 1: patch source image to align 16 bytes, read into the buffer
     * ,calculate SHA256 and prefill into bhc
     */
    p_file_bin = fopen(bin_filename, "r");
    if (p_file_bin == NULL) {
        fprintf(stderr, "ERROR: failed to open image file\n");
        return -1;
    }
    /*
     * establish a big buffer to accormadate the size of space from
     * 0 to offset, followed by the original image
     */
    stat(bin_filename, &bin_stats);
    len = (bin_stats.st_size + 15) / 16 * 16;
    p_buf = malloc(offset + len);
    if (p_buf == NULL) {
        fprintf(stderr, "ERROR: failed to allocate enough memory\n");
        fclose(p_file_bin);
        return -2;
    }

    /* copy the image into the space at offset in buffer */
    memset(p_buf + offset, 0xFF, len);
    ret_code = fread(p_buf + offset, bin_stats.st_size, 1, p_file_bin);
    fclose(p_file_bin);
    if (ret_code != 1) {
        fprintf(stderr, "ERROR: failed to read binary\n");
        return -3;
    }
    /* calculate hash of the bin, and fill into bhc */
    calc_sha256((uint8_t *)p_buf + offset, len, &hash[0]);
    for (int i = 0; i < 8; i++) {
        hash[i] = htobe32(hash[i]);
    }
    memcpy((void *)&bhc.hash[0], (void *)&hash[0], 32);
    //calc_sha256((uint8_t *)p_buf + offset, len, (uint32_t *)&bhc.hash[0]);

    /* step 2: parse Boot Header Config file into bhc */
    ret_code = parse_boot_header_cfg(cfg_filename, &bhc, len);
    if (ret_code == 0) {
        /* now pre-fill 0xFF and overwrite with Boot_Header_Config in buffer*/
        memset(p_buf, 0xFF, offset);
        memcpy(p_buf, &bhc, sizeof bhc);

        /* step 3: finally write packed image bin */
        ret_code = write_file(out_filename, (void *)p_buf, offset + len);
        if (ret_code != 0) {
            fprintf(stderr, "ERROR: failed in writing packed bin\n");
            return ret_code;
        }
    }
    exit(EXIT_SUCCESS);
}
