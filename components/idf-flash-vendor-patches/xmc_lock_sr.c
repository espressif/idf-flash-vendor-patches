/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <inttypes.h>
#include "esp_bit_defs.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "bootloader_flash.h"
#include "sdkconfig.h"


#if __has_include("esp_rom_spiflash.h")
#include "esp_rom_spiflash.h"

#elif CONFIG_IDF_TARGET_ESP32
#include "esp32/rom/spi_flash.h"

#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/rom/spi_flash.h"

#elif CONFIG_IDF_TARGET_ESP32S3
#include "esp32s3/rom/spi_flash.h"

#elif CONFIG_IDF_TARGET_ESP32C3
#include "esp32c3/rom/spi_flash.h"

#else
#error Not supported on this target yet.

#endif

static const char TAG[] = "xmc_sr";

#define CMD_WRSR        0x01
#define CMD_WRDI        0x04
#define CMD_RDSR        0x05
#define CMD_WREN        0x06
#define CMD_WRSR3       0x11 /* Not all SPI flash uses this command */
#define CMD_RDSR3       0x15 /* Not all SPI flash uses this command */
#define CMD_RDSR2       0x35 /* Not all SPI flash uses this command */
#define CMD_VSR_WREN    0x50

#define SR_SRP1_BIT     BIT8
#define SR_SRP0_BIT     BIT7

#define IS_XMC(flash_id)                        (((flash_id) & 0xFF0000) == 0x200000)
#define XMC_NEEDS_SR_LOCK(sfdp_06, sfdp_f4)     ((sfdp_06) == 0x02 && (sfdp_f4) == 0xff)


void spi_flash_disable_interrupts_caches_and_other_cpu(void);
void spi_flash_enable_interrupts_caches_and_other_cpu(void);

uint32_t bootloader_flash_read_sfdp(uint32_t sfdp_addr, unsigned int miso_byte_num);
uint32_t bootloader_execute_flash_command(uint8_t command, uint32_t mosi_data, uint8_t mosi_len, uint8_t miso_len);

__attribute__((weak)) unsigned IRAM_ATTR bootloader_read_status_8b_rdsr(void)
{
    return bootloader_execute_flash_command(CMD_RDSR, 0, 0, 8);
}

__attribute__((weak)) unsigned IRAM_ATTR bootloader_read_status_8b_rdsr2(void)
{
    return bootloader_execute_flash_command(CMD_RDSR2, 0, 0, 8);
}

__attribute__((weak)) unsigned IRAM_ATTR bootloader_read_status_8b_rdsr3(void)
{
    return bootloader_execute_flash_command(CMD_RDSR3, 0, 0, 8);
}

__attribute__((weak)) void IRAM_ATTR bootloader_write_status_16b_wrsr(unsigned new_status)
{
    bootloader_execute_flash_command(CMD_WRSR, new_status, 16, 0);
}

__attribute__((weak)) void IRAM_ATTR bootloader_write_status_8b_wrsr3(unsigned new_status)
{
    bootloader_execute_flash_command(CMD_WRSR3, new_status, 8, 0);
}

void IRAM_ATTR xmc_read_flash_id(uint32_t *out_flash_id, uint8_t *out_sfdp_06, uint8_t *out_sfdp_f4)
{
    uint32_t flash_id;
    uint8_t sfdp_06 = 0xcc;
    uint8_t sfdp_f4 = 0xcc;

    spi_flash_disable_interrupts_caches_and_other_cpu();
    esp_rom_spiflash_wait_idle(&g_rom_flashchip);
    flash_id = bootloader_read_flash_id();
    if (IS_XMC(flash_id)) {
        sfdp_06 = bootloader_flash_read_sfdp(0x06, 1);
        sfdp_f4 = bootloader_flash_read_sfdp(0xF4, 1);
    }
    spi_flash_enable_interrupts_caches_and_other_cpu();

    *out_flash_id = flash_id;
    *out_sfdp_06 = sfdp_06;
    *out_sfdp_f4 = sfdp_f4;
}

