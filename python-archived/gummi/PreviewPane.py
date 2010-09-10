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

from __future__ import division # needed for proper int devision

import gtk
import os
import poppler
from ctypes import *

import Environment

# TODO: Documentation

class PreviewPane:

	def __init__(self, builder, pdffile=None):

		self.libgobject = self.setlibgobject()
		if self.libgobject is not None:
			print "libgobject was detected at " + self.libgobject
		
		self.drawarea = builder.get_object("preview_drawarea")
		self.toolbar = builder.get_object("preview_toolbar")
		self.prev = builder.get_object("preview_prev")
		self.next = builder.get_object("preview_next")
		self.pageinput = builder.get_object("page_input")
		self.pagelabel = builder.get_object("page_label")
		self.zoomcombo = builder.get_object("zoomcombo")
		self.scrollw = builder.get_object("preview_scroll")

		self.drawarea.connect("expose-event", self.on_expose)
		self.prev.connect("clicked", self.prev_page)
		self.next.connect("clicked", self.next_page)
		self.pageinput.connect("activate", self.page_input)
		self.zoomcombo.connect("changed", self.zoom_combo)

		# TODO: get this color from the gtk-theme?
		self.drawarea.modify_bg(gtk.STATE_NORMAL,
								gtk.gdk.color_parse('#edeceb'))

		self.page_total = 0
		self.current_page = 0
		self.scale = 1.0
		self.best_fit = False
		self.fit_width = True

		self.page_ratio = None # init now so we use them later in on_expose
		self.page_width = None # to check if have been set (succesful refresh)

	def unref_object(self, object):
		if self.libgobject is not None:
			try:
				glib = CDLL(self.libgobject)
				glib.g_object_unref(hash(object))
				del object
			except AttributeError: pass

	def set_pdffile(self, pdffile):
		if pdffile:
			self.pdffile = pdffile
			self.current_page = 0 #reset to 0 on new pdf load

			uri = self.geturi(pdffile)
			try:
				document = poppler.document_new_from_file(uri, None)		
			except:
				return False			
			self.page_total = document.get_n_pages()

			self.pagelabel.set_text('of ' + str(self.page_total))
			self.pageinput.set_text(str(self.current_page + 1))
			# TODO: determine page_width/page_height per page
			page = document.get_page(self.current_page)
			self.page_width, self.page_height = page.get_size()
			self.page_ratio = self.page_width / self.page_height
			self.goto_page(0)
			self.unref_object(document)
			self.unref_object(page)
			return True
		else: # unnecessary?
			self.pdffile = None
			return True

	def refresh_preview(self):
		if not os.path.exists(self.pdffile):
			print "can't refresh without a pdf file!"
			return
		uri = self.geturi(self.pdffile)
		document = poppler.document_new_from_file(uri, None)
		self.page_total = document.get_n_pages()
		if self.page_total - 1 > self.current_page:
			self.next.set_sensitive(True)
		elif self.current_page >= self.page_total:
			self.goto_page(self.page_total - 1)
		self.pagelabel.set_text('of ' + str(self.page_total))
		self.pageinput.set_text(str(self.current_page + 1))
		page = document.get_page(self.current_page)
		self.page_width, self.page_height = page.get_size()
		self.page_ratio = self.page_width / self.page_height
		self.drawarea.queue_draw()
		self.unref_object(document)
		self.unref_object(page)

	def on_expose(self, drawarea, data):
		cr = drawarea.window.cairo_create()		
		vp_size = self.scrollw.get_allocation()

		view_height = vp_size.height
		view_width = vp_size.width
		view_ratio = view_width / view_height

		# do not attempt any of the following if these two values have not
		# been set, because that means set_pdffile and refresh_preview have
		# not been succesfull so far. 
		if self.page_ratio is None or self.page_width is None: return

		if (self.best_fit or self.fit_width) and not self.page_ratio is None:
			self.scrollw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
			if view_ratio < self.page_ratio or self.fit_width:
				self.scale = view_width / self.page_width
			else:
				self.scale = view_height / self.page_height

		if not (self.best_fit or self.fit_width):
			self.scrollw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
			self.drawarea.set_size_request(int(self.page_width * self.scale),
			                               int(self.page_height * self.scale))
		elif self.fit_width:
			if abs(self.page_ratio - view_ratio) > 0.01:
				self.drawarea.set_size_request(-1, int(self.page_height *
				                                       self.scale))
		elif self.best_fit:
			self.drawarea.set_size_request(-1,
			                              int(self.page_height*self.scale)-10)

		cr.scale(self.scale, self.scale)

		cr.set_source_rgb(1, 1, 1)
		cr.rectangle(0, 0, self.page_width, self.page_height)
		cr.fill()
		uri = self.geturi(self.pdffile)
		document = poppler.document_new_from_file(uri, None)
		page = document.get_page(self.current_page)
		page.render(cr)
		self.unref_object(document)
		self.unref_object(page)


	def goto_page(self, page):
		if page < 0 or page >= self.page_total:
			return

		self.current_page = page
		self.prev.set_sensitive(page > 0)
		self.next.set_sensitive(page < self.page_total - 1)
		self.pageinput.set_text(str(self.current_page + 1))
		uri = self.geturi(self.pdffile)
		document = poppler.document_new_from_file(uri, None)
		page = document.get_page(self.current_page)
		self.page_width, self.page_height = page.get_size()
		self.drawarea.queue_draw()
		self.unref_object(document)
		self.unref_object(page)

	def next_page(self, button):
		self.goto_page(self.current_page + 1)

	def prev_page(self, button):
		self.goto_page(self.current_page - 1)

	def page_input(self, widget):
		pagenum = widget.get_text()
		try:
			page = int(pagenum) - 1
			if page <= 0 or page >= self.page_total:
				widget.set_text(str(self.current_page + 1))
			else:
				self.goto_page(page)
		except ValueError:
			widget.set_text(str(self.current_page + 1))

	def zoom_combo(self, widget):
		# scale values
		zoomlist = [0.50, 0.70, 0.85, 1.0, 1.25, 1.5, 2.0, 3.0, 4.0]

		zoom = widget.get_active()
		if zoom == -1:
			return

		self.best_fit = self.fit_width = False
		if zoom < 2:
			if zoom == 0: # Best Fit
				self.best_fit = True
			elif zoom == 1: # Fit Page Width
				self.fit_width = True
		else:
			self.scale = zoomlist[zoom-2]
		self.drawarea.queue_draw()

	def setlibgobject(self):
		# TODO: Write Python/Cython module to use this function directly.. 
		# TODO: Find function to detect system library paths, should exist. 
		paths = ['/usr/lib/', '/lib/', '/lib64/']
		for path in paths:
			status, langs = self.findgobject(path + 'libgobject*.so*')
			if status == 0:
				return langs[0]
		return None

	def findgobject(self, path):
		import commands, re
		status, langs = commands.getstatusoutput('ls ' + path)
		langs = sorted(list(set(re.sub(' \(.*?\)','', langs).split('\n'))))
		return status, langs

	def geturi(self, filename):
		if Environment.running_os == 'Windows':
			return 'file:///' + filename.replace('\\', '/')
		else:
			return 'file://' + filename


		
			


