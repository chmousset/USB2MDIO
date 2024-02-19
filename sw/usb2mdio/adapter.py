# Handles serial communication with adapter
# Copyright (C) 2024 Charles-Henri Mousset <ch.mousset@gmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import serial

class UsbMdio:
	endline = "\r\n"
	def __init__(self, serial_path="/dev/ttyACM0"):
		self.serial = serial.Serial(serial_path, timeout=0.2)
		self.command("")
		while self.serial.read():
			pass  # empty buffer

	def command(self, command, get_response=True):
		self.serial.write((command + self.endline).encode('utf-8'))
		_ = self.serial.readline()  # echo
		resp = self.serial.readline().strip()
		logging.info(f"Command [{command}] => {resp}")
		return resp

	def write_reg(self, reg, value):
		self.command(f"mdio_w {reg} {value}")

	def read_reg(self, reg):
		response = self.command(f"mdio_r {reg}")
		if response is None:
			return None
		print(response.split())
		_, value, _ = response.split()
		return int(value, 16)

	def select(self, port, phy_id):
		self.command(f"mdio_port {port}")
		self.command(f"mdio_phy {phy_id}")

	def __del__(self):
		self.serial.close()
