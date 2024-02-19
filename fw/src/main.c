/*
   ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <ch.h>
#include <hal.h>

#include "usbcfg.h"

#include "stm32f072_bootloader.h"
#include "chprintf.h"
#include "shell.h"
#include "mdio.h"
#include "dp83tc813.h"
#include "board.h"


static const ShellCommand commands[] = {
    MDIO_CMD
    {NULL, NULL}
};

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)
static thread_t *shelltp = NULL;
char histbuffer[SHELL_HISTORY_BUFFER_SIZE];
char completion[4][32];

static const ShellConfig shell_cfg1 = {
    (BaseSequentialStream *)&SDU1,
    commands,
    histbuffer,
    SHELL_HISTORY_BUFFER_SIZE,
    completion
};


static void shell_setup(void)
{
    /* Disconnect from USB */
    usbDisconnectBus(serusbcfg.usbp);

    /* Start the USB serial interface */
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    /* Start USB */
    chThdSleepMilliseconds(100);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    // ChibiOS Shell
    cmd_mdio_scan(NULL, 0, NULL);
    shellInit();
    while (true) {
        if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE)) {
            shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
                                          "shell", NORMALPRIO + 1,
                                          shellThread, (void *)&shell_cfg1);
            chThdWait(shelltp);               /* Waiting termination.             */
        }
        chThdSleepMilliseconds(1000);
    }
}

int main(void) {
    halInit();
    chSysInit();
    mdio_init();  // init MDIO driver
    lateBoardInit();  // board-specific init (after OS init)

#if defined(APP_DP83TC813_START)
    APP_DP83TC813_START();  // polls DP83TC813 status reg to update LED to display link status
#endif

    shell_setup();
}
