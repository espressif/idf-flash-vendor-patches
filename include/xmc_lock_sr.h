/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Locks the XMC with SR lock issue with pre-defined good values.
 *
 * This is a convenient function that calls `xmc_read_flash_id` and `xmc_lock_sr` for specific flash models with pre-defined good values.
 *
 * This function will check and lock the SR of flash chips that matches following IDs. Other flash chips will not be affected.
 *  - Flash ID is 204016, 204017, 204018
 *  - SFDP[0x06] == 0x02, SFDP[0xf4] == 0xff
 *
 * The good value written will: (See the source code in xmc_lock_sr.c for the exact value. This value is different according to the chips)
 *  - Disable block/sector protection
 *  - Not lock Security Registers
 *  - Set Quad Enabled (WP/HD disabled)
 *  - Set Output Strength same as factory value (25% for XM25QH32C and XM25QH128C, 75% for XM25QH64C)
 *  - Lock SR (permanently or until power-cycle, according to argument `permanent`).
 *  - Set Dummy Cycle same as factory value.
 *
 * If you have different requirements, you may use the utility functions below to rewrite your own version.
 *
 * @param permanent True to lock permanently (SRP1, SRP0) = (1, 1), otherwise until power-cycle (1, 0).
 * @note CAUTION: This may avoid further usage of HFM mode (> 80Mhz). Only call this when you are sure this device will not run above 80MHz.
 */
void xmc_check_lock_sr(bool permanent);

/*
 * Utils for writing your own-version of `xmc_check_lock_sr`, for example, write different good values or on different flash chips
 */

/**
 * @brief Read the IDs of XMC flash chips used to identify XMC flash chips that needs SR locking.
 *
 * @param out_flash_id The flash ID read by RDID (command 9Fh).
 * @param out_sfdp_06  The data read by RDSFDP (command 5Ah), address 06h.
 * @param out_sfdp_f4  The data read by RDSFDP (command 5Ah), address F4h.
 */
void xmc_read_flash_id(uint32_t *out_flash_id, uint8_t *out_sfdp_06, uint8_t *out_sfdp_f4);

/**
 * @brief Lock the SR with given good-values.
 *
 * This function will skip when SRP1 is already set (locked permanently/power-cycle). Unnecessary WRSR (write to same value) will be skipped for better stability.
 *
 * @param good_value Good-values to write, bit 23-16 for SR3, bit 15-8 for SR2, bit 7-0 for SR1. This function will handle SRP1 and SRP0, don't set them yourself.
 * @param permanent - True to lock the SR permanently. THIS IS IRREVERSIBLE, please check the good value before doing this.
 *                  - False to lock the SR until power-cycle. SR written by this function will restore after power-cycle.
 */
void xmc_lock_sr(uint32_t good_value, bool permanent);

#ifdef __cplusplus
}
#endif
