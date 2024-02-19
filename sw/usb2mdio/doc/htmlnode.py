# Simple HTML generator
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

class HtmlNode:
	def __init__(self, data=None, props=dict()):
		self.props = props
		if data is not None:
			self.children = [data]
		else:
			self.children = list()

	def __repr__(self):
		props = " ".join(f"{k}=\"{v}\"" for k, v in self.props.items())
		return f"<{self.name} {props}>" + "".join(f"{c}" for c in self.children) + f"</{self.name}>"

	def append(self, child):
		self.children.append(child)


class HtmlTr(HtmlNode):
	name = "tr"


class HtmlTd(HtmlNode):
	name = "td"

class HtmlTh(HtmlNode):
	name = "th"


class HtmlTable(HtmlNode):
	name = "table"


class HtmlHtml(HtmlNode):
	name = "html"


class HtmlBody(HtmlNode):
	name = "body"


class HtmlHead(HtmlNode):
	name = "head"


class HtmlLink(HtmlNode):
	name = "link"


class HtmlH1(HtmlNode):
	name = "h1"


class HtmlP(HtmlNode):
	name = "p"


class HtmlDiv(HtmlNode):
	name = "div"

