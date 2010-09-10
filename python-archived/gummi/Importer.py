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

import os

CURRENT = "CURRENT"
LINE = "\hline\n"

class Importer:

	def __init__(self, editorpane, builder):
		self.editorpane = editorpane
		self.import_tabs = builder.get_object("import_tabs")

		self.image_pane = builder.get_object("image_pane")
		self.image_file = builder.get_object("image_file")
		self.image_caption = builder.get_object("image_caption")
		self.image_label = builder.get_object("image_label")
		self.image_scale = builder.get_object("image_scale")
		self.scaler = builder.get_object("image_scaler")

		self.table_pane = builder.get_object("table_pane")
		self.table_comboalign = builder.get_object("table_comboalign")
		self.table_comboborder = builder.get_object("table_comboborder")
		self.table_rows = builder.get_object("table_rows")
		self.table_cols = builder.get_object("table_cols")

		self.matrix_rows = builder.get_object("matrix_rows")
		self.matrix_cols = builder.get_object("matrix_cols")
		self.matrix_combobracket = builder.get_object("matrix_combobracket")

		self.table_cols.set_value(3)
		self.table_rows.set_value(3)
		self.matrix_cols.set_value(3)
		self.matrix_rows.set_value(3)

	def insert_image(self, imagefile):
		if imagefile is not None and os.path.exists(imagefile):
			self.editorpane.insert_package("graphicx")
			f = self.image_file.get_text()
			s = self.scaler.get_value()
			c = self.image_caption.get_text()
			l = imagefile
			code = self.generate_image(f, s, c, l)
			position = self.editorpane.get_iterator(CURRENT)
			self.editorpane.editorbuffer.insert(position, code)
			self.editorpane.set_buffer_changed()
		self.activate_imagegui("", 0)
		self.import_tabs.set_current_page(0)

	def activate_imagegui(self, image, mode):
		self.image_label.set_sensitive(mode)
		self.image_scale.set_sensitive(mode)
		self.image_caption.set_sensitive(mode)
		self.image_file.set_text(image)
		self.image_caption.set_text("")
		self.image_label.set_text("")
		self.scaler.set_value(1.00)

	def insert_table(self):
		position = self.editorpane.get_iterator(CURRENT)
		code = self.generate_table(self.table_rows.get_value(), self.table_cols.get_value())
		position = self.editorpane.get_iterator(CURRENT)
		self.editorpane.editorbuffer.insert(position, code)
		self.editorpane.set_buffer_changed()
		self.import_tabs.set_current_page(0)

	def insert_matrix(self):
		self.editorpane.insert_package("amsmath")
		position = self.editorpane.get_iterator(CURRENT)
		code = self.generate_matrix(self.matrix_rows.get_value(), self.matrix_cols.get_value())
		position = self.editorpane.get_iterator(CURRENT)
		self.editorpane.editorbuffer.insert(position, code)
		self.editorpane.set_buffer_changed()
		self.import_tabs.set_current_page(0)

	def generate_table(self, rows, columns):
		table = ""
		rows = int(rows) + 1
		columns = int(columns) + 1
		borders = self.table_comboborder.get_active()
		# get column with l/c/r directly?
		alignment = self.table_comboalign.get_active()
		if alignment is 0: aligntype = "l"
		elif alignment is 1: aligntype = "c"
		else: aligntype = "r"
		if borders is 2:
				aligntype = aligntype + "|"
		for f in range(1,columns):
				align = f * aligntype
		if borders is 1: align = "|" + align + "|"
		elif borders is 2: align = "|" + align
		begin_tabular = "\\begin{tabular}{" + align + "}\n"
		end_tabular = "\\end{tabular}\n"
		for k in range(1, rows):
			for i in range(1,columns):
				if i == (columns-1): 
					new = str(k) + str(i) + "\\\\ \n"
					if borders is 2: new = new + LINE
				elif i == 1: new = "\t" + str(k) + str(i) + " & "
				else: new = str(k) + str(i) + " & "
				table = table + new
		if borders is 1: table = LINE + table + LINE
		if borders is 2: table = LINE + table
		return begin_tabular + table + end_tabular

	def generate_image(self, imagefile,  scale, caption, label):
		caption = self.image_caption.get_text()
		label = self.image_label.get_text()
		begin = "\\begin{figure}[htp]\n\\centering\n"
		end = "\\end{figure}"
		include = "\\includegraphics"
		scale = "[scale=" + str(scale) + "]"
		filename = "{" + imagefile + "}\n"
		if self.image_caption.get_text() is not "":
			caption = "\\caption{" + caption + "}\n"
		if self.image_label.get_text() is not "":
			label = "\\label{" + label + "}\n"	
		return begin + include + scale + filename + caption + label + end

	def generate_matrix(self, rows, columns):
		bracket = self.matrix_combobracket.get_active()
		if bracket == 0: mode = "matrix"
		elif bracket == 1: mode = "pmatrix"
		elif bracket == 2: mode = "bmatrix"
		elif bracket == 3: mode = "Bmatrix"
		elif bracket == 4: mode = "vmatrix"
		elif bracket == 5: mode = "Vmatrix"
		matrix = ""
		rows = int(rows) + 1
		columns = int(columns) + 1
		begin_matrix = "$\\begin{" + mode + "}\n"
		end_matrix = "\\end{" + mode +"}$\n"
		for k in range(1, rows):
			for i in range(1,columns):
				if i == (columns-1): new = str(k) + str(i) + "\\\\ \n"
				elif i == 1: new = "\t" + str(k) + str(i) + " & "
				else: new = str(k) + str(i) + " & "
				matrix = matrix + new
		return begin_matrix + matrix + end_matrix










