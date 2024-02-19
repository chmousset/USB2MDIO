// dp83tc813 handling code
// Copyright (C) 2024 Charles-Henri Mousset <ch.mousset@gmail.com>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <ch.h>
#include <hal.h>
#include "dp83tc813.h"

// LED driver thread. It reads the DP83TC813 link status register to display link up on the LED
static THD_WORKING_AREA(waThreadLed, 256);
static THD_FUNCTION(ThreadLed, arg) {
    (void)arg;
    chRegSetThreadName("LED");

    int btn_cnt = 0;

    while (true) {
        if(palReadLine(LINE_BUTTON)) {
            btn_cnt++;
        }
        else {
            btn_cnt = 0;
        }
        if(btn_cnt == 50) {
            for(int i=0; i<5; i++) {
                palSetLine(LINE_LED);
                chThdSleepMilliseconds(200);
                palClearLine(LINE_LED);
                chThdSleepMilliseconds(200);
            }
            while(palReadLine(LINE_BUTTON)) {  // wait for the button to be released
                chThdSleepMilliseconds(20);
            }

            uint16_t new_val = mdio_extended_read(1, 0, 0x1834) ^ (uint16_t)0x4000;
            mdio_extended_write(1, 0, 0x1834, new_val);

            btn_cnt = 0;
        }

        uint16_t reg_value = mdio_extended_read(1, 0, 0x01);
        bool link_up = reg_value & (1 << 2);

        if(link_up) {
            palSetLine(LINE_LED);
        }
        else {
            palClearLine(LINE_LED);
        }
        chThdSleepMilliseconds(20);
    }
}

void _app_dp83tc813_start(void) {
    chThdCreateStatic(waThreadLed, sizeof(waThreadLed), NORMALPRIO, ThreadLed, NULL);
}
