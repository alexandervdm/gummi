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
import glib
import sys
import shutil
import pango
import subprocess
import traceback
import tempfile
import re
import gtk

import Environment
import Preferences

class Motion:

	def __init__(self, config, editor, preview, builder):
		self.config = config
		self.editorpane = editor
		self.previewpane = preview
		self.pdffile = None

		self.status = 1
		self.laststate = None

		# Timer settings for compilation
		self.timer = None
		self.signal_handlers = None
		self.IDLE_THRESHOLD = int(self.config.get_value\
							("compile", "idle_threshold"))
		self.update_event = 0

		try: 
			self.texcmd = self.config.get_value("compile", "typesetter")
		except: 
			self.texcmd = Preferences.TYPESETTER

		self.errormesg = re.compile(':[\d+]+:([^.]+)\.')
		self.errorline = re.compile(':([\d+]+):')

		self.preview_viewport = builder.get_object("preview_viewport")
		self.statuslight = builder.get_object("tool_statuslight")
		self.statusbar = builder.get_object("statusbar")
		self.statusbar_cid = self.statusbar.get_context_id("Gummi")
		self.errorfield = builder.get_object("errorfield")
		self.errorfield.modify_font(pango.FontDescription("monospace 8"))
		self.errorbuffer = self.errorfield.get_buffer()

		self.editorviewer = self.editorpane.editorviewer
		self.editorbuffer = self.editorpane.editorbuffer
		self.compilescheme = self.config.get_value("compile", "compile_scheme")

		#self.start_updatepreview()

	def start_updatepreview(self):
		self.compilescheme = self.config.get_value("compile", "compile_scheme")
		if self.compilescheme == 'on_idle':
			self.signal_handlers = [
				self.editorpane.editorviewer.connect('key-press-event',\
											self.on_key_pressed),
				self.editorpane.editorviewer.connect('key-release-event',\
											self.on_key_release)
			]

		compile_interval = self.config.get_value("compile", "compile_timer")
		if self.compilescheme == 'on_idle':
			self.start_timer()
		else:
			self.update_event = glib.timeout_add_seconds \
				(int(compile_interval), self.update_preview)

	def stop_updatepreview(self):
		if self.compilescheme == 'on_idle':
			for handler in self.signal_handlers:
				self.editorpane.editorviewer.disconnect(handler)
			self.stop_timer()
		else:
			if self.update_event:
				glib.source_remove(self.update_event)

	def update_envfiles(self, envfile):
		if self.pdffile is not None:
			self.cleanup_fd(100)			
		self.tempdir = envfile[0]
		self.filename = envfile[1]
		self.texpath = envfile[2]
		self.workfile = envfile[3]
		self.pdffile = envfile[4]

	def initial_preview(self, errormode=False):
		self.update_workfile()
		self.update_pdffile()
		pdffile_set = self.previewpane.set_pdffile(self.pdffile)
		if pdffile_set: # poppler object only created when pdffile valid
			self.previewpane.refresh_preview()
			if self.config.get_value("compile", "compile_status"):
				self.start_updatepreview()
			return True
		elif not errormode:
			self.setup_preview_error_mode()
			return False


	def setup_preview_error_mode(self):
		eventbox = gtk.EventBox()
		eventbox.set_events(gtk.gdk.BUTTON_PRESS_MASK)
		eventbox.connect('button-press-event', self.preview_error_mode)
		label = gtk.Label( \
			_("PDF-Preview could not initialize.\n\n" \
			"It appears your LaTeX document contains errors or\n" \
			"the program `%s' was not installed.\n"\
			"Additional information is available on the Error Output tab.\n" \
			"Please correct the listed errors and click this area\n" \
			"to reload the preview panel.") % self.texcmd)
		label.set_justify(gtk.JUSTIFY_CENTER)
		eventbox.add(label)
		self.preview_viewport.remove(self.previewpane.drawarea)
		self.preview_viewport.add(eventbox)
		self.preview_viewport.show_all()

	def preview_error_mode(self, widget, event):
		self.preview_viewport.remove(widget)
		self.preview_viewport.add(self.previewpane.drawarea)
		if not self.initial_preview(True):
			self.preview_viewport.remove(self.previewpane.drawarea)
			self.preview_viewport.add(widget)
	

	def update_workfile(self):
		try:
			# these two lines make the program hang in certain situations
			#self.editorpane.editorview.set_sensitive(False)
			buff = self.editorpane.editorviewer.get_buffer()
			start_iter, end_iter = buff.get_start_iter(), buff.get_end_iter()
			content = buff.get_text(start_iter, end_iter)
			#self.editorpane.editorview.set_sensitive(True)
			tmpmake = open(self.workfile, "w")
			tmpmake.write(content)
			tmpmake.close()
			self.editorviewer.grab_focus() #editorpane regrabs focus
		except:
			print traceback.print_exc()

	def update_auxfile(self):
		try:
			auxupdate = subprocess.Popen(self.texcmd + \
				' --draftmode \
				-interaction=nonstopmode \
				--output-directory="%s" "%s"' \
				% (Environment.tempdir, self.workfile), 
				shell=True, stdin=None, stdout=subprocess.PIPE, stderr=None)
			output = auxupdate.communicate()[0]
			auxupdate.wait()
		except: 
			print traceback.print_exc()

	def update_pdffile(self):
		try:
			pdfmaker = subprocess.Popen(self.texcmd + \
					' -interaction=nonstopmode \
					-file-line-error \
					-halt-on-error \
					--output-directory="%s" "%s"' \
					% (Environment.tempdir, self.workfile), 
					shell=True, cwd=self.texpath,
					close_fds=(Environment.running_os != 'Windows'), \
					stdin=None, stdout = subprocess.PIPE, stderr=None )
			self.output = pdfmaker.communicate()[0]
			pdfmaker.wait()
			self.errorbuffer.set_text(self.output)
			if pdfmaker.returncode:
				self.statuslight.set_stock_id("gtk-no")
			else:
				self.statuslight.set_stock_id("gtk-yes")
		except: print traceback.print_exc()
		return pdfmaker.returncode

	def update_errortags(self, errorstate):
		if errorstate is 1 and "Fatal error" in self.output:
			lineresult = self.errorline.findall(self.output)
			if lineresult == []: # end tag error
				lineresult.insert(0, self.editorbuffer.get_line_count())
			mesgresult = self.errormesg.findall(self.output)
			self.editorpane.apply_errortags(int(lineresult[0]))	
		elif errorstate is 0 and errorstate < self.laststate:
			self.editorpane.apply_errortags(None)
		else: pass
		self.laststate = errorstate

	def cleanup_fd(self, end=15):
		""" Dirty way to clean up the file descriptors from poppler
			objects that refuse to close. Also, xelatex appears to
			create many temp files that are deleted but not cleaned
			up properly, they are removed if xelatex is being used
			The call to /proc/ was only tested on linux systems. 
			FreeBSD won't work for lack of a fat linux-like procfs"""
		popplers = []
		if "bsd" in Environment.running_os.lower():
			try: os.close(6)
			except OSError: pass
		else:
			try:
				for i in range(6,end):
					fd = '/proc/self/fd/' + str(i)
					try:
						if os.readlink(fd) == self.pdffile:
							popplers.append(i)
						elif "(deleted)" in os.readlink(fd):
							os.close(i)
					except: break
				if len(popplers) > 1:
					os.close(min(popplers))
			except: pass



	def update_preview(self):
		try:
			if self.previewpane and self.editorpane.check_buffer_changed():
				self.editorpane.check_buffer_changed()
				self.update_workfile()
				retcode = self.update_pdffile()
				self.update_errortags(retcode)
				self.cleanup_fd(15)
				self.previewpane.refresh_preview()
		except:
			print traceback.print_exc()

		return self.compilescheme != 'on_idle'

	def start_timer(self):
		self.stop_timer()
		self.timer = glib.timeout_add(self.IDLE_THRESHOLD, self.update_preview)

	def stop_timer(self):
		if self.timer:
			 glib.source_remove(self.timer)

	def on_key_pressed(self, view, event):
		self.stop_timer()

	def on_key_release(self, view, event):
		self.start_timer()




