bin_PROGRAMS = gummi
AM_CFLAGS = $(GUI_CFLAGS) \
	    -export-dynamic -Wall -O2 \
	    -DDATADIR=\"$(datadir)/$(PACKAGE)\" \
	    -DLOCALEDIR=\"$(datadir)/locale\"

gummi_LDADD = $(GUI_LIBS) \
	      $(LIBINTL)

gummi_SOURCES = biblio.c  biblio.h \
		configfile.c configfile.h \
		editor.c editor.h \
		environment.c environment.h \
		fileinfo.c fileinfo.h \
		gummi-gui.c gummi-gui.h \
		importer.c importer.h \
		import-gui.c import-gui.h \
		iofunctions.c iofunctions.h \
		motion.c motion.h \
		prefs-gui.c prefs-gui.h \
		preview.c preview.h \
		search-gui.c search-gui.h \
		template.c template.h \
		updatecheck.c updatecheck.h \
		utils.c utils.h \
		gummi.c
