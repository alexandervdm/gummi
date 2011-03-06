# Tempraroly Makefile for development, the release will use GNU autotools

TARGET=gummi
OBJS = main.o gui/gui-main.o gui/gui-prefs.o gui/gui-search.o gui/gui-import.o gui/gui-preview.o gui/gui-editortabs.o gui/gui-snippets.o motion.o latex.o editor.o utils.o configfile.o iofunctions.o environment.o importer.o updatecheck.o template.o biblio.o snippets.o signals.o

CFLAGS=-g -Wall -export-dynamic -I. `pkg-config --cflags --libs gtk+-2.0 gtksourceview-2.0 cairo poppler-glib gtkspell-2.0` -DUSE_GTKSPELL -DLOCALEDIR="\"/usr/share/locale\"" -DDATADIR="\"../data\""

gummi: $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)

clean:
	rm -f $(TARGET) $(OBJS)
