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
		gui-main.c gui-main.h \
		importer.c importer.h \
		gui-import.c gui-import.h \
		iofunctions.c iofunctions.h \
		motion.c motion.h \
		gui-prefs.c gui-prefs.h \
		preview.c preview.h \
		gui-search.c gui-search.h \
		template.c template.h \
		updatecheck.c updatecheck.h \
		utils.c utils.h \
		main.c
