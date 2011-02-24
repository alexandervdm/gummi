man_MANS = $(PACKAGE).1

desktopdir = $(datadir)/applications
desktop_DATA = $(PACKAGE).desktop

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = $(PACKAGE).png

imagesdir = $(datadir)/$(PACKAGE)
images_DATA = $(PACKAGE).png icon.png

gladedir = $(datadir)/$(PACKAGE)
glade_DATA = gummi.glade prefs.glade

snippetsdir = $(datadir)/$(PACKAGE)
snippets_DATA = snippets

EXTRA_DIST = $(man_MANS) \
	     $(desktop_DATA).in \
	     $(pixmaps_DATA) \
	     $(images_DATA) \
	     $(glade_DATA) \
	     $(snippets_DATA)
