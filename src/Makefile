all: GtkPainter

CXXFLAGS = -O0 -DLINUX -DUSE_STDIO_H -fexec-charset=cp1251 -finput-charset=cp1251 -Wno-deprecated `pkg-config gtkmm-2.4 hildonmm --cflags` -ggdb
LDFLAGS = `pkg-config gtkmm-2.4 hildonmm --libs` `pkg-config gthread-2.0 --libs` -ggdb

OBJS = TreSubfile.o SubDivision.o SubFile.o MapLevel.o \
	PlatformDef.o LblSubfile.o BitStream.o FATBlock.o NetSubfile.o \
	RgnSubfile.o Header.o File.o Atlas.o NMEAParser.o EGM96Geoid.o \
	MapApp.o Common.o OptionSet.o Track.o WayPoints.o Lock.o ScreenPoint.o GtkPainter.o \
	Traffic.o GeoPoint.o MonitorSet.o TrackCompetition.o Monitors.o MRUPoints.o MaemoMain.o

Mapapp.o: GtkPainter.h ScreenToGeo.h

GtkPainter: ${OBJS}
	g++ ${LDFLAGS} -o GtkPainter ${OBJS}

clean:
	rm -f ${OBJS} GtkPainter
