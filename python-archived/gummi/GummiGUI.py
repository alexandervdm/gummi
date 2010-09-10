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
import sys
import glib
import gtk
import pango
import traceback

import Importer
import UpdateCheck
import Template
import Preferences
import Environment

class MainGUI:

	def __init__(self, config, builder, iofunc, biblio, motion, editor, preview):
		self.config = config
		self.editorpane = editor
		self.previewpane = preview
		self.iofunc = iofunc
		self.biblio = biblio
		self.motion = motion

		self.builder = builder
		self.exitinterrupt = False
		
		self.mainwindow = self.builder.get_object("mainwindow")
		self.toolbar = self.builder.get_object("toolbar")
		self.mainnotebook = self.builder.get_object("main_notebook")
		self.editorscroll = self.builder.get_object("editor_scroll")
		self.drawarea = self.builder.get_object("preview_drawarea")
		self.preview_toolbar = self.builder.get_object("preview_toolbar")
		self.bibprogressbar = self.builder.get_object("bibprogressbar")
		self.bibprogressmon = self.builder.get_object("bibprogressmon")
		self.bibprogressval = 0
		self.list_biblios = self.builder.get_object("list_biblios")
		self.rightpane = self.builder.get_object("vbox2")
		self.pausebutton = self.builder.get_object("tool_previewoff")

		if not self.editorpane.gtkspell_available():
			self.builder.get_object("menu_spelling").set_sensitive(False)

		if self.config.get_value("view", "toolbar"):
			menu_toolbar = self.builder.get_object("menu_toolbar")
			menu_toolbar.set_active(True)
			self.toolbar.show()

		if self.config.get_value("view", "statusbar"):
			menu_statusbar = self.builder.get_object("menu_statusbar")
			menu_statusbar.set_active(True)
			self.iofunc.statusbar.show()

		if self.config.get_value("view", "rightpane"):
			menu_rightpane = self.builder.get_object("menu_rightpane")
			menu_rightpane.set_active(True)
			self.rightpane.show()
		else:
			tool_rightpane = self.builder.get_object("tool_hide_rightpane")
			tool_rightpane.set_active(True)

		if not self.config.get_value('compile', 'compile_status'):
			self.pausebutton.set_active(True)

		if self.config.get_value("editor", "spelling"):
			self.builder.get_object("menu_spelling").set_active(True)
		
		self.hpaned = self.builder.get_object("hpaned")
		mainwidth = self.mainwindow.get_size()[0]
		self.hpaned.set_position(mainwidth/2)

		self.editorscroll.add(self.editorpane.editorviewer)
		self.importgui = ImportGUI(self.builder, self.editorpane)
		self.recentgui = RecentGUI(self.builder, self.config, self)
		self.searchgui = SearchGUI(self.builder, self.editorpane)
		self.builder.connect_signals(self) #split signals?
		self.mainwindow.show_all()

	def main(self):
		self.mainwindow.show_all()
		gtk.main()

	def set_window_title(self, filename):
		self.mainwindow.set_title \
					(os.path.basename(filename) + " - Gummi")

	def on_menu_new_activate(self, menuitem, data=None):
		if self.check_for_save(): self.on_menu_save_activate(None, None)
		self.editorpane.fill_buffer \
			(self.config.get_value("default_text", "welcome"))
		self.editorpane.editorbuffer.set_modified(False)
		self.filename = None
		self.iofunc.make_environment(self.filename)

	def on_menu_template_activate(self, menuitem, data=None):
		self.template_doc = Template.Template(self.builder)

	def on_menu_open_activate(self, menuitem, data=None):
		if os.getcwd() == Environment.tempdir:
			os.chdir(Environment.HOME)
		if self.check_for_save(): self.on_menu_save_activate(None, None)
		filename = self.get_open_filename()
		if filename: self.load_document(filename)

	def on_menu_save_activate(self, menuitem, data=None):
		if os.getcwd() == Environment.tempdir:
			os.chdir(Environment.HOME)
		if self.iofunc.filename is None:
			filename = self.get_save_filename()
			if filename: self.save_document(filename)
		else: self.save_document(None)

	def on_menu_saveas_activate(self, menuitem, data=None):
		if os.getcwd() == Environment.tempdir:
			os.chdir(Environment.HOME)
		self.filename = self.get_save_filename()
		if self.filename: self.save_document(self.filename)

	def on_menu_exportpdf_activate(self, menuitem, data=None):
		# We ask user for filename everytime because this is an 'export' util.
		pdfname = self.get_save_filename(of_pdf=True)
		if pdfname: self.iofunc.export_pdffile(pdfname)

	def on_menu_recent_activate(self, widget, data=None):
		self.recentgui.activate_recentfile(widget)

	def on_menu_undo_activate(self, menuitem, data=None):
		self.editorpane.undo_change()

	def on_menu_redo_activate(self, menuitem, data=None):
		self.editorpane.redo_change()

	def on_menu_cut_activate(self, menuitem, data=None):
		buff = self.editorpane.editorviewer.get_buffer()
		buff.cut_clipboard (gtk.clipboard_get(), True)
		self.editorpane.set_buffer_changed()

	def on_menu_copy_activate(self, menuitem, data=None):
		buff = self.editorpane.editorviewer.get_buffer()
		buff.copy_clipboard (gtk.clipboard_get())

	def on_menu_paste_activate(self, menuitem, data=None):
		buff = self.editorpane.editorviewer.get_buffer()
		buff.paste_clipboard (gtk.clipboard_get(), None, True)
		self.editorpane.set_buffer_changed()

	def on_menu_delete_activate(self, menuitem, data=None):
		buff = self.editorpane.editorviewer.get_buffer()
		buff.delete_selection (False, True)

	def on_menu_selectall_activate(self, menuitem, data=None):
		buff = self.editorpane.editorviewer.get_buffer()
		buff.select_range(buff.get_start_iter(),buff.get_end_iter())

	def on_menu_find_activate(self, menuitem, data=None):
		self.searchgui.show_searchwindow()

	def on_menu_findnext_activate(self, menuitem, data=None):
		self.editorpane.jumpto_searchresult(1)

	def on_menu_findprev_activate(self, menuitem, data=None):
		self.editorpane.jumpto_searchresult(-1)

	def on_menu_fullscreen_toggled(self, menuitem, data=None):
		if menuitem.get_active():
			self.mainwindow.fullscreen()
		else:
			self.mainwindow.unfullscreen()

	def on_menu_spelling_toggled(self, menuitem, data=None):
		if menuitem.get_active():
			self.editorpane.activate_spellchecking(1)
			self.config.set_value("editor", "spelling", True)
		else:
			self.editorpane.activate_spellchecking(0)
			self.config.set_value("editor", "spelling", False)

	def on_menu_toolbar_toggled(self, menuitem, data=None):
		if menuitem.get_active():
			self.toolbar.show()
			self.config.set_value("view", "toolbar", True)
		else:
			self.toolbar.hide()
			self.config.set_value("view", "toolbar", False)

	def on_menu_statusbar_toggled(self, menuitem, data=None):
		if menuitem.get_active():
			self.iofunc.statusbar.show()
			self.config.set_value("view", "statusbar", True)
		else:
			self.iofunc.statusbar.hide()
			self.config.set_value("view", "statusbar", False)

	def on_button_searchwindow_close_clicked(self, button, data=None):
		self.searchgui.close_searchwindow()
		return True

	def on_button_searchwindow_find_clicked(self, button, data=None):
		self.searchgui.start_search()

	def on_button_searchwindow_replace_next_clicked(self, button, data=None):
		self.searchgui.start_replace_next()

	def on_button_searchwindow_replace_all_clicked(self, button, data=None):
		self.searchgui.start_replace_all()

	def on_import_tabs_switch_page(self, notebook, page, page_num):
		self.importgui.show_importpane(notebook, page_num)

	def on_button_template_ok_clicked(self, button, data=None):
		template = self.template_doc.get_template()
		if template is not None:
			self.editorpane.fill_buffer(template)
			self.filename = None
			self.iofunc.make_environment(self.filename)
			self.template_doc.templatewindow.hide()
		else: pass

	def on_button_template_cancel_clicked(self, button, data=None):
		self.template_doc.templatewindow.hide()
		return True

	def on_menu_bibupdate_activate(self, menuitem, data=None):
		self.biblio.compile_bibliography()

	def on_bibprogressbar_update(self):
		self.bibprogressmon.set_value(self.bibprogressval)
		self.bibprogressval = self.bibprogressval + 1
		if self.bibprogressval > 60:
			return False
		else:
			return True
		
	def on_bibrefresh_clicked(self, button, data=None):
		self.bibprogressval = 0
		glib.timeout_add(2, self.on_bibprogressbar_update)
		self.list_biblios.clear()
		bibfilenm = self.builder.get_object("bibfilenm")
		bibrefnr = self.builder.get_object("bibrefnr")
		if self.biblio.detect_bibliography():
			filenm, number = self.biblio.setup_bibliography()
			number = self.biblio.parse_entries(self.list_biblios)
			bibfilenm.set_text(filenm)
			bibrefnr.set_text(str(number))
			self.bibprogressbar.set_text(filenm + " loaded")
		else:
			self.bibprogressbar.set_text(_("no bibliography file detected"))
			bibfilenm.set_text(_("None"))
			bibrefnr.set_text("N/A")

	def on_bibcompile_clicked(self, button, data=None):
		self.bibprogressval = 0
		glib.timeout_add(10, self.on_bibprogressbar_update)
		if self.biblio.compile_bibliography(self.bibprogressbar):
			self.iofunc.set_status(_("Compiling bibliography file.."))
			self.bibprogressbar.set_text(_("bibliography compiled without errors"))
		else:
			self.iofunc.set_status(_("Error compiling bibliography file or none detected.."))
			self.bibprogressbar.set_text(_("error compiling bibliography file"))



	def on_menu_bibload_activate(self, menuitem, data=None):
		bibfile = None
		chooser = gtk.FileChooserDialog(_("Open File..."), self.mainwindow,
								gtk.FILE_CHOOSER_ACTION_OPEN,
								(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
								gtk.STOCK_OPEN, gtk.RESPONSE_OK))
		bibfilter = gtk.FileFilter()
		bibfilter.set_name('Bibtex files')
		bibfilter.add_pattern("*.bib")
		chooser.add_filter(bibfilter)
		response = chooser.run()
		if response == gtk.RESPONSE_OK: 
			bibfile = chooser.get_filename()
			if self.biblio.check_valid_file(bibfile):
				filenm, number = self.biblio.setup_bibliography()
				bibfilenm = self.builder.get_object("bibfilenm")
				bibfilenm.set_text(filenm)
		chooser.destroy()

	def on_bibcolumn_clicked(self, item, data=None):
		sortid = item.get_sort_column_id()
		item.set_sort_column_id(sortid)

	def on_bibreference_clicked(self, item, event, data=None):
		selection = item.get_selection()
		myiter = selection.get_selected()[1]
		value = self.list_biblios.get_value(myiter, 0)
		self.editorpane.editorbuffer.insert_at_cursor("\cite{" + value + "}")
		self.editorpane.set_buffer_changed()
	
	def on_menu_docstat_activate(self, item, data=None):
		pass

	def on_menu_preferences_activate(self, menuitem, data=None):
		PrefsGUI(self.config, self.editorpane, \
				 self.mainwindow, self.iofunc, self.motion, self.pausebutton)

	def on_menu_update_activate(self, menuitem, data=None):
		update = UpdateCheck.UpdateCheck()

	def on_menu_about_activate(self, menuitem, data=None):
		authors = [
		"Alexander van der Mey",
		"<alexvandermey@gmail.com>",
		"Wei-Ning Huang",
		"<aitjcize@gmail.com>",
		"",
		"Contributors:",
		"Thomas van der Burgt",
		"Cameron Grout"]
		translators = "(Taiwanese) - Wei-Ning Huang"	
		artwork = ["Template icon set from:\nhttp://www.fatcow.com/"
                           "free-icons/",
                           "Windows version Icon set from Elemetary "
                           "Project:\nhttp://www.elementary-project.com/"]
		about_dialog = gtk.AboutDialog()
		about_dialog.set_transient_for(self.mainwindow)
		about_dialog.set_destroy_with_parent(True)
		about_dialog.set_name("Gummi")
		about_dialog.set_translator_credits(translators)
		about_dialog.set_license(Preferences.LICENSE)
		about_dialog.set_version(Preferences.VERSION)
		about_dialog.set_copyright("Copyright \xc2\xa9 2009 Alexander van der Mey")
		about_dialog.set_website("http://gummi.midnightcoding.org")
		about_dialog.set_comments("Simple LaTeX Editor for GTK+ users")
		about_dialog.set_authors(authors)
		about_dialog.set_artists(artwork)
		about_dialog.set_logo_icon_name	 (gtk.STOCK_EDIT)
		# callbacks for destroying the dialog
		def close(dialog, response, editor):
			editor.about_dialog = None
			dialog.destroy()
		def delete_event(dialog, event, editor):
			editor.about_dialog = None
			return True
		about_dialog.connect("response", close, self)
		about_dialog.connect("delete-event", delete_event, self)
		self.about_dialog = about_dialog
		about_dialog.show()

	def on_tool_textstyle_activate(self, button, data=None):
		self.editorpane.set_selection_textstyle(button)
	
	def on_tool_previewoff_toggled(self, button, data=None):
		value = button.get_active()
		self.config.set_value('compile', 'compile_status', not value)
		if value:
			self.motion.stop_updatepreview()
		else:
			self.motion.start_updatepreview()
	
	def on_tool_hide_rightpane_toggled(self, button, data=None):
		# This callback served both "tool_hide_rightpane" and "menu_rightpane",
		# but the behavior is different.
		#                   active   not_active
		# checkbutton        show      hide
		# not_checkbutton    hide      show

		if button.get_active() ^ (type(button) == gtk.CheckMenuItem):
			self.rightpane.hide()
			self.pausebutton.set_active(True)
			self.pausebutton.set_sensitive(False)
		else:
			self.rightpane.show()
			self.pausebutton.set_active(False)
			self.pausebutton.set_sensitive(True)

		# synchronize status of "tool_hide_rightpane" and "menu_rightpane"
		if type(button) == gtk.CheckMenuItem:
			self.config.set_value("view", "rightpane", button.get_active())
		else:
			menu_button = self.builder.get_object("menu_rightpane")
			menu_button.set_active(not button.get_active())

	def on_button_import_apply_clicked(self, button, data=None):
		self.importgui.insert_object(button)

	def on_image_file_activate(self, button, event, data=None):
		self.importgui.choose_imagefile()

	def on_button_pageback_clicked(self, button, data=None):
		self.previewpane.jump_to_prevpage()

	def on_button_pageforward_clicked(self, button, data=None):
		self.previewpane.jump_to_nextpage()

	def on_button_zoomin_clicked(self, button, data=None):
		self.previewpane.zoom_in_pane()

	def on_button_zoomout_clicked(self, button, data=None):
		self.previewpane.zoom_out_pane()

	def on_button_bestfit_toggled(self, button, data=None):
		if button.get_active() == False:
			self.previewpane.set_bestfitmode(False)
		else:
			self.previewpane.set_bestfitmode(True)

	def on_button_bibadd_clicked(self, button, data=None):
		self.biblio.add_bibliography()

	def on_button_bibdel_clicked(self, button, data=None):
		self.biblio.del_bibliography()

	def on_button_bibapply_clicked(self, button, data=None):
		self.biblio.setup_bibliography()
		self.mainnotebook.set_current_page(0)

	def set_file_filters(self, dialog, to_pdf=False):
		if to_pdf:
			pdffilter = gtk.FileFilter()
			pdffilter.set_name('application/pdf')
			pdffilter.add_pattern('*.pdf')
			dialog.add_filter(pdffilter)
			dialog.set_filter(pdffilter)
		else:
			plainfilter = gtk.FileFilter()
			plainfilter.set_name('Text files')
			plainfilter.add_mime_type("text/plain")
			dialog.add_filter(plainfilter)

			latexfilter = gtk.FileFilter()
			latexfilter.set_name('LaTeX files')
			latexfilter.add_pattern('*.tex')
			dialog.add_filter(latexfilter)
			dialog.set_filter(plainfilter)

	def get_open_filename(self):
		filename = None
		chooser = gtk.FileChooserDialog(_("Open File..."), self.mainwindow,
										gtk.FILE_CHOOSER_ACTION_OPEN,
										(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
										gtk.STOCK_OPEN, gtk.RESPONSE_OK))
		self.set_file_filters(chooser)

		response = chooser.run()
		if response == gtk.RESPONSE_OK: filename = chooser.get_filename()
		chooser.destroy()
		return filename

	def get_save_filename(self, of_pdf=False):
		# reuse this function to get save filename of exported pdf
		filename = None
		chooser = gtk.FileChooserDialog(_("Save File..."), self.mainwindow,
										gtk.FILE_CHOOSER_ACTION_SAVE,
										(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
										gtk.STOCK_SAVE, gtk.RESPONSE_OK))
		self.set_file_filters(chooser, of_pdf)
		response = chooser.run()
		if response == gtk.RESPONSE_CANCEL:
			self.exitinterrupt = True
		if response == gtk.RESPONSE_OK:
			filename = chooser.get_filename()
			if of_pdf:
				# export_pdffile will add '.pdf'
				if ".pdf" == filename[-4:]:
					filename = filename[:-4]
			else:
				if not ".tex" in filename[-4:]:
					filename = filename + ".tex"
				self.iofunc.make_environment(filename)
		chooser.destroy()
		return filename

	def check_for_save(self):
		ret = False
		if self.editorpane.editorbuffer.get_modified():
			# we need to prompt for save
			message = _("Do you want to save the changes you have made?")
			dialog = gtk.MessageDialog(self.mainwindow,
							gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
							gtk.MESSAGE_QUESTION, gtk.BUTTONS_YES_NO,
							message)
			dialog.set_title(_("Save?"))
			if dialog.run() == gtk.RESPONSE_NO: ret = False
			else: ret = True
			dialog.destroy()
		return ret


	def load_document(self, filename):
		while gtk.events_pending(): gtk.main_iteration()
		try:
			self.iofunc.load_file(filename)
			self.filename = filename
			self.recentgui.add_recentfile(self.filename)
			self.set_window_title(self.filename)
			os.chdir(os.path.dirname(filename))
		except:
			print traceback.print_exc()


	def save_document(self, filename):
		try:
			self.iofunc.save_file(filename)
			if filename: self.filename = filename
			self.iofunc.set_status(_("Saving file ") + self.filename)
			self.iofunc.export_pdffile()
			self.set_window_title(self.filename)
		except:
			print traceback.print_exc()


	def gtk_main_quit(self, menuitem, data=None):
		if self.check_for_save(): self.on_menu_save_activate(None, None)
		if self.exitinterrupt is False:
			print "   ___ "
			print "  {o,o}	  Thanks for using Gummi!"
			print "  |)__)	  I welcome your feedback at:"
			print '  -"-"-	  http://gummi.midnightcoding.org\n'
			sys.exit(0)
		else: self.exitinterrupt = False; return True



