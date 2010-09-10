#!/usr/bin/python
# -*- encoding: utf-8 -*-

# Copyright (c) 2009 Alexander van der Mey <alexvandermey@gmail.com>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

from gtk import Buildable

STYLES = ["tool_bold", "tool_italic", "tool_unline"]
ALLIGN = ["tool_left", "tool_center", "tool_right"]


class Formatting:

	def __init__(self, widget, editorbuffer):
		try:
			self.buffer = editorbuffer
			self.buffer.begin_user_action()
			ins, end = self.buffer.get_selection_bounds()
			begintag, endtag = self.get_style_commands(widget)
			if self.check_duplicate_tags(ins, end, begintag):
				self.buffer.end_user_action()
			else:
				self.buffer.insert(ins, begintag)
				end = self.buffer.get_selection_bounds()[1]
				self.buffer.insert(end, endtag)
				ins = self.buffer.get_selection_bounds()[0]
				ins.backward_chars(len(begintag))
				self.buffer.select_range(ins, end)
				self.buffer.end_user_action()
		except (IndexError, ValueError):
			return 	# will silenty pass if no text selected
					# may replace this with get_selection_bound check


	def check_duplicate_tags(self, ins, end, tag):
		if tag in self.buffer.get_slice(ins, end):
			return True
		else: return False

	def get_style_commands(self, widget):
		caller = Buildable.get_name(widget)
		begintag = ""; endtag = ""
		if caller in STYLES:
			if caller == "tool_bold": begintag = "\\textbf{"
			elif caller == "tool_italic": begintag = "\\textit{"
			elif caller == "tool_unline": begintag = "\\underline{"
			endtag = "}"
		elif caller in ALLIGN:
			if caller == "tool_left": cmd = "flushleft"
			elif caller == "tool_center": cmd = "center"
			elif caller == "tool_right": cmd = "flushright"
			begintag = "\\begin{" + cmd + "}"
			endtag = "\\end{" + cmd + "}"
		return begintag, endtag





