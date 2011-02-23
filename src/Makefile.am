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
		gui/gui-editortabs.c gui/gui-editortabs.h \
		gui/gui-import.c gui/gui-import.h \
		gui/gui-main.c gui/gui-main.h \
		gui/gui-prefs.c gui/gui-prefs.h \
		gui/gui-preview.c gui/gui-preview.h \
		gui/gui-search.c gui/gui-search.h \
		importer.c importer.h \
		iofunctions.c iofunctions.h \
		latex.c latex.h \
		motion.c motion.h \
		snippets.c snippets.h \
		template.c template.h \
		updatecheck.c updatecheck.h \
		utils.c utils.h \
		main.c
