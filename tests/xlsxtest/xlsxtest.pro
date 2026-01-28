#-------------------------------------------------
#
# Project created by QtCreator 2025-12-11T12:17:05
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = xlsxtest
TEMPLATE = app

include(../../3rdparty/quazip/quazip.pri)

DEFINES += QUAZIP_STATIC

contains(CONFIG, static){
  INCLUDEPATH += $(QTDIR)/src/3rdparty/zlib
}
win32 {
  INCLUDEPATH += $(QTDIR)/src/3rdparty/zlib
  greaterThan(QT_MAJOR_VERSION, 4) {
LIBS += -lz
}
}
unix:LIBS += -lz

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../../core/formats

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        ../../core/formats/simplexlsxreader.cpp

HEADERS += \
        mainwindow.h \
        ../../core/formats/simplexlsxreader.h

FORMS += \
        mainwindow.ui
