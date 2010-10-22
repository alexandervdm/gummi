# Tempraroly Makefile for development, the release will use GNU autotools

TARGET=gummi
OBJS = gummi.o gummi-gui.o prefs-gui.o search-gui.o import-gui.o fileinfo.o preview.o motion.o editor.o utils.o configfile.o iofunctions.o environment.o importer.o updatecheck.o template.o biblio.o

CFLAGS=-g -Wall -export-dynamic `pkg-config --cflags --libs gtk+-2.0 gtksourceview-2.0 cairo poppler-glib gtkspell-2.0` -DUSE_GTKSPELL -DLOCALEDIR="\"/usr/share/locale\"" -DDATADIR="\"../data\""

gummi: $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)

clean:
	rm -f $(TARGET) $(OBJS)
