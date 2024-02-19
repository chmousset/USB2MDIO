# Documentation parsing helpers
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

from usb2mdio.htmlnode import *
from usb2mdio.doc.dp83tc813 import parse_datasheet as dp83tc813_parse_datasheet


def format_html(regs, path="regs.html"):
	html = HtmlHtml()
	head = HtmlHead()
	stylesheet = HtmlLink()
	# stylesheet.props = {"rel":"stylesheet", "href":"pico.css"}
	stylesheet.props = {"rel":"stylesheet", "href":"https://cdn.jsdelivr.net/npm/@picocss/pico@1/css/pico.min.css"}
	head.append(stylesheet)
	body = HtmlBody()
	for reg in regs:
		body.append(reg.html())
	html.append(head)
	html.append(body)

	with open(path, "w") as f:
		f.write(f"{html}")


if __name__ == "__main__":
	tables, regs = dp83tc813_parse_datasheet("DP83TC813_registers.html")
	format_html(regs, "DP83TC813_registers_parsed.html")
