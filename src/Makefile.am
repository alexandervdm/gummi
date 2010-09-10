bin_PROGRAMS = gummi-beta
AM_CFLAGS = $(GUI_CFLAGS) \
	    -export-dynamic -Wall -O2 \
	    -DDATADIR=\"$(datadir)/$(PACKAGE)\" \
	    -DLOCALEDIR=\"$(datadir)/locale\"

gummi_beta_LDADD = $(GUI_LIBS) \
	      $(LIBINTL)

gummi_beta_SOURCES = biblio.c  biblio.h \
		configfile.c configfile.h \
		editor.c editor.h \
		environment.c environment.h \
		fileinfo.c fileinco.h \
		gui.c gui.h \
		importer.c importer.h \
		iofunctions.c iofunctions.h \
		main.c main.h \
		motion.c motion.h \
		preview.c preview.h \
		template.c template.h \
		updatecheck.c updatecheck.h \
		utils.c utils.h \
		gettext.h
