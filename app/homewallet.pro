#-------------------------------------------------
# Home Wallet
# Copyright 2023 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version. See COPYING file for more details.
#
# Project created by QtCreator 2023-12-30T12:45:59
#-------------------------------------------------

QT       += core gui

# For older GCCs, such as gcc 4.7.2 (also work on modern gcc, don't work on Mac with clang 7.0.2 aka 700.1.81)
# QMAKE_CXXFLAGS += -std=c++0x -fpermissive

include(../core/core.pri)
include(../model/model.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = homewallet
TEMPLATE = app


SOURCES += \
    aboutdialog.cpp \
    helpers.cpp \
    languageselectdialog.cpp \
    logwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    postimportdialog.cpp

HEADERS  += \
    aboutdialog.h \
    helpers.h \
    languageselectdialog.h \
    logwindow.h \
    mainwindow.h \
    postimportdialog.h

FORMS    += \
    aboutdialog.ui \
    languageselectdialog.ui \
    logwindow.ui \
    mainwindow.ui \
    postimportdialog.ui

unix { 
    OBJECTS_DIR = .obj
    UI_DIR = .ui
    MOC_DIR = .moc
}

TRANSLATIONS += \
    ../translations/homewallet_ru_RU.ts \


exists( $$dirname(QMAKE_QMAKE)/lrelease-qt4 ) {
      tr.commands = $$dirname(QMAKE_QMAKE)/lrelease-qt4  $$_PRO_FILE_
} else:exists( $$dirname(QMAKE_QMAKE)/lrelease-qt5 ) {
      tr.commands = $$dirname(QMAKE_QMAKE)/lrelease-qt5  $$_PRO_FILE_
} else {
      tr.commands = $$dirname(QMAKE_QMAKE)/lrelease  $$_PRO_FILE_
}
    
QMAKE_EXTRA_TARGETS += tr
POST_TARGETDEPS += tr

RESOURCES += \
    homewallet.qrc