class PrefsGUI:

	def __init__(self, config, editorpane, mainwindow, iofunc, motion, pausebutton):
		self.config = config
		self.editorpane = editorpane
		self.iofunc = iofunc
		self.motion = motion
		self.pausebutton = pausebutton
		builder = gtk.Builder()
		builder.set_translation_domain("gummi")
		builder.add_from_file(Environment.prefs_glade)
		self.builder = builder

		self.prefwindow = builder.get_object("prefwindow")
		self.notebook = builder.get_object("notebook1")
		self.prefwindow.set_transient_for(mainwindow)
		self.textwrap_button = builder.get_object("textwrapping")
		self.wordwrap_button = builder.get_object("wordwrapping")
		self.autosave_timer = builder.get_object("autosave_timer")
		self.default_text = builder.get_object("default_text")
		self.default_text.modify_font(pango.FontDescription("monospace 10"))
		self.default_buffer = self.default_text.get_buffer()
		self.typesetter = builder.get_object("combo_typesetter")
		self.editor_font = builder.get_object("editor_font")
		self.compilescheme = builder.get_object("combo_compilescheme")
		self.compile_timer = builder.get_object("compile_timer")

		self.view_box = builder.get_object("view_box")
		self.set_checkbox_status(self.view_box, 'view')
		self.editor_box = builder.get_object("editor_box")
		self.set_checkbox_status(self.editor_box, 'editor')
		self.compile_box = builder.get_object("compile_box")
		self.set_checkbox_status(self.compile_box, 'compile')

		self.autosave_timer.set_value \
			(int(self.config.get_value("editor", "autosave_timer"))/60)
		self.compile_timer.set_value \
			(int(self.config.get_value("compile", "compile_timer")))
		self.editor_font.set_font_name(self.config.get_value("editor", "font"))
		self.default_buffer.set_text \
			(self.config.get_value("default_text", "welcome"))
		if self.config.get_value("compile", "typesetter") == "xelatex":
			self.typesetter.set_active(1)
		if self.config.get_value("compile","compile_scheme") =='real_time':
			self.compilescheme.set_active(1)

		self.list_available_spell_languages()

		builder.connect_signals(self)
		self.prefwindow.show_all()

	def set_checkbox_status(self, box, page):
		""" Sets the status for all checkboxes in the Preferences screen."""
		listmy = box.get_children()
		for item in listmy:
			if type(item) == gtk.CheckButton:
				result = self.config.get_value(page, gtk.Buildable.get_name(item))
				item.set_active(result)

	def toggle_linenumbers(self, widget, data=None):
		value = widget.get_active()
		self.config.set_value('view', 'line_numbers', value)
		self.editorpane.editorviewer.set_show_line_numbers(value)

	def toggle_highlighting(self, widget, data=None):
		value = widget.get_active()
		self.config.set_value('view', 'highlighting', value)
		self.editorpane.editorviewer.set_highlight_current_line(value)

	def toggle_textwrapping(self, widget, data=None):
		value = widget.get_active()
		self.config.set_value('view', 'textwrapping', value)
		if widget.get_active():
			self.editorpane.editorviewer.set_wrap_mode(gtk.WRAP_CHAR)
			self.wordwrap_button.set_sensitive(True)
		else:
			self.editorpane.editorviewer.set_wrap_mode(gtk.WRAP_NONE)
			self.config.set_value("view", "wordwrapping", False)
			self.wordwrap_button.set_active(False)
			self.wordwrap_button.set_sensitive(False)

	def toggle_wordwrapping(self, widget, data=None):
		value = widget.get_active()
		self.config.set_value('view', 'wordwrapping', value)
		if widget.get_active():
			self.editorpane.editorviewer.set_wrap_mode(gtk.WRAP_WORD)
		else:
			self.editorpane.editorviewer.set_wrap_mode(gtk.WRAP_CHAR)

	def toggle_autosaving(self, widget, data=None):
		value = widget.get_active()
		self.config.set_value('editor', 'autosaving', value)
		if widget.get_active():
			self.autosave_timer.set_sensitive(True)
			time = int(self.config.get_value("editor", "autosave_timer"))
			self.autosave_timer.set_value(time/60)
			self.iofunc.start_autosave(time)
		else:
			self.autosave_timer.set_sensitive(False)
			self.iofunc.stop_autosave()

	def toggle_compilestatus(self, widget, data=None):
		value = widget.get_active()
		self.config.set_value('compile', 'compile_status', value)
		if widget.get_active():
			self.pausebutton.set_active(False)
		else:
			self.pausebutton.set_active(True)

	def on_autosave_value_changed(self, event):
		newvalue = int(event.get_value()) * 60
		self.config.set_value('editor', 'autosave_timer', newvalue)
		self.iofunc.reset_autosave()

	def on_compile_value_changed(self, event):
		newvalue = int(event.get_value())
		self.config.set_value('compile', 'compile_timer', newvalue)
		if self.config.get_value('compile', 'compile_status'):
			self.motion.stop_updatepreview()
			self.motion.start_updatepreview()

	def on_editor_font_set(self, widget):
		selected = widget.get_font_name()
		self.editorpane.editorviewer.modify_font(pango.FontDescription(selected))
		self.config.set_value("editor", "font", selected)

	def on_combo_typesetter_changed(self, widget, data=None):
		model = widget.get_model()
		newvalue = model[widget.get_active()][0]
		self.config.set_value('compile', 'typesetter', newvalue)
		self.builder.get_object("changeimg").show()
		self.builder.get_object("changelabel").show()

	def on_combo_language_changed(self, widget, data=None):
		model = widget.get_model()
		newvalue = model[widget.get_active()][0]
		self.config.set_value('editor', 'spell_language', newvalue)
		self.editorpane.activate_spellchecking(0)
		self.editorpane.activate_spellchecking(1)
	
	def on_combo_compilescheme_changed(self, widget, data=None):
		value = ['on_idle', 'real_time'][widget.get_active()]
		if self.config.get_value('compile', 'compile_status'):
			self.motion.stop_updatepreview()
			self.config.set_value('compile', 'compile_scheme', value)
			self.motion.start_updatepreview()
		else:
			self.config.set_value('compile', 'compile_scheme', value)

	def list_available_spell_languages(self):
		import re, commands
		list_languages = self.builder.get_object("list_languages")
		combo_languages = self.builder.get_object("combo_languages")
		language_set = self.config.get_value("editor", "spell_language")
		status, langs = commands.getstatusoutput('enchant-lsmod -list-dicts')
		combo_languages.set_active(0)
		if status == 0:		
			langs = sorted(list(set(re.sub(' \(.*?\)','', langs).split('\n'))))
			counter = 0			
			for item in langs:
				list_languages.append([item])
				counter += 1
				if language_set == item:
					combo_languages.set_active(counter)
					

	def on_prefs_close_clicked(self, widget, data=None):
		if self.notebook.get_current_page() == 2:
			start_iter = self.default_buffer.get_start_iter()
			end_iter = self.default_buffer.get_end_iter()
			newvalue = self.default_buffer.get_text(start_iter, end_iter)
			self.config.set_value("default_text", "welcome", newvalue)
		self.prefwindow.destroy()

	def on_prefs_reset_clicked(self, widget, data=None):
		if self.notebook.get_current_page() == 0:
			self.config.reset_section("view")
			self.set_checkbox_status(self.view_box, "view")
		elif self.notebook.get_current_page() == 1:
			self.config.reset_section("editor")
			self.set_checkbox_status(self.editor_box, "editor")
			deffont = self.config.get_value("editor", "font")
			self.editor_font.set_font_name(deffont)
			self.editorpane.editorviewer.modify_font(pango.FontDescription(deffont))
		elif self.notebook.get_current_page() == 2:
			self.config.reset_section("default_text")
			self.default_buffer.set_text \
				(self.config.get_value("default_text", "welcome"))
		elif self.notebook.get_current_page() == 3:
			self.config.reset_section("compile")
			self.set_checkbox_status(self.compile_box, 'compile')
			self.compile_timer.set_value \
				(int(self.config.get_value("compile", "compile_timer")))
			self.typesetter.set_active(0)


