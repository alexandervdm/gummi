#!/usr/bin/env python

from sys import exit
from distutils.core import setup

try:
	from DistUtilsExtra.command import *
except ImportError:
	print "For packaging, you are required to have the\n" + \
		  "python-distutils-extra package installed..\n" + \
		  "https://launchpad.net/python-distutils-extra"
	exit()


_data_files = [
	('share/applications', ['gummi/misc/gummi.desktop']),
	('share/pixmaps', ['gummi/misc/gummi.png']),
	('share/man/man1', ['gummi/misc/gummi.1'])
	]

files = ["gui/gummi.glade",
	 "gui/prefs.glade",
	 "docs/CHANGES", 
	 "docs/LICENSE",
	 "docs/INSTALL",
	 "gui/icon.png",
	 "gui/article.png",
	 "gui/book.png",
	 "gui/letter.png",
	 "gui/report.png"
	]

setup(
	name = 'gummi',
	version = 'S.E.D',
	description = 'Simple LaTeX editor for GTK users',
	author = 'Alexander van der Mey',
	author_email = 'alexvandermey@gmail.com',
	url = 'http://gummi.midnightcoding.org',
	license = 'MIT',
    	packages = ['gummi'],
	package_data = {'gummi' : files },
	scripts = ['gummi/misc/gummi'],
	data_files = _data_files,
        cmdclass = { "build": build_extra.build_extra,
                     "build_i18n": build_i18n.build_i18n }
)

