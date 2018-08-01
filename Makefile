all : wdb

wdb: wdb.c
	$(CC) -o $@ wdb.c linenoise/linenoise.c `pkg-config glib-2.0 --cflags --libs` -I ./linenoise/