class SearchGUI:

	def __init__(self, builder, editorpane):
		self.builder = builder
		self.editorpane = editorpane
		self.setup_searchwindow()
		self.searchentry.connect('changed', self.on_text_changed_event)

	def setup_searchwindow(self):
		self.searchwindow = self.builder.get_object("searchwindow")
		self.searchentry = self.builder.get_object("searchentry")
		self.replaceentry = self.builder.get_object("replaceentry")
		self.backwards = self.builder.get_object("toggle_backwards")
		self.matchcase = self.builder.get_object("toggle_matchcase")
		self.wholeword = self.builder.get_object("toggle_wholeword")

	def show_searchwindow(self):
		self.searchentry.set_text("")
		self.replaceentry.set_text("")
		self.searchentry.grab_focus()
		self.searchwindow.show()

	def close_searchwindow(self):
		self.searchwindow.hide()
		return True

	def start_search(self):
		term = self.searchentry.get_text()
		backwards = self.backwards.get_active()
		matchcase = self.matchcase.get_active()
		wholeword = self.wholeword.get_active()
		self.editorpane.start_search(term, backwards, wholeword, matchcase)

	def start_replace_next(self):
		term = self.searchentry.get_text()
		rpterm = self.replaceentry.get_text()
		backwards = self.backwards.get_active()
		matchcase = self.matchcase.get_active()
		wholeword = self.wholeword.get_active()
		self.editorpane.start_replace_next(term, rpterm, backwards, wholeword,
									  matchcase)

	def start_replace_all(self):
		term = self.searchentry.get_text()
		rpterm = self.replaceentry.get_text()
		backwards = self.backwards.get_active()
		matchcase = self.matchcase.get_active()
		wholeword = self.wholeword.get_active()
		self.editorpane.start_replace_all(term, rpterm, backwards, wholeword,
									matchcase)
	
	def on_text_changed_event(self, event):
		self.editorpane.replace_activated = False


