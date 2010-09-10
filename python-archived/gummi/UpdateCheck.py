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

import gtk
import urllib

import Preferences

class UpdateCheck:
	"""Function that requests latest version info from the Gummi website"""

	def __init__(self):
		try:
			url = urllib.urlopen(Preferences.UPDATEURL)
			latest = str(url.readline())

			message = gtk.MessageDialog(None,
						gtk.DIALOG_MODAL, gtk.MESSAGE_INFO, gtk.BUTTONS_NONE,
						_("Currently installed:\n") + Preferences.VERSION +
						_("\n\nCurrently available:\n") + latest)
			message.add_button(gtk.STOCK_CLOSE, gtk.RESPONSE_CLOSE)
			message.set_title(_("Update Check"))
		except IOError: # catches no internet connection situations
			message = gtk.MessageDialog(None,
						gtk.DIALOG_MODAL, gtk.MESSAGE_INFO, gtk.BUTTONS_NONE,
						_("The server could not be contacted.\n\n") +
						_("This function requires an active\n") +
						_("internet connection."))
			message.add_button(gtk.STOCK_CLOSE, gtk.RESPONSE_CLOSE)
			message.set_title(_("Error"))
		resp = message.run()
		if resp == gtk.RESPONSE_CLOSE:
			message.destroy()












