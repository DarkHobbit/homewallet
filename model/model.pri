# Model-related part of HomeWallet

QT += gui

INCLUDEPATH += $$PWD

HEADERS	+= \
    $$PWD/categoriesbasedquerymodel.h \
    $$PWD/configmanager.h \
    $$PWD/expensemodel.h \
    $$PWD/filteredquerymodel.h \
    $$PWD/importcandidatesmodel.h \
    $$PWD/importmodelset.h \
    $$PWD/incomemodel.h \
    $$PWD/testmanager.h

SOURCES	+= \
    $$PWD/categoriesbasedquerymodel.cpp \
    $$PWD/configmanager.cpp \
    $$PWD/expensemodel.cpp \
    $$PWD/filteredquerymodel.cpp \
    $$PWD/importcandidatesmodel.cpp \
    $$PWD/importmodelset.cpp \
    $$PWD/incomemodel.cpp \
    $$PWD/testmanager.cpp