class ImportGUI:

	def __init__(self, builder, editorpane):
		self.builder = builder
		self.editorpane = editorpane

		self.mainwindow = builder.get_object("mainwindow")

		self.imagefile = None

		self.setup_importpanes()

	def setup_importpanes(self):
		self.box_image = self.builder.get_object("box_image")
		self.box_table = self.builder.get_object("box_table")
		self.box_matrix = self.builder.get_object("box_matrix")
		self.image_pane = self.builder.get_object("image_pane")
		self.table_pane = self.builder.get_object("table_pane")
		self.matrix_pane = self.builder.get_object("matrix_pane")
		self.importer = Importer.Importer(self.editorpane, self.builder)

	def show_importpane(self, notebook, page_num):
		newactive = gtk.Buildable.get_name(notebook.get_nth_page(page_num))
		self.box_image.foreach(lambda x:self.box_image.remove(x))
		self.box_table.foreach(lambda x:self.box_table.remove(x))
		self.box_matrix.foreach(lambda x:self.box_matrix.remove(x))
		if newactive == "box_image":
			self.box_image.add(self.image_pane)
		elif newactive == "box_table":
			self.box_table.add(self.table_pane)
		elif newactive == "box_matrix":
			self.box_matrix.add(self.matrix_pane)

	def insert_object(self, widget):
		caller = gtk.Buildable.get_name(widget)
		if caller == "table_apply":
			self.importer.insert_table()
		elif caller == "image_apply":
			self.importer.insert_image(self.imagefile)
		elif caller == "matrix_apply":
			self.importer.insert_matrix()

	def choose_imagefile(self):
		chooser = gtk.FileChooserDialog("Open File...", self.mainwindow,
								gtk.FILE_CHOOSER_ACTION_OPEN,
								(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
								gtk.STOCK_OPEN, gtk.RESPONSE_OK))
		imagefilter = gtk.FileFilter()
		imagefilter.set_name('Image files')
		imagefilter.add_mime_type("image/*")
		chooser.add_filter(imagefilter)
		response = chooser.run()
		if response == gtk.RESPONSE_OK: 
			imagefile = chooser.get_filename()
			self.imagefile = imagefile
			self.importer.activate_imagegui(imagefile, 1)
		chooser.destroy()

