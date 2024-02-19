// Management Data Input/Output protocol
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


#include <hal.h>
#include <ch.h>
#include <chprintf.h>

#include "mdio.h"
#include "chprintf.h"
#include "shell.h"

#if defined(LINE_MDIO_MDC_2)
#define MDIO_CNT 2
#else
#define MDIO_CNT 1
#endif

unsigned int phy_id = 0;
unsigned int mdio_port = 0;
static mdio_t MDIO_PORTS[MDIO_CNT] = {
    {.clk_line=LINE_MDIO_MDC_1, .dat_line=LINE_MDIO_MDIO_1},
#if defined(LINE_MDIO_MDC_2)
    {.clk_line=LINE_MDIO_MDC_2, .dat_line=LINE_MDIO_MDIO_2},
#endif
};


enum MODE {
    INT=0,
    HEX,
    BIN
};

static int32_t autoint(char *str) {
    int32_t value = 0;
    enum MODE mode = INT;
    while(*str) {
        if(*str == ' ') {
            return value;
        }
        if(mode == INT) {
            if(*str >= '0' && *str <= '9') {
                value = value*10 + *str - '0';
            }
            else if(*str == 'x') {
                mode = HEX;
            }
            else if(*str == 'X') {
                mode = HEX;
            }
            else if(*str == 'h') {
                mode = HEX;
            }
            else if(*str == 'H') {
                mode = HEX;
            }
            else if(*str == 'b') {
                mode = BIN;
            }
            else if(*str == 'B') {
                mode = BIN;
            }
        }
        else if(mode == HEX) {
            if(*str >= '0' && *str <= '9') {
                value = value << 4;
                value |= *str - '0';
            }
            else if(*str >= 'A' && *str <= 'F') {
                value = value << 4;
                value |= *str - 'A';
            }
            else if(*str >= 'a' && *str <= 'f') {
                value = value << 4;
                value |= *str - 'a';
            }
            else {
                return value;
            }
        }
        else if(mode == BIN) {
            if(*str == '0') {
                value = value << 1;
            }
            else if(*str == '1') {
                value = value << 1;
                value |= 1;
            }
            else {
                return value;
            }
        }

        str++;
    }
    return value;
}

static void mdio_tbit4(void) {
    // chThdSleepMicroseconds(1);
    volatile int i;
    while(i=0, i<10, i++) {
        asm("nop");
    }
}

static int mdio_bit(mdio_t *mdio, int bit, bool read) {
    int bit_read;

    // read operation require to set the dat line as input
    if(read) {
        palSetLineMode(mdio->dat_line, PAL_MODE_INPUT);
    }
    else {
        palSetLineMode(mdio->dat_line, PAL_MODE_OUTPUT_PUSHPULL);
    }

    // bit setup
    palWriteLine(mdio->dat_line, bit);
    mdio_tbit4();
    // clock rising edge
    bit_read = palReadLine(mdio->dat_line);
    palSetLine(mdio->clk_line);
    // bit read
    mdio_tbit4();
    // clock falling edge
    palClearLine(mdio->clk_line);
    // mdio_tbit4();

    if(bit_read) {
        return 1;
    }
    return 0;
}


static void mdio_preamble(mdio_t *mdio) {
    chThdSleepMicroseconds(100);
    for(int i=0; i<32; i++) {
        mdio_bit(mdio, 1, false);
    }
    chThdSleepMicroseconds(10);
}

static void mdio_start_read(mdio_t *mdio) {
    mdio_preamble(mdio);
    mdio_bit(mdio, 0, false);
    mdio_bit(mdio, 1, false);
    mdio_bit(mdio, 1, false);
    mdio_bit(mdio, 0, false);
}

static void mdio_start_write(mdio_t *mdio) {
    mdio_preamble(mdio);
    mdio_bit(mdio, 0, false);
    mdio_bit(mdio, 1, false);
    mdio_bit(mdio, 0, false);
    mdio_bit(mdio, 1, false);
}


static void mdio_turnaround(mdio_t *mdio) {
    mdio_bit(mdio, 1, true);
    mdio_bit(mdio, 0, true);
}


