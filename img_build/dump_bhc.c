/*
 * for dumping the offset related information in GDB
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
 * usage steps:
 * gcc test.c -g -o ./test
 * gdb ./test
 * ptype /o bhc
 */
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define BFLB_BOOT2_HASH_SIZE                    256/8
#define BFLB_BOOT2_SIGN_MAXSIZE                 2048/8

/* port from bl602_sflash.h */
typedef struct SPI_Flash_Cfg_Type  {
    uint8_t ioMode;                         /*!< Serail flash interface mode,bit0-3:IF mode,bit4:unwrap */
    uint8_t cReadSupport;                   /*!< Support continuous read mode,bit0:continuous read mode support,bit1:read mode cfg */
    uint8_t clkDelay;                       /*!< SPI clock delay,bit0-3:delay,bit4-6:pad delay */
    uint8_t clkInvert;                      /*!< SPI clock phase invert,bit0:clck invert,bit1:rx invert,bit2-4:pad delay,bit5-7:pad delay */
    uint8_t resetEnCmd;                     /*!< Flash enable reset command */
    uint8_t resetCmd;                       /*!< Flash reset command */
    uint8_t resetCreadCmd;                  /*!< Flash reset continuous read command */
    uint8_t resetCreadCmdSize;              /*!< Flash reset continuous read command size */
    uint8_t jedecIdCmd;                     /*!< JEDEC ID command */
    uint8_t jedecIdCmdDmyClk;               /*!< JEDEC ID command dummy clock */
    uint8_t qpiJedecIdCmd;                  /*!< QPI JEDEC ID comamnd */
    uint8_t qpiJedecIdCmdDmyClk;            /*!< QPI JEDEC ID command dummy clock */
    uint8_t sectorSize;                     /*!< *1024bytes */
    uint8_t mid;                            /*!< Manufacturer ID */
    uint16_t pageSize;                      /*!< Page size */
    uint8_t chipEraseCmd;                   /*!< Chip erase cmd */
    uint8_t sectorEraseCmd;                 /*!< Sector erase command */
    uint8_t blk32EraseCmd;                  /*!< Block 32K erase command,some Micron not support */
    uint8_t blk64EraseCmd;                  /*!< Block 64K erase command */
    uint8_t writeEnableCmd;                 /*!< Need before every erase or program */
    uint8_t pageProgramCmd;                 /*!< Page program cmd */
    uint8_t qpageProgramCmd;                /*!< QIO page program cmd */
    uint8_t qppAddrMode;                    /*!< QIO page program address mode */
    uint8_t fastReadCmd;                    /*!< Fast read command */
    uint8_t frDmyClk;                       /*!< Fast read command dummy clock */
    uint8_t qpiFastReadCmd;                 /*!< QPI fast read command */
    uint8_t qpiFrDmyClk;                    /*!< QPI fast read command dummy clock */
    uint8_t fastReadDoCmd;                  /*!< Fast read dual output command */
    uint8_t frDoDmyClk;                     /*!< Fast read dual output command dummy clock */
    uint8_t fastReadDioCmd;                 /*!< Fast read dual io comamnd */
    uint8_t frDioDmyClk;                    /*!< Fast read dual io command dummy clock */
    uint8_t fastReadQoCmd;                  /*!< Fast read quad output comamnd */
    uint8_t frQoDmyClk;                     /*!< Fast read quad output comamnd dummy clock */
    uint8_t fastReadQioCmd;                 /*!< Fast read quad io comamnd */
    uint8_t frQioDmyClk;                    /*!< Fast read quad io comamnd dummy clock */
    uint8_t qpiFastReadQioCmd;              /*!< QPI fast read quad io comamnd */
    uint8_t qpiFrQioDmyClk;                 /*!< QPI fast read QIO dummy clock */
    uint8_t qpiPageProgramCmd;              /*!< QPI program command */
    uint8_t writeVregEnableCmd;             /*!< Enable write reg */
    uint8_t wrEnableIndex;                  /*!< Write enable register index */
    uint8_t qeIndex;                        /*!< Quad mode enable register index */
    uint8_t busyIndex;                      /*!< Busy status register index */
    uint8_t wrEnableBit;                    /*!< Write enable bit pos */
    uint8_t qeBit;                          /*!< Quad enable bit pos */
    uint8_t busyBit;                        /*!< Busy status bit pos */
    uint8_t wrEnableWriteRegLen;            /*!< Register length of write enable */
    uint8_t wrEnableReadRegLen;             /*!< Register length of write enable status */
    uint8_t qeWriteRegLen;                  /*!< Register length of contain quad enable */
    uint8_t qeReadRegLen;                   /*!< Register length of contain quad enable status */
    uint8_t releasePowerDown;               /*!< Release power down command */
    uint8_t busyReadRegLen;                 /*!< Register length of contain busy status */
    uint8_t readRegCmd[4];                  /*!< Read register command buffer */
    uint8_t writeRegCmd[4];                 /*!< Write register command buffer */
    uint8_t enterQpi;                       /*!< Enter qpi command */
    uint8_t exitQpi;                        /*!< Exit qpi command */
    uint8_t cReadMode;                      /*!< Config data for continuous read mode */
    uint8_t cRExit;                         /*!< Config data for exit continuous read mode */
    uint8_t burstWrapCmd;                   /*!< Enable burst wrap command */
    uint8_t burstWrapCmdDmyClk;             /*!< Enable burst wrap command dummy clock */
    uint8_t burstWrapDataMode;              /*!< Data and address mode for this command */
    uint8_t burstWrapData;                  /*!< Data to enable burst wrap */
    uint8_t deBurstWrapCmd;                 /*!< Disable burst wrap command */
    uint8_t deBurstWrapCmdDmyClk;           /*!< Disable burst wrap command dummy clock */
    uint8_t deBurstWrapDataMode;            /*!< Data and address mode for this command */
    uint8_t deBurstWrapData;                /*!< Data to disable burst wrap */
    uint16_t timeEsector;                   /*!< 4K erase time */
    uint16_t timeE32k;                      /*!< 32K erase time */
    uint16_t timeE64k;                      /*!< 64K erase time */
    uint16_t timePagePgm;                   /*!< Page program time */
    uint16_t timeCe;                        /*!< Chip erase time in ms */
    uint8_t pdDelay;                        /*!< Release power down command delay time for wake up */
    uint8_t qeData;                         /*!< QE set data */
}__attribute__ ((packed)) SPI_Flash_Cfg_Type_t;
/* port ended */

