/* Home Wallet
 *
 * Module: Money application database wrapper
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef HWDATABASE_H
#define HWDATABASE_H

#include <QObject>
#include "genericdatabase.h"

#define S_FIRST_TIME QObject::tr("It seems you ran HomeWallet for the first time.")
#define S_WILL_CREAT QObject::tr("Database will created in %1")
#define S_ALIEN_DB QObject::tr("Database is not HomeWallet format\n in %1")
#define S_NEED_UPGRADE \
 QObject::tr("Database structure out of time\n in %1\(%2)\n Upgrade it to actual state?")
#define S_REPEAT_IMPORT \
QObject::tr("File was already imported:\n%1\n Repeat it?")

class HwDatabase: public GenericDatabase
{
public:
    enum DBFileState {
        OpenError,
        Alien,       // SQLite database, but not HW database
        NeedUpgrade, // need to upgrade :)
        Actual       // nothing to do
    };
    typedef QMap<QString, int> MultiCurrByChar; // key - $ (etc), value - sum
    typedef QMap<int, int> MultiCurrById; // key - id, value - sum
    HwDatabase();
    ~HwDatabase();
    // Entire database actions
    DBFileState test(const QString& dir);
    bool upgrade(const QString& dir);
    bool create(const QString& dir);
    bool isEmpty();
    // Export & Import
    int addImportFile(const QString& fileName, const QString& fileType);
    int findImportFile(const QString& fileName);
    // Subject area
    virtual QString fileName();
    void getCounts(int& totalInCount, int& totalExpCount);
    bool addAccount(const QString& name, const QString& descr,
                    const QDateTime& foundation=QDateTime(), const MultiCurrById& startBalance=MultiCurrById());
    int accountId(const QString& name);
    int addUnit(const QString& name, const QString& shortName, const QString& descr);
    int unitId(const QString& name);
    int currencyIdByAbbr(const QString& abbr);
    // Categories
    int addIncomeCategory(const QString& name, const QString& descr);
    int incomeCategoryId(const QString& name);
    int addIncomeSubCategory(int idParentCat, const QString& name, const QString& descr);
    int incomeSubCategoryId(int idParentCat, const QString& name);
    int addExpenseCategory(const QString& name, const QString& descr);
    int expenseCategoryId(const QString& name);
    QString expenseCategoryById(int idCat);
    int addExpenseSubCategory(int idParentCat, const QString& name, const QString& descr);
    int expenseSubCategoryId(int idParentCat, const QString& name);
    QString expenseSubCategoryById(int idSubCat);
    int addTransferType(const QString& name, const QString& descr);
    int transferTypeId(const QString& name);
    // Inc/Exp
    bool addIncomeOp(const QDateTime& opDT, double quantity,
        int amount, int idAcc, int idCur, int idSubcat, int idUnit,
        bool attention, const QString& descr, int idImp=-1, const QString& uid="");
    bool addExpenseOp(const QDateTime& opDT, double quantity,
        int amount, int idAcc, int idCur, int idSubcat, int idUnit, int idReceipt,
        int discount, bool attention, const QString& descr, int idImp=-1, const QString& uid="");
    // Transfer, currencyConv
    bool addTransfer(const QDateTime& opDT, int amount, int idCur, int idAccFrom, int idAccTo,
        int idTransferType, const QString& descr, int idImp=-1, const QString& uid="");
    bool addCurrencyConv(const QDateTime& opDT, int idAcc,
        int idCurFrom, int amountFrom,
        int idCurTo, int amountTo,
        const QString& descr, int idImp=-1, const QString& uid="");
};

#endif // HWDATABASE_H
