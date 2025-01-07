# Model-related part of HomeWallet

QT += gui

INCLUDEPATH += $$PWD

HEADERS	+= \
    $$PWD/categoriesbasedquerymodel.h \
    $$PWD/configmanager.h \
    $$PWD/expensemodel.h \
    $$PWD/filteredquerymodel.h \
    $$PWD/testmanager.h

SOURCES	+= \
    $$PWD/categoriesbasedquerymodel.cpp \
    $$PWD/configmanager.cpp \
    $$PWD/expensemodel.cpp \
    $$PWD/filteredquerymodel.cpp \
    $$PWD/testmanager.cpp

