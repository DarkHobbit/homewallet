# Core (GUI-independent) part of HomeWallet

QT += core sql xml

INCLUDEPATH += $$PWD

HEADERS	+= \
    $$PWD/corehelpers.h \
    $$PWD/formats/commonexpimpdef.h \
    $$PWD/formats/fileformat.h \
    $$PWD/formats/formatfactory.h \
    $$PWD/formats/interactiveformat.h \
    $$PWD/formats/txtcompactfile.h \
    $$PWD/formats/xmlfile.h \
    $$PWD/formats/xmlhbfile.h \
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
    $$PWD/formats/interactiveformat.cpp \
    $$PWD/formats/txtcompactfile.cpp \
    $$PWD/formats/xmlfile.cpp \
    $$PWD/formats/xmlhbfile.cpp \
    $$PWD/genericdatabase.cpp \
    $$PWD/globals.cpp \
    $$PWD/hwdatabase.cpp \
    $$PWD/languagemanager.cpp \
    $$PWD/pathmanager.cpp \

DISTFILES += \
    $$PWD/dbinit/dbinit.sql \
    $$PWD/dbinit/loadcurrency.sql