static void mdio_send_address(mdio_t *mdio, uint8_t phy_address, uint8_t reg_address) {
    for(int i=0; i<5; i++) {
        mdio_bit(mdio, phy_address >> 4, false);
        phy_address = phy_address << 1;
    }
    for(int i=0; i<5; i++) {
        mdio_bit(mdio, reg_address >> 4, false);
        reg_address = reg_address << 1;
    }
}

uint16_t mdio_read(unsigned int port, uint8_t phy_address, uint8_t reg_address) {
    if(port >= 2) {
        return 0;
    }
    chMtxLock(&MDIO_PORTS[port].mutex);
    mdio_start_read(&MDIO_PORTS[port]);
    mdio_send_address(&MDIO_PORTS[port], phy_address, reg_address);
    mdio_turnaround(&MDIO_PORTS[port]);

    uint16_t reg_value = 0;
    for(int i=0; i<16; i++) {
        reg_value = reg_value << 1;
        reg_value |= mdio_bit(&MDIO_PORTS[port], 1, true);
    }
    chMtxUnlock(&MDIO_PORTS[port].mutex);
    return reg_value;
}


uint16_t mdio_extended_read(unsigned int mdio_port, uint8_t phy_id, uint16_t reg) {
    if(reg < 32) {
        return mdio_read(mdio_port, phy_id, reg);
    }
    else if(reg <= 0xEFD) {  // MMD1F
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x1F);
        (void)mdio_write(mdio_port, phy_id, 0xE, reg);
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x401F);
        return mdio_read(mdio_port, phy_id, 0xE);
    }
    else if(reg >= 0x1000) {  // MMD1
        (void)mdio_write(mdio_port, phy_id, 0xD, reg >> 12);
        (void)mdio_write(mdio_port, phy_id, 0xE, reg & 0x0FFF);
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x4001);
        return mdio_read(mdio_port, phy_id, 0xE);
    }
    else {
        return 0;
    }
}


void mdio_write(unsigned int port, uint8_t phy_address, uint8_t reg_address, uint16_t reg_value) {
    if(port >= 2) {
        return;
    }
    chMtxLock(&MDIO_PORTS[port].mutex);
    mdio_start_write(&MDIO_PORTS[port]);
    mdio_send_address(&MDIO_PORTS[port], phy_address, reg_address);
    mdio_turnaround(&MDIO_PORTS[port]);

    for(int i=0; i<16; i++) {
        reg_value |= mdio_bit(&MDIO_PORTS[port], reg_value>>15, false);
        reg_value = reg_value << 1;
    }
    chMtxUnlock(&MDIO_PORTS[port].mutex);
}


void mdio_extended_write(unsigned int mdio_port, uint8_t phy_id, uint16_t reg, uint16_t value) {
    if(reg < 32) {
        (void)mdio_write(mdio_port, phy_id, reg, value);
    }
    else if(reg <= 0xEFD) {  // MMD1F
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x1F);
        (void)mdio_write(mdio_port, phy_id, 0xE, reg);
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x401F);
        (void)mdio_write(mdio_port, phy_id, 0xE, value);
    }
    else if(reg >= 0x1000) {  // MMD1
        (void)mdio_write(mdio_port, phy_id, 0xD, reg >> 12);
        (void)mdio_write(mdio_port, phy_id, 0xE, reg & 0x0FFF);
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x4001);
        (void)mdio_write(mdio_port, phy_id, 0xE, value);
    }
    else {
        return;
    }
}

void cmd_mdio_port(BaseSequentialStream *chp, int argc, char *argv[]) {
    if(argc != 1) {
        chprintf(chp, "usage: mdio_port <phy number>   : select MDIO port (0/1)\r\n");
        return;
    }
    mdio_port = autoint(argv[0]);
    if(mdio_port > 1) {
        chprintf(chp, "invalid MDIO port (>1)\r\n");
        mdio_port = 0;
        return;
    }
    chprintf(chp, "selected MDIO port %d\r\n", mdio_port);
}

