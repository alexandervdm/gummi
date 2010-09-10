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


ARTICLE = """\\documentclass{article}
\\author{[YOUR NAME]\\\\
\\texttt{[YOUR EMAIL]}
}
\\title{[TITLE OF YOUR ARTICLE]}
\\begin{document}
\\maketitle
\\dots

\\end{document}
"""

BOOK = """\\documentclass[12pt]{book}

\\begin{document}

\\chapter*{\\Huge \\center [BOOKTITLE] }
\\thispagestyle{empty}
\\section*{\\huge \\center [AUTHOR]}
\\newpage

\\subsection*{\\center \\normalsize Copyright \\copyright [YEAR] [NAME]}
\\subsection*{\\center \\normalsize All rights reserved.}
\\subsection*{\\center \\normalsize ISBN \\dots}
\\subsection*{\\center \\normalsize Publications}

\\tableofcontents

\\mainmatter
\\chapter{[CHAPTER1-TITLE]}
\\dots
\\chapter{[CHAPTER2-TITLE]}
\\dots
\\backmatter

\\end{document}
"""

LETTER = """\\documentclass{letter}

\\signature{[YOURNAME]}
\\address{[YOURADDRESS]}

\\begin{document}
\\begin{letter}{Company name \\\\ Street\\\\ City\\\\ Country}
\\opening{[HEADING]}

\\dots\\\\\\dots\\\\\\dots\\\\\\dots\\\\\\dots

\\closing{[CLOSING]}
\\end{letter}
\\end{document}
"""

REPORT = """\\documentclass[]{report}
\\begin{document}

\\title{[YOUR TITLE]}
\\author{[YOUR NAME]}
\\maketitle

\\chapter{[CHAPTERTITLE]}
\\section{Introduction}
\\dots
\\chapter{[CHAPTERTITLE]}
\\section{Introduction}
\\dots
\\subsection{Subsection}
\\end{document}
"""

class Template:


	# TODO add template options (multiple columns support etc.)
	def __init__(self, builder):
		self.templatewindow = builder.get_object("templatewindow")
		self.template_ok = builder.get_object("template_ok")

		self.iconview = builder.get_object("templateicons")
		self.iconview.set_text_column(0)
		self.iconview.set_pixbuf_column(1)
		self.iconview.connect("selection-changed", self.update_window)

		self.templatewindow.show_all()

	def update_window(self, event):
		self.template_ok.set_sensitive(1)

	def get_template(self):
		try:
			selection = self.iconview.get_selected_items()[0]
			selectedtemplate = selection[0]
			if selectedtemplate == 0: return ARTICLE
			elif selectedtemplate == 1: return BOOK
			elif selectedtemplate == 2:	return LETTER
			elif selectedtemplate == 3:	return REPORT
		except IndexError: return None







