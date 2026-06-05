QT += sql widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

contains(QMAKE_CXX, g++) {
    # For older GCCs, such as gcc 4.7.2 (also work on modern gcc, don't work on Mac with clang 7.0.2 aka 700.1.81)
    QMAKE_CXXFLAGS += -std=c++0x -fpermissive
}

INCLUDEPATH += \
    ../../core \
../../model \

SOURCES += \
    ../../core/corehelpers.cpp \
    ../../core/genericdatabase.cpp \
    ../../core/globals.cpp \
    ../../core/hwdatabase.cpp \
    ../../core/pathmanager.cpp \
    ../../model/categoryhiermodel.cpp \
    ../../model/configmanager.cpp \
    ../../model/filteredquerymodel.cpp \
    ../../model/hierfilterproxymodel.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    ../../core/corehelpers.h \
    ../../core/genericdatabase.h \
    ../../core/globals.h \
    ../../core/hwdatabase.h \
    ../../core/pathmanager.h \
    ../../model/categoryhiermodel.h \
    ../../model/configmanager.h \
    ../../model/filteredquerymodel.h \
    ../../model/hierfilterproxymodel.h \
    widget.h

FORMS += \
    widget.ui

LIBS += -lsqlite3

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
