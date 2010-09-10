
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
import re
import shutil
import subprocess
import traceback
import Environment


class Biblio:

	def __init__(self, editorpane, motion):
		self.editorpane = editorpane
		self.motion = motion
		self.compileout = None

	def detect_bibliography(self):
		content = self.editorpane.grab_buffer()
		bib = re.compile('\\\\bibliography{([^{}]*})')
		for elem in bib.findall(content):
			candidate = elem[:-1]
			if candidate[-4:] == ".bib":
				if self.check_valid_file(candidate):
					return True
			else:
				if self.check_valid_file(candidate + ".bib"):
					return True
		return False

	def check_valid_file(self, bibfile):
		if os.path.isfile(bibfile):
			if os.path.isabs(bibfile):
				self.bibdirname = os.path.dirname(bibfile) + "/"
				self.bibbasename = os.path.basename(bibfile)
				return True
			else:
				self.bibdirname = os.getcwd() + "/"
				self.bibbasename = bibfile
				return True
		return False
		
	def setup_bibliography(self):
		try:
			# cite is not a standard package
			# self.editorpane.insert_package("cite")
			bibtitle = self.bibbasename[:-4]
			shutil.copy2(self.bibdirname + self.bibbasename,
					Environment.tempdir + "/" + self.bibbasename)
			self.editorpane.insert_bib(self.bibdirname + self.bibbasename)
		except:
			print traceback.print_exc()
		return self.bibbasename, "N/A"

	def compile_bibliography(self, progressbar):
		self.motion.update_workfile()
		workfile = self.motion.workfile[:-4]
		self.motion.update_auxfile()
		bibcompile = subprocess.Popen('bibtex "%s"' \
				 % (workfile), cwd=Environment.tempdir, 
				shell=True, stdin=None, 
				stdout=subprocess.PIPE, stderr=None)
		bibcompile.wait()
		output = bibcompile.communicate()[0]
		self.editorpane.set_buffer_changed()
		progressbar.set_tooltip_text(output)
		if not "Database file #1" in output:
			return False
		else:
			return True


	def parse_entries(self, biblist):
		""" my dirty dirty parser"""
		refnr = 0
		bibfile = open(Environment.tempdir + "/" + self.bibbasename)
		bibstr = bibfile.read()
		entries = re.compile('(@article|@book|@booklet|@conference|@inbook|' \
			'@incollection|@inproceedings|@manual|@mastersthesis|@misc|' \
			'@phdthesis|@proceedings|@techreport|@unpublished)([^@]*)' \
			, re.DOTALL | re.IGNORECASE)
		for elem in entries.findall(bibstr):
			entry = elem[1]
			ident_exp = re.compile('{([^,]*)')		
			author_exp = re.compile('author\s*=\s*(.*)')
			title_exp = re.compile('[^book]title\s*=\s*(.*)')
			year_exp = re.compile('year\s*=\s*{?"?([1|2][0-9][0-9][0-9])}?"?')

			ident_res = ident_exp.findall(entry)[0]
			try: author_res = author_exp.findall(entry)[0]
			except: author_res = "????"
			try: title_res = title_exp.findall(entry)[0]
			except: title_res = "????"
			try: year_res = year_exp.findall(entry)[0]
			except: year_res = "????"

			author_fmt = re.sub("[{|}|\"|\,]", "", author_res)	
			title_fmt = re.sub("[{|}|\"|\,|\$]", "", title_res)
			year_fmt = year_res

			biblist.append([ident_res, title_fmt, author_fmt, year_fmt])
			refnr = refnr + 1
			
		return refnr


		



			
			

			
		
