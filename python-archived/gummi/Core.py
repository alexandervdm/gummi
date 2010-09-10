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

from gtk import Builder
import os
import sys
import traceback

from Biblio import Biblio
import Environment
from GummiGUI import MainGUI
from IOFunctions import IOFunctions
from Motion import Motion
from Preferences import Preferences
from PreviewPane import PreviewPane
from TexPane import TexPane


class Core:
	
	def __init__(self):

		# environment: 
		self.filename = None

		# graphical components:
		builder = Builder()
		builder.set_translation_domain("gummi")
		builder.add_from_file(Environment.gummi_glade)

		# class instances:
		config = Preferences()
		editorpane = TexPane(config)
		previewpane = PreviewPane(builder)
		motion = Motion(config, editorpane, previewpane, builder)
		iofunc = IOFunctions(config, editorpane, motion, builder)
		biblio = Biblio(editorpane, motion)
		gui = MainGUI \
			(config, builder, iofunc, biblio, motion, editorpane, previewpane)
		editorpane.set_motion(motion)

		# setup document to load:
		if len(sys.argv) > 1:
			self.filename = sys.argv[1]
			iofunc.load_file(self.filename)
		else:
			self.filename = None
			iofunc.load_default_text()
		iofunc.make_environment(self.filename)

		# gui main:
		gui.main()

try:
	instance = Core()
except:
	print traceback.print_exc()
