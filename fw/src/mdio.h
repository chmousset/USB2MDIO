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


#if !defined(MDIO_H)
#define MDIO_H

#include <chmtx.h>


typedef struct MDIO_DRIVER {
	uint32_t clk_line;
	uint32_t dat_line;
    mutex_t mutex;
} mdio_t;

void mdio_init(void);

uint16_t mdio_read(unsigned int port_n, uint8_t phy_address, uint8_t reg_address);

void mdio_write(unsigned int port_n, uint8_t phy_address, uint8_t reg_address, uint16_t reg_value);

void cmd_mdio_port(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_mdio_scan(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_mdio_phy(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_mdio_r(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_mdio_w(BaseSequentialStream *chp, int argc, char *argv[]);
uint16_t mdio_extended_read(unsigned int mdio_port, uint8_t phy_id, uint16_t reg);
void mdio_extended_write(unsigned int mdio_port, uint8_t phy_id, uint16_t reg, uint16_t value);
void cmd_mdio_dump(BaseSequentialStream *chp, int argc, char *argv[]);

#define MDIO_CMD \
    {"mdio_scan", cmd_mdio_scan}, \
    {"mdio_phy", cmd_mdio_phy}, \
    {"mdio_port", cmd_mdio_port}, \
    {"mdio_r", cmd_mdio_r}, \
    {"mdio_w", cmd_mdio_w}, \
    {"mdio_dump", cmd_mdio_dump}, \


#endif // MDIO_H
