#-------------------------------------------------
#
# Project created by QtCreator 2017-06-23T17:23:35
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NetShare
TEMPLATE = app


SOURCES += main.cpp\
    idmanager.cpp \
    aliasmanager.cpp \
    base.cpp \
    mainwidget.cpp \
    browsemanager.cpp \
    servermanager.cpp \
    singleapplication.cpp \
    applicationmanager.cpp \
    directorybroswer.cpp \
    downloadmanager.cpp \
    helpmanager.cpp

HEADERS  += \
    idmanager.h \
    aliasmanager.h \
    base.h \
    mainwidget.h \
    browsemanager.h \
    servermanager.h \
    singleapplication.h \
    applicationmanager.h \
    directorybroswer.h \
    downloadmanager.h \
    helpmanager.h

FORMS    += \
    idmanager.ui \
    aliasmanager.ui \
    base.ui \
    mainwidget.ui \
    browsemanager.ui \
    servermanager.ui \
    directorybroswer.ui \
    downloadmanager.ui \
    helpmanager.ui \
    infomationmanager.ui

RC_FILE += rsc.rc

RESOURCES += \
    icon.qrc
