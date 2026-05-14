# Model-related part of HomeWallet

QT += gui

INCLUDEPATH += $$PWD

HEADERS	+= \
    $$PWD/categoriesbasedquerymodel.h \
    $$PWD/categoryhiermodel.h \
    $$PWD/configmanager.h \
    $$PWD/creditmodel.h \
    $$PWD/currconvmodel.h \
    $$PWD/expensemodel.h \
    $$PWD/filteredquerymodel.h \
    $$PWD/hierfilterproxymodel.h \
    $$PWD/importcandidatesmodel.h \
    $$PWD/importmodelset.h \
    $$PWD/incomemodel.h \
    $$PWD/miscmodels.h \
    $$PWD/testmanager.h \
    $$PWD/transfermodel.h

SOURCES	+= \
    $$PWD/categoriesbasedquerymodel.cpp \
    $$PWD/categoryhiermodel.cpp \
    $$PWD/configmanager.cpp \
    $$PWD/creditmodel.cpp \
    $$PWD/currconvmodel.cpp \
    $$PWD/expensemodel.cpp \
    $$PWD/filteredquerymodel.cpp \
    $$PWD/hierfilterproxymodel.cpp \
    $$PWD/importcandidatesmodel.cpp \
    $$PWD/importmodelset.cpp \
    $$PWD/incomemodel.cpp \
    $$PWD/miscmodels.cpp \
    $$PWD/testmanager.cpp \
    $$PWD/transfermodel.cpp

