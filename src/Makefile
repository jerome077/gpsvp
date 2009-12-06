all: GtkPainter

# CXXFLAGS = -O0 -DLINUX -DUSE_STDIO_H -fexec-charset=cp1251 -finput-charset=cp1251 -Wno-deprecated `pkg-config gtkmm-2.4 hildonmm hildon-fmmm --cflags` -ggdb
CXXFLAGS = -O0 -DLINUX -DUSE_STDIO_H -fexec-charset=cp1251 -finput-charset=cp1251 -Wno-deprecated `pkg-config gtkmm-2.4 --cflags` -ggdb
LDFLAGS = `pkg-config gtkmm-2.4 --libs` `pkg-config gthread-2.0 --libs` -ggdb

OBJS = TreSubfile.o SubDivision.o SubFile.o MapLevel.o \
	PlatformDef.o LblSubfile.o BitStream.o FATBlock.o NetSubfile.o \
	RgnSubfile.o Header.o File.o Atlas.o NMEAParser.o EGM96Geoid.o \
	MapApp.o Common.o OptionSet.o Track.o WayPoints.o Lock.o ScreenPoint.o GtkPainter.o \
	Traffic.o GeoPoint.o MonitorSet.o TrackCompetition.o Monitors.o MRUPoints.o MaemoMain.o

# OBJS = test.o

MapApp.o: GtkPainter.h ScreenToGeo.h Menu.h

GtkPainter: ${OBJS}
	g++ ${LDFLAGS} -o GtkPainter ${OBJS}

clean:
	rm -f ${OBJS} GtkPainter