/* port start from blsp_bootinfo.h */
typedef struct Boot_Flash_Config{
    uint32_t magicCode;       /*'FCFG'*/
    struct SPI_Flash_Cfg_Type cfg;
    uint32_t crc32;
}Boot_Flash_Config_t;

typedef struct Boot_Sys_Clk_Config{
    uint8_t xtalType;
    uint8_t pllClk;
    uint8_t hclkDiv;
    uint8_t bclkDiv;

    uint8_t flashClkType;
    uint8_t flashClkDiv;
    uint8_t rsvd[2];
}Boot_Sys_Clk_Config_t;

typedef struct Boot_Clk_Config{
    uint32_t magicCode;       /*'PCFG'*/
    struct Boot_Sys_Clk_Config cfg;
    uint32_t crc32;
}Boot_Clk_Config_t;

#define __PACKED_STRUCT struct __attribute__ ((__packed__))
#define __PACKED_UNION union __attribute__ ((__packed__))

struct Boot_Header_Config{
    uint32_t magicCode;                   /*'BFXP'*/
    uint32_t rivison;
    struct Boot_Flash_Config flashCfg;
    struct Boot_Clk_Config   clkCfg;
    __PACKED_UNION {
        __PACKED_STRUCT {
          uint32_t sign              :  2;   /* [1: 0]      for sign*/
          uint32_t encryptType       :  2;   /* [3: 2]      for encrypt */
          uint32_t keySel            :  2;   /* [5: 4]      for key sel in boot interface*/
          uint32_t rsvd6_7           :  2;   /* [7: 6]      for encrypt*/
          uint32_t noSegment         :  1;   /* [8]         no segment info */
          uint32_t cacheEnable       :  1;   /* [9]         for cache */
          uint32_t notLoadInBoot     :  1;   /* [10]        not load this img in bootrom */
          uint32_t aesRegionLock     :  1;   /* [11]        aes region lock */
          uint32_t cacheWayDisable   :  4;   /* [15: 12]    cache way disable info*/
          uint32_t crcIgnore         :  1;   /* [16]        ignore crc */
          uint32_t hashIgnore        :  1;   /* [17]        hash crc */
          uint32_t haltCPU1          :  1;   /* [18]        halt ap */
          uint32_t rsvd19_31         :  13;  /* [31:19]     rsvd */
        } bval;
        uint32_t wval;
    }bootCfg ;

    __PACKED_UNION {
        uint32_t segmentCnt;
        uint32_t imgLen;
    }imgSegmentInfo;

    uint32_t bootEntry;      /* entry point of the image*/
    __PACKED_UNION {
        uint32_t ramAddr;
        uint32_t flashOffset;
    }imgStart;

    uint8_t hash[BFLB_BOOT2_HASH_SIZE];    /*hash of the image*/

    uint32_t rsv1;
    uint32_t rsv2;
    uint32_t crc32;
};
/* port end */

union {
    uint8_t bin[sizeof(struct Boot_Header_Config)];
    struct Boot_Header_Config bhc;
} bhc_bin;

void break_point(void) {
}

struct Boot_Header_Config g_bhc;
int main(int argc, char *argv[]) {
    FILE *p_file = NULL;

    if (argc < 2) {
        fprintf(stderr, "ERROR: nothing to dump\n");
        return -1;
    }
    p_file = fopen(argv[1], "r");
    if (p_file == NULL) {
    }

    memset(&g_bhc, 0, sizeof g_bhc);

    printf("%d\n", sizeof g_bhc);
    fread(&bhc_bin, sizeof bhc_bin, 1, p_file);

    fclose(p_file);

    break_point();
    return 0;
}
