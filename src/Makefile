all: GtkPainter

CXXFLAGS = -O0 -DLINUX -DUSE_STDIO_H -fexec-charset=cp1251 -finput-charset=cp1251 -Wno-deprecated `pkg-config gtkmm-2.4 --cflags` -ggdb
LDFLAGS = `pkg-config gtkmm-2.4 --libs` -ggdb

OBJS = TreSubfile.o SubDivision.o SubFile.o MapLevel.o Common.o \
	PlatformDef.o LblSubfile.o BitStream.o FATBlock.o NetSubfile.o \
	GtkPainter.o RgnSubfile.o Header.o File.o

GtkPainter: ${OBJS}
	g++ ${LDFLAGS} -o GtkPainter ${OBJS}

clean:
	rm -f ${OBJS} GtkPainter
