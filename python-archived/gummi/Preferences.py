#!/usr/bin/python
# -*- encoding: utf-8 -*-

LICENSE="""
Copyright (c) 2009 Alexander van der Mey <alexvandermey@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
"""

import os
import Environment
import ConfigParser

VERSION = "svn"
UPDATEURL = "http://dev.midnightcoding.org/redmine/projects/" + \
			"gummi/repository/raw/trunk/dev/latest"

CFGDEFAULTS = {
'line_numbers': True,
'highlighting': True,
'textwrapping': True,
'wordwrapping': True,
'spelling': False,
'spell_language': None,
'toolbar': True,
'statusbar': True,
'rightpane': True,
'font': 'Monospace 10',
'autosaving': False,
'autosave_timer': 600,
'typesetter': 'pdflatex',
'compile_scheme': 'on_idle',
'compile_timer': 1,
'compile_status': True,
'idle_threshold': 2000,
'recent1': '',
'recent2': '',
'recent3': '',
'welcome': '''\documentclass{article}
\\begin{document}

\\begin{center}
\Huge{Welcome to Gummi} \\\\\\
\\\\
\\LARGE{You are using the ''' + VERSION + ''' version.\\\\
I welcome your suggestions at\\\\
http://gummi.midnightcoding.org}\\\\
\\end{center}

\\end{document}
'''
}

class Preferences:

	def __init__(self):
		self.config = ConfigParser.RawConfigParser(CFGDEFAULTS)
		self.cfgpath = Environment.HOME + "/.config/gummi/gummi.cfg"

		if not os.path.exists(os.path.dirname(self.cfgpath)):
			os.makedirs(os.path.dirname(self.cfgpath))

		try:
			self.config.readfp(open(self.cfgpath))
			if not self.config.has_section("bibtex_files"):
				self.config.add_section("bibtex_files")
		except:
			print "Could not load configuration, setting defaults.."
			cfgfile = open(self.cfgpath, 'w')
			self.config.write(cfgfile)
			cfgfile.close()


	def get_value(self, section, option):
		if not self.config.has_section(section):
			self.config.add_section(section)
			self.write_config_change()
		if not self.config.has_option(section, option):
			defvalue = self.config.get('DEFAULT', option)
			self.config.set(section, option, defvalue)
			self.write_config_change()
		result = self.config.get(section, option)
		if result == 'True': # this is silly.. 
			return True
		elif result == 'False':
			return False
		else:
			return result

	def set_value(self, section, option, value):
		if not self.config.has_section(section):
			self.config.add_section(section)
		self.config.set(section, option, value)
		self.write_config_change()

	def reset_section(self, section):
		self.config.remove_section(section)
		self.write_config_change()

	def write_recentfiles(self, recent1, recent2, recent3):
		self.set_value("recent_files", "recent1", recent1)
		self.set_value("recent_files", "recent2", recent2)
		self.set_value("recent_files", "recent3", recent3)

	def write_config_change(self):
			cfgfile = open(self.cfgpath, 'w')
			self.config.write(cfgfile)
			cfgfile.close()






			





	
