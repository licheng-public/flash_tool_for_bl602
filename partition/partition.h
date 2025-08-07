/**
 *  revised by Liang Cheng for the tool of partition generation
  ******************************************************************************
  * @file    bl602_partition.h
  * @version V1.0
  * @date
  * @brief   This file is the standard driver header file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2019 Bouffalo Lab</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of Bouffalo Lab nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#ifndef __BL602_PARTITION_H__
#define __BL602_PARTITION_H__

#include <stdint.h>

/** @addtogroup  BL602_Common_Driver
 *  @{
 */

/** @addtogroup  PARTITION
 *  @{
 */

/** @defgroup  PARTITION_Public_Types
 *  @{
 */

/**
 *  @brief Partition table error type definition
 */
typedef enum {
    PT_ERROR_SUCCESS,                       /*!< Partition table error type:success */
    PT_ERROR_TABLE_NOT_VALID,               /*!< Partition table error type:entry not found */
    PT_ERROR_ENTRY_NOT_FOUND,               /*!< Partition table error type:entry not found */
    PT_ERROR_ENTRY_UPDATE_FAIL,             /*!< Partition table error type:entry update fail */
    PT_ERROR_CRC32,                         /*!< Partition table error type:crc32 error */
    PT_ERROR_PARAMETER,                     /*!< Partition table error type:input parameter error */
    PT_ERROR_FALSH_READ,                    /*!< Partition table error type:flash read error */
    PT_ERROR_FALSH_WRITE,                   /*!< Partition table error type:flash write error */
    PT_ERROR_FALSH_ERASE,                   /*!< Partition table error type:flash erase error */
} pt_table_error_t;

/**
 *  @brief Partition id type definition
 */
typedef enum {
    PT_TABLE_ID_0,                          /*!< Partition table ID 0 */
    PT_TABLE_ID_1,                          /*!< Partition table ID 1 */
    PT_TABLE_ID_INVALID,                    /*!< Partition table ID invalid */
} pt_table_id_t;

/**
 *  @brief Partition id type definition
 */
typedef enum {
    PT_ENTRY_FW_CPU0,                       /*!< Partition entry type:CPU0 firmware */
    PT_ENTRY_FW_CPU1,                       /*!< Partition entry type:CPU1 firmware */
    PT_ENTRY_MAX=16,                        /*!< Partition entry type:Max */
} pt_table_entry_t;

/**
 *  @brief Partition table config definition
 */
typedef struct {
    uint32_t magic;                         /*!< Partition table magic code */
    uint16_t version;                       /*!< Partition table verdion */
    uint16_t entry_cnt;                     /*!< Partition table entry count */
    uint32_t age;                           /*!< Partition table age */
    uint32_t crc32;                         /*!< Partition table CRC32 value */
} pt_table_config_t;

/**
 *  @brief Partition table entry config definition
 */
typedef struct {
    uint8_t type;                           /*!< Partition entry type */
    uint8_t device;                         /*!< Partition entry device */
    uint8_t active_index;                    /*!< Partition entry active index */
    uint8_t name[9];                        /*!< Partition entry name */
    uint32_t address[2];                    /*!< Partition entry start address */
    uint32_t max_len[2];                     /*!< Partition entry max length */
    uint32_t len;                           /*!< Partition entry length */
    uint32_t age;                           /*!< Partition entry age */
} pt_table_entry_config_t;

/**
 *  @brief Partition table stuff config definition
 */
typedef struct {
    pt_table_config_t pt_table;                 /*!< Partition table */
    pt_table_entry_config_t pt_entries[PT_ENTRY_MAX];    /*!< Partition entries */
    uint32_t crc32;                         /*!< Partition entries crc32 */
} pt_table_stuff_config_t;

/*@} end of group PARTITION_Public_Types */

/** @defgroup  PARTITION_Public_Constants
 *  @{
 */

/** @defgroup  PTTABLE_ERROR_TYPE
 *  @{
 */
#define IS_PTTABLE_ERROR_TYPE(type)                      (((type) == PT_ERROR_SUCCESS) || \
                                                          ((type) == PT_ERROR_TABLE_NOT_VALID) || \
                                                          ((type) == PT_ERROR_ENTRY_NOT_FOUND) || \
                                                          ((type) == PT_ERROR_ENTRY_UPDATE_FAIL) || \
                                                          ((type) == PT_ERROR_CRC32) || \
                                                          ((type) == PT_ERROR_PARAMETER) || \
                                                          ((type) == PT_ERROR_FALSH_READ) || \
                                                          ((type) == PT_ERROR_FALSH_WRITE) || \
                                                          ((type) == PT_ERROR_FALSH_ERASE))

/** @defgroup  PTTABLE_ID_TYPE
 *  @{
 */
#define IS_PTTABLE_ID_TYPE(type)                         (((type) == PT_TABLE_ID_0) || \
                                                          ((type) == PT_TABLE_ID_1) || \
                                                          ((type) == PT_TABLE_ID_INVALID))

/** @defgroup  PTTABLE_ENTRY_TYPE
 *  @{
 */
#define IS_PTTABLE_ENTRY_TYPE(type)                      (((type) == PT_ENTRY_FW_CPU0) || \
                                                          ((type) == PT_ENTRY_FW_CPU1) || \
                                                          ((type) == PT_ENTRY_MAX))

/*@} end of group PARTITION_Public_Constants */

/** @defgroup  PARTITION_Public_Macros
 *  @{
 */
#define BFLB_PT_TABLE0_ADDRESS                               0xE000
#define BFLB_PT_TABLE1_ADDRESS                               0xF000
#define BFLB_PT_MAGIC_CODE                                   0x54504642

/*@} end of group PARTITION_Public_Macros */

/*@} end of group PARTITION */

/*@} end of group BL602_Common_Driver */

#endif /* __BL602_PARTITION_H__ */
