# DP83TC HTML datasheet parser
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

import logging
import re
from html.parser import HTMLParser
from usb2mdio.htmlnode import *
from usb2mdio.reg import BitField as _BitField, Reg as _Reg


class BitField(_BitField):
	def parse(self, bit_start_stop, name, access, reset_value, description):
		# self.bit_start_stop = bit_start_stop
		self.name = name.strip()
		self.access = access.strip()
		self.description = description.replace("\n\n", "\n").strip()

		# parse reset value
		reset_value = reset_value.strip()
		if 'b' in reset_value:
			self.reset_value = int(reset_value[:-1], 2)
		else:
			try:
				self.reset_value = int(reset_value)
			except:
				self.reset_value = reset_value

		# Start and Stop bit size
		bit_start_stop = bit_start_stop.strip()
		if '-' in bit_start_stop:
			stop, start = bit_start_stop.split('-')
			self.start = int(start)
			self.stop = int(stop) #+ 1
		else:
			self.start = int(bit_start_stop)
			self.stop = self.start #+ 1
		self.bitlen = self.stop - self.start + 1

		# extract_values_from_description
		self.values = dict()
		re_value_bit = re.compile('([01]+)b ?[-=] ?(.+)')
		re_value_int = re.compile('([01]+) ?[-=] ?(.+)')
		for line in description.split("\n"):
			line = line.strip()
			m = re_value_bit.match(line)
			if m:
				value, desc = m.group(1), m.group(2)
				value = int(value, 2)
				self.values[value] = desc.strip()
			else:
				m = re_value_int.match(line)
				if m:
					value, desc = m.group(1), m.group(2)
					value = int(value)
					self.values[value] = desc.strip()


class Reg(_Reg):
	def parse(self, title):
		paragraph, name, _, _, _, address, _, _, reset_value = title.split()
		address = address[:-2]
		reset_value = reset_value[:-2]
		self.name = name
		self.address = int(address, 16)
		try:
			self.reset_value = int(reset_value, 16)
		except:
			self.reset_value = None
		self.bitfields = list()
		self.current_reg_value = None


class Table:
	def __init__(self, title):
		self.lines = list()
		self.title = title

	def append_current_line(self, element):
		self.lines[-1].append(element)

	def newline(self):
		self.lines.append(list())

	def extract_reg(self):
		"""Extract a register if the table format corresponds"""
		if len(self.lines) < 2:
			return None

		if self.lines[0] != ['Bit', 'Field', 'Type', 'Reset', 'Description']:
			logging.info(f"dropped {self.title}: {self.lines[0]}")
			return None

		try:
			reg = Reg().parse(self.title)
		except:
			logging.warning(f"Parse Error: Register {self.title} unrecocnized")
			return None

		for e in self.lines[1:]:
			reg.append_bitfield(BitField().parse(*e))
		return reg

	def __repr__(self):
		return f"{self.title}:\n" + "\n".join(f"{line}" for line in self.lines)


class HtmlDocParser(HTMLParser):
	def __init__(self):
		super().__init__()
		self.current_table = None
		self.register_tables = list()
		self.section_title = None

		# Data handling
		self.in_h2_section = False
		self.in_table_cell = False

	def handle_starttag(self, tag, attrs):
		# reset FSM
		self.in_h2_section = False

		# handle the different tags
		if tag == 'h2':  # register name, address and reset values are in h2 tags
			self.in_h2_section = True
		if tag == 'table':
			assert self.current_table == None
			self.current_table = Table(self.section_title)
		if tag == 'tr':  # new line
			self.current_table.newline()
		if tag in ['th', 'td']:
			self.in_table_cell = True
			self.current_cell_data = ""
		if tag == "span":
			# treat it like a line break
			if self.in_table_cell:
				self.current_cell_data += '\n'

	def handle_data(self, data):
		if self.in_h2_section:
			self.section = data
			self.in_h2_section = False
			self.section_title = data
		if self.in_table_cell:
			self.current_cell_data += data

	def handle_endtag(self, tag):
		if tag == 'table':
			self.register_tables.append(self.current_table)
			self.current_table = None
		if tag in ['th', 'td']:
			self.in_table_cell = True
			self.current_table.append_current_line(self.current_cell_data)


def parse_datasheet(path):
	with open(path, 'r') as f:
		html = f.read()

	parser = HtmlDocParser()
	parser.feed(html)
	regs = list()
	for table in parser.register_tables:
		reg = table.extract_reg()
		if reg:
			regs.append(reg)
	return parser.register_tables, regs
