# Releases use autotools, this makefile is for development purposes. 

TARGET=gummi

OBJS = main.o gui/gui-main.o syncTeX/synctex_parser.o syncTeX/synctex_parser_utils.o gui/gui-prefs.o gui/gui-menu.o gui/gui-search.o gui/gui-import.o gui/gui-preview.o gui/gui-tabmanager.o gui/gui-project.o gui/gui-snippets.o gui/gui-infoscreen.o compile/texlive.o compile/rubber.o compile/latexmk.o motion.o external.o latex.o editor.o utils.o configfile.o iofunctions.o environment.o project.o importer.o update.o tabmanager.o template.o biblio.o snippets.o signals.o 


CFLAGS=-g -Wall -export-dynamic -I. `pkg-config --cflags --libs gtk+-2.0 gtksourceview-2.0 cairo poppler-glib gtkspell-2.0 zlib` -lm -DUSE_GTKSPELL -DLOCALEDIR="\"/usr/share/locale\"" -DDATADIR="\"../data\"" -DLIBDIR="\"$$PWD/../lib\""

gummi: $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)

clean:
	rm -f $(TARGET) $(OBJS)