void cmd_mdio_scan(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    (void)argc;
    // read reg 0 value for each PHY ID between 0 and 15 and display the result
    for(int port=0; port<2; port++) {
        if(chp) {
            chprintf(chp, "port %d:\r\n", port);
        }
        for(int i=0; i<16; i++) {
            uint16_t reg_value = mdio_read(port, i, 2);  // ID register
            if(chp) {
                chprintf(chp, "PHY %d: reg_0= %04X (%d)\r\n", i, reg_value, reg_value);
            }
        }
    }
}

void cmd_mdio_phy(BaseSequentialStream *chp, int argc, char *argv[]) {
    if(argc != 1) {
        chprintf(chp, "usage: mdio_phy <phy number>   : select PHY address\r\n");
        return;
    }
    phy_id = autoint(argv[0]);
    if(phy_id > 15) {
        chprintf(chp, "invalid PHY ID (>15)\r\n");
        phy_id = 0;
        return;
    }
    chprintf(chp, "selected PHY ID %d\r\n", phy_id);
}


void cmd_mdio_r(BaseSequentialStream *chp, int argc, char *argv[]) {
    if(argc != 1) {
        chprintf(chp, "usage: mdio_read <register>    : read register\r\n");
        return;
    }
    int reg = autoint(argv[0]);
    uint16_t reg_value;

    if(reg < 31) {
        reg_value = mdio_read(mdio_port, phy_id, reg);
    }
    else if(reg <= 0xEFD) {  // MMD1F
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x1F);
        (void)mdio_write(mdio_port, phy_id, 0xE, reg);
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x401F);
        reg_value = mdio_read(mdio_port, phy_id, 0xE);
    }
    else if(reg <= 0xFFFF && reg >= 0x1000) {  // MMD1
        (void)mdio_write(mdio_port, phy_id, 0xD, reg >> 12);
        (void)mdio_write(mdio_port, phy_id, 0xE, reg & 0x0FFF);
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x4001);
        reg_value = mdio_read(mdio_port, phy_id, 0xE);
    }
    else {
        chprintf(chp, "invalid reg ID\r\n");
        return;
    }
    chprintf(chp, "reg_%01X(%d)= %04X (%d)\r\n", reg, reg, reg_value, reg_value);
}


void cmd_mdio_dump(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    if(argc != 0) {
        chprintf(chp, "usage: mdio_dump    : read all 32 registers from a single PHY\r\n");
        return;
    }
    for(int reg=0; reg<32; reg++) {
        uint16_t reg_value = mdio_read(mdio_port, phy_id, reg);
        chprintf(chp, "reg_%01X(%d)= %04X (%d)\r\n", reg, reg, reg_value, reg_value);
    }
}

void cmd_mdio_w(BaseSequentialStream *chp, int argc, char *argv[]) {
    if(argc != 2) {
        chprintf(chp, "usage: mdio_read <register> <value>    : write register\r\n");
        return;
    }
    int reg = autoint(argv[0]);
    uint16_t value = autoint(argv[1]);

    if(reg < 32) {
        (void)mdio_write(mdio_port, phy_id, reg, value);
        chprintf(chp, "reg_%01X(%d) written\r\n", reg, reg);
    }
    else if(reg <= 0xEFD) {  // MMD1F
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x1F);
        (void)mdio_write(mdio_port, phy_id, 0xE, reg);
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x401F);
        (void)mdio_write(mdio_port, phy_id, 0xE, value);
        chprintf(chp, "reg_%01X(%d) written(2)\r\n", reg, reg);
    }
    else if(reg <= 0xFFFF && reg >= 0x1000) {  // MMD1
        (void)mdio_write(mdio_port, phy_id, 0xD, reg >> 12);
        (void)mdio_write(mdio_port, phy_id, 0xE, reg & 0x0FFF);
        (void)mdio_write(mdio_port, phy_id, 0xD, 0x4001);
        (void)mdio_write(mdio_port, phy_id, 0xE, value);
        chprintf(chp, "reg_%01X(%d) written(3)\r\n", reg, reg);
    }
    else {
        chprintf(chp, "invalid reg ID (>15)\r\n");
        return;
    }
}


void mdio_init(void) {
    // init the Mutex
    for(int i=0; i<MDIO_CNT; i++) {
        chMtxObjectInit(&MDIO_PORTS[i].mutex);
    }
}

