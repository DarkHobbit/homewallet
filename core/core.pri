# Core (GUI-independent) part of HomeWallet

QT += core sql xml

# Don't work on macOS currently
!mac:DEFINES += WITH_SQLITE_EXTENSIONS

include(../3rdparty/quazip/quazip.pri)

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

INCLUDEPATH += $$PWD

HEADERS	+= \
    $$PWD/corehelpers.h \
    $$PWD/formats/commonexpimpdef.h \
    $$PWD/formats/fileformat.h \
    $$PWD/formats/formatfactory.h \
    $$PWD/formats/hbhelper.h \
    $$PWD/formats/interactiveformat.h \
    $$PWD/formats/simplexlsxreader.h \
    $$PWD/formats/txtcompactfile.h \
    $$PWD/formats/xlsxrepaymentfile.h \
    $$PWD/formats/xmlfile.h \
    $$PWD/formats/xmlhbfile.h \
    $$PWD/formats/xmlhwfile.h \
    $$PWD/genericdatabase.h \
    $$PWD/globals.h \
    $$PWD/hwdatabase.h \
    $$PWD/languagemanager.h \
    $$PWD/pathmanager.h \

SOURCES	+= \
    $$PWD/corehelpers.cpp \
    $$PWD/formats/commonexpimpdef.cpp \
    $$PWD/formats/fileformat.cpp \
    $$PWD/formats/formatfactory.cpp \
    $$PWD/formats/hbhelper.cpp \
    $$PWD/formats/interactiveformat.cpp \
    $$PWD/formats/simplexlsxreader.cpp \
    $$PWD/formats/txtcompactfile.cpp \
    $$PWD/formats/xlsxrepaymentfile.cpp \
    $$PWD/formats/xmlfile.cpp \
    $$PWD/formats/xmlhbfile.cpp \
    $$PWD/formats/xmlhwfile.cpp \
    $$PWD/genericdatabase.cpp \
    $$PWD/globals.cpp \
    $$PWD/hwdatabase.cpp \
    $$PWD/languagemanager.cpp \
    $$PWD/pathmanager.cpp \

DISTFILES += \
    $$PWD/dbinit/dbinit.sql \
    $$PWD/dbinit/debugquerysamples.sql \
    $$PWD/dbinit/loadcurrency.sql

LIBS += -lsqlite3

