TARGET = qtservice
TEMPLATE = lib
DEPENDPATH += .
INCLUDEPATH += . \
               ../include
!win32:QT += network
win32:LIBS += -luser32
DEFINES += QT_QTSERVICE_EXPORT

win32:CONFIG += debug_and_release build_all
win32:CONFIG -= embed_manifest_dll
unix:CONFIG += release

win32-msvc*:contains(QMAKE_TARGET.arch, x86_64):{
	QMAKE_LFLAGS *= /MACHINE:X64

	CONFIG(debug, debug|release) {
		DESTDIR = ../../../../run/bin64_debug
		OBJECTS_DIR = ../../../../obj/debug64/vz/qtservice
		MOC_DIR = ../../../../obj/debug64/vz/qtservice
	} else {
		DESTDIR = ../../../../run/bin64
		OBJECTS_DIR = ../../../../obj/release64/vz/qtservice
		MOC_DIR = ../../../../obj/release64/vz/qtservice
	}
} else {
	CONFIG(debug, debug|release) {
		DESTDIR = ../../../../run/bin_debug
		OBJECTS_DIR = ../../../../obj/debug/vz/qtservice
		MOC_DIR = ../../../../obj/debug/vz/qtservice
	} else {
		DESTDIR = ../../../../run/bin
		OBJECTS_DIR = ../../../../obj/release/vz/qtservice
		MOC_DIR = ../../../../obj/release/vz/qtservice
	}
}

HEADERS       += ../include/qtservice.h \
				 qtservice_p.h

SOURCES       += qtservice.cpp

win32:SOURCES += qtservice_win.cpp

unix:HEADERS  += qtunixsocket.h \
                 qtunixserversocket.h

unix:SOURCES  += qtservice_unix.cpp \
                 qtunixsocket.cpp \
				 qtunixserversocket.cpp

