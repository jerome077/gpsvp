SRCS = Track.cpp WayPoints.cpp \
	GoogleMaps/GMCommon.cpp GoogleMaps/VariableInterpreter.cpp GoogleMaps/RasterServerSources.cpp GoogleMaps/GMFileHolder.cpp GoogleMaps/GMPainter.cpp \
	VisualPainter.cpp OptionSet.cpp Atlas.cpp File.cpp HttpClient.cpp Monitors.cpp PlatformDef.cpp SubDivision.cpp Traffic.cpp \
	BitStream.cpp FileDialogs.cpp Keymap.cpp MonitorSet.cpp RegValues.cpp SubFile.cpp TreSubfile.cpp \
	Common.cpp FontCache.cpp LblSubfile.cpp MRUPoints.cpp RgnSubfile.cpp Sun.cpp TypeInfo.cpp \
	DebugOutput.cpp GDIPainter.cpp Lock.cpp NetSubfile.cpp ScreenButtons.cpp Team.cpp \
	Dialogs.cpp GeoPoint.cpp MapApp.cpp \
	NMEAParser.cpp ScreenPoint.cpp TrackCompetition.cpp \
	FATBlock.cpp Header.cpp MapLevel.cpp EGM96Geoid.c
#SRCS += FileFormats/OSM.cpp FileFormats/GPX.cpp
#SRCS = Atlas.cpp
	
CXX_FLAGS = -mwindows -municode -Wno-write-strings -finput-charset=cp1251 -Wno-deprecated -DUNDER_WINE -DUNICODE -DWINE_UNICODE_NATIVE -DUSE_STDIO_H -DNOMINMAX -I/usr/include/wine/windows/ -I.
#CXX_FLAGS = -c -Wno-write-strings -finput-charset=cp1251 -Wno-deprecated -DUNDER_WINE -DUNICODE -DWINE_UNICODE_NATIVE -DUSE_STDIO_H -DNOMINMAX -I/usr/include/wine/windows/ -I.

all: sources gpsVP
sources:
	mkdir -p GoogleMaps
	for i in ../*.cpp ../*.c ../*.h ../GoogleMaps/*.cpp ../GoogleMaps/*.h ; do \
		test ! -f $${i#../} -o $${i} -nt $${i#../} && ( echo $${i} ; iconv -f utf8 -t cp1251 -c $$i > $${i#../} ) || true; \
	done
%.o: %.cpp
	wineg++ ${CXX_FLAGS} -c $< -o $@
%.o: %.c
	wineg++ ${CXX_FLAGS} -c $< -o $@
OBJS = $(subst .c,.o,$(subst .cpp,.o,$(SRCS)))
gpsVP: ${OBJS}
	wineg++ ${CXX_FLAGS} -loleaut32 ${OBJS} -o gpsVP
