# Core (GUI-independent) part of HomeWallet

QT += core sql xml

INCLUDEPATH += $$PWD

HEADERS	+= \
    $$PWD/corehelpers.h \
    $$PWD/formats/commonexpimpdef.h \
    $$PWD/formats/interactiveformat.h \
    $$PWD/formats/xmlfile.h \
    $$PWD/genericdatabase.h \
    $$PWD/globals.h \
    $$PWD/hwdatabase.h \
    $$PWD/languagemanager.h \
    $$PWD/pathmanager.h \
    $$PWD/formats/fileformat.h \
    $$PWD/formats/xmlhbfile.h \

SOURCES	+= \
    $$PWD/corehelpers.cpp \
    $$PWD/formats/commonexpimpdef.cpp \
    $$PWD/formats/interactiveformat.cpp \
    $$PWD/formats/xmlfile.cpp \
    $$PWD/genericdatabase.cpp \
    $$PWD/globals.cpp \
    $$PWD/hwdatabase.cpp \
    $$PWD/languagemanager.cpp \
    $$PWD/pathmanager.cpp \
    $$PWD/formats/fileformat.cpp \
    $$PWD/formats/xmlhbfile.cpp \

DISTFILES += \
    $$PWD/dbinit/dbinit.sql \
    $$PWD/dbinit/loadcurrency.sql

