/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* SR lock Example (currently only for XMC)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "xmc_lock_sr.h"

void app_main(void)
{
    /*
     * Call this at the beginning of your app.
     * - If permanent==false, the SR will be locked after this line is executed and until power down.
     * - If permanent==true, the SR will be locked to a specific value forever.
     *          This is irreversible even after power down.
     *          Please do read readme before doing this.
     *
     * CAUTION: This may avoid further usage of HFM mode (> 80Mhz). Only call this when you are sure this device will not run above 80MHz.
     */
    xmc_check_lock_sr(false);

    printf("XMC SR lock applied. Here goes your application.\n");
    while (1) {
        vTaskDelay(1);
    }
}