class RecentGUI:

	def __init__(self, builder, config, parent):	
		self.builder = builder
		self.config = config
		self.parent = parent
		self.iofunc = self.parent.iofunc

		self.recentlist = []
		self.setup_recentfiles()

		
	def setup_recentfiles(self):
		self.recent1 = self.builder.get_object("menu_recent1")
		self.recent2 = self.builder.get_object("menu_recent2")
		self.recent3 = self.builder.get_object("menu_recent3")
		recent1 = self.config.get_value("recent_files", "recent1")
		recent2 = self.config.get_value("recent_files", "recent2")
		recent3 = self.config.get_value("recent_files", "recent3")
		self.recentlist.append(recent1)	
		self.recentlist.append(recent2)	
		self.recentlist.append(recent3)
		self.display_recentfile(0, self.recent1)
		self.display_recentfile(1, self.recent2)
		self.display_recentfile(2, self.recent3)

	def display_recentfile(self, i, widget):
		try:
			if not self.recentlist[i] == "":
				entry = os.path.basename(self.recentlist[i])
				widget.get_children()[0].set_label(str(i+1) + ". " + entry)
				widget.show()
		except IndexError: widget.hide()

	def activate_recentfile(self, widget):
		indexstr = gtk.Buildable.get_name(widget)[11:] # dirty hack
		indexnr = int(indexstr)-1 # to get index number
		if os.path.exists(self.recentlist[indexnr]):
			self.load_recentfile(self.recentlist[indexnr])
		else:
			self.remove_recentfile(indexnr)

	def add_recentfile(self, filename):
		if filename not in self.recentlist:
			self.recentlist.insert(0, filename)
			if len(self.recentlist) > 3:
				del self.recentlist[3]
			for index,value in enumerate(self.recentlist):
				position = str(index+1)
				self.config.set_value \
					("recent_files", "recent" + position, value)
		self.display_recentfile(0, self.recent1)
		self.display_recentfile(1, self.recent2)
		self.display_recentfile(2, self.recent3)


	def remove_recentfile(self, position):
		self.iofunc.set_status(_("Error loading recent file: ") + str(self.recentlist[position]))
		self.recentlist.pop(position)
		for index,value in enumerate(self.recentlist):
			position = str(index+1)
			self.config.set_value("recent_files", "recent" + position, value)
		self.display_recentfile(0, self.recent1)
		self.display_recentfile(1, self.recent2)
		self.display_recentfile(2, self.recent3)
		

	def load_recentfile(self, filename):
		self.parent.check_for_save()
		self.parent.load_document(filename)