void IRAM_ATTR xmc_lock_sr(uint32_t good_value, bool permanent)
{
    bool skip = false;
    const uint8_t cmd_wren = (permanent? CMD_WREN: CMD_VSR_WREN);
    uint8_t sr1_after;
    uint8_t sr2_after;
    uint8_t sr3_after;

    //SRP0 and SRP1 will be handled by this function. Remove the user-specified bits.
    good_value &= ~(SR_SRP0_BIT | SR_SRP1_BIT | 0xFF000000);

    uint32_t sr12_set = (good_value & 0xFFFF) | SR_SRP1_BIT;
    if (permanent) {
        sr12_set |= SR_SRP0_BIT;
    }

    uint8_t sr3_set = (good_value & 0xFF0000) >> 16;

    spi_flash_disable_interrupts_caches_and_other_cpu();
    esp_rom_spiflash_wait_idle(&g_rom_flashchip);
    uint8_t sr1_before = bootloader_read_status_8b_rdsr();
    uint8_t sr2_before = bootloader_read_status_8b_rdsr2();
    uint8_t sr3_before = bootloader_read_status_8b_rdsr3();

    sr1_after = sr1_before;
    sr2_after = sr2_before;
    sr3_after = sr3_before;

    uint32_t sr12_before = (sr2_before << 8) | sr1_before;
    //When SRP1 is already set, the SR is already protected (permanently/power-cycle) and cannot be modified anymore.
   if (sr12_before & SR_SRP1_BIT) {
       skip = true;
   }

    bool status_written = false;
    //SR3 needs to be written before SRP bits are set.
    if (!skip && sr3_set != sr3_before) {
        bootloader_execute_flash_command(cmd_wren, 0, 0, 0);
        bootloader_write_status_8b_wrsr3(sr3_set);
        status_written = true;

        esp_rom_spiflash_wait_idle(&g_rom_flashchip);
        sr3_after = bootloader_read_status_8b_rdsr3();
    }

   //Write SRP1, 0 and good value bits at same time.
   if (!skip && sr12_set != sr12_before) {
        bootloader_execute_flash_command(cmd_wren, 0, 0, 0);
        bootloader_write_status_16b_wrsr(sr12_set);
        status_written = true;

        esp_rom_spiflash_wait_idle(&g_rom_flashchip);
        sr2_after = bootloader_read_status_8b_rdsr2();
        sr1_after = bootloader_read_status_8b_rdsr();
    }

    //If status bits are written, WRDI is requried to avoid WEL left ON.
    if (status_written) {
        bootloader_execute_flash_command(CMD_WRDI, 0, 0, 0);
    }
    spi_flash_enable_interrupts_caches_and_other_cpu();

    if (skip) {
        ESP_LOGI(TAG, "SRP1 already set (%02X%02X%02X), skip SR lock.", sr3_before, sr2_before, sr1_before);
    } else {
        if (permanent) {
            ESP_LOGI(TAG, "Try lock SR permanently.");
        } else {
            ESP_LOGI(TAG, "Try lock SR until power cycle.");
        }
        ESP_LOGI(TAG, "SR3 update: %02X -> %02X (%02X).", sr3_before, sr3_after, sr3_set);
        ESP_LOGI(TAG, "SR1 & SR2 update: %02X%02X -> %02X%02X (%04"PRIX32").", sr2_before, sr1_before, sr2_after, sr1_after, sr12_set);
    }
}

void xmc_check_lock_sr(bool permanent)
{
    uint32_t flash_id;
    uint8_t sfdp_06;
    uint8_t sfdp_f4;

    //These IDs are used to identify XMC flash chips with SR lock issue.
    xmc_read_flash_id(&flash_id, &sfdp_06, &sfdp_f4);

    if (!IS_XMC(flash_id)) {
        ESP_LOGI(TAG, "non-xmc (%06"PRIX32"). SR lock skipped.", flash_id);
        return;
    } else if (!XMC_NEEDS_SR_LOCK(sfdp_06, sfdp_f4)) {
        ESP_LOGI(TAG, "ver not match (%02X, %02X), SR lock skipped.", sfdp_06, sfdp_f4);
        return;
    }

    //See the header for the meaning for the value
    //Different flash chips have different default values for DRV1, DRV0, that's why good values are different.
    uint32_t good_value;
    if (flash_id == 0x204016) {
        good_value =  0x600200;
    } else if (flash_id == 0x204017) {
        good_value = 0x200200;
    } else if (flash_id == 0x204018) {
        good_value = 0x600200;
    } else {
        ESP_LOGE(TAG, "Unsupported XMC model: %06"PRIX32, flash_id);
        return;
    }

    xmc_lock_sr(good_value, permanent);
}
