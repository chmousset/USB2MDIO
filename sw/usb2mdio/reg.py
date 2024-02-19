# PHY registers model
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
from htmlnode import *

class BitField:
	def __init__(self):
		self.bitlen = None
		self.start = None
		self.stop = None
		self.name = None
		self.access = None
		self.reset_value = None
		self.values = None

	def __repr__(self):
		return f"{self.start}-{self.stop}: {self.name}\n      "+ "\n      ".join(f"{k}: {v}" for k, v in self.values.items())

	def html(self, current_reg_value=None):
		html = HtmlTr()
		if self.bitlen == 1:
			html.append(HtmlTd(f"{self.start}"))
		else:
			html.append(HtmlTd(f"{self.start}-{self.stop}"))
		html.append(HtmlTd(self.name))
		html.append(HtmlTd(self.access))
		html.append(HtmlTd(self.reset_value))
		if current_reg_value is not None:
			current_value = current_reg_value >> self.start
			current_value &= (1 << self.bitlen) - 1
			current_value_description = self.values.get(current_value, "")
			if current_value_description != "":
				current_value_description = "<br>" + current_value_description
			current_value_cell = HtmlTd(f"0b{current_value:b}{current_value_description}")
			if current_value != self.reset_value:
				print(f"{self.name}: 0b{current_value:b} != {self.reset_value}")
				current_value_cell.props = {"class": "red"}
			html.append(current_value_cell)
		html.append(HtmlTd(self.description.replace('\n', "<br>")))
		return html

class Reg:
	def __init__(self):
		self.name = None
		self.address = None
		self.reset_value = None
		self.bitfields = list()
		self.current_reg_value = None

	def __repr__(self):
		bitfields = "\n  ".join(f"{bf}" for bf in self.bitfields)
		if self.reset_value is not None:
			return f"{self.name}[0x{self.address:X}] reset=0x{self.reset_value:X}\n  {bitfields}"
		return f"{self.name}[{self.address:X}] reset UNDEFINED\n  {bitfields}"

	def append_bitfield(self, bitfield):
		self.bitfields.append(bitfield)

	def set_value(self, value):
		self.current_reg_value = value

	def html(self):
		html = HtmlDiv()
		# Title
		if self.reset_value is None:
			reset_value = "UNDEFINED"
		else:
			reset_value = f"{self.reset_value:X}"
		if self.current_reg_value is not None:
			title = HtmlH1(f"{self.name}[0x{self.address:X}] = 0x{self.current_reg_value:X} (reset 0x{reset_value})")
			if self.current_reg_value != self.reset_value:
				title.props = {"class": "red"}
		else:
			title = HtmlH1(f"{self.name}[0x{self.address:X}] (reset {reset_value})")			
		html.append(title)

		# Table
		table = HtmlTable()
		header = HtmlTr()
		header.append(HtmlTh("Bit", {"style": "width:7%"}))
		header.append(HtmlTh("Field", {"style": "width:20%"}))
		header.append(HtmlTh("Type", {"style": "width:7%"}))
		header.append(HtmlTh("Reset", {"style": "width:7%"}))
		if self.current_reg_value is not None:
			header.append(HtmlTh("CurrentValue"))
		header.append(HtmlTh("Description"))
		table.append(header)
		for bf in self.bitfields:
			table.append(bf.html(self.current_reg_value))
		html.append(table)

		return html


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
			reg = Reg(self.title)
		except:
			logging.warning(f"Parse Error: Register {self.title} unrecocnized")
			return None

		for e in self.lines[1:]:
			reg.append_bitfield(BitField(*e))
		return reg

	def __repr__(self):
		return f"{self.title}:\n" + "\n".join(f"{line}" for line in self.lines)


def format_html(regs, path="regs.html"):
	html = HtmlHtml()
	head = HtmlHead()
	stylesheet = HtmlLink()
	stylesheet.props = {"rel":"stylesheet", "href":"pico.css"}
	head.append(stylesheet)
	body = HtmlBody()
	for reg in regs:
		body.append(reg.html())
	html.append(head)
	html.append(body)

	with open(path, "w") as f:
		f.write(f"{html}")
