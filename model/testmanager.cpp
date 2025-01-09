/* Home Wallet
 *
 * Module: Test utilities
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <iostream>
#include <QSqlQuery>
#include "testmanager.h"

TestManager::TestManager()
{
}

QSqlQueryModel *TestManager::dbDebug(const QString &queryText, GenericDatabase &db)
{
    QSqlQuery q;
    q.prepare(queryText);
    q.exec();
    QSqlQueryModel* m = new QSqlQueryModel();
    m->setQuery(q);
    while (m->canFetchMore())
        m->fetchMore();
    return m;
}

bool TestManager::createTestData(HwDatabase &db)
{
    // General
    if (!db.addAccount("VISA Mary", "Special expense card")) {
        std::cerr << "Account add error" << std::endl;
        return false;
    }
    int idAcc = db.accountId("VISA Mary");
    GenericDatabase::DictColl currs;
    db.collectDict(currs, "hw_currency", "short_name");
    if (currs.isEmpty()) {
        std::cerr << "Currencies not loaded" << std::endl;
        return false;
    }
    int idCur = currs.values().first();
    int idUnit = db.addUnit("kilogram", "kg", "");
    // Incomes
    int idCat = db.addIncomeCategory("Work", "Main work");
    if (idCat==-1) {
        std::cerr << "Income category add error" << std::endl;
        return false;
    }
    int idSubcat = db.addIncomeSubCategory(idCat, "Prepayment", "");
    if (idSubcat==-1) {
        std::cerr << "Income subcategory add error" << std::endl;
        return false;
    }
    if (!db.addIncomeOp(QDateTime::currentDateTime(),
        1, 200, idAcc, idCur, idSubcat, idUnit, false, "")) {
        std::cerr << "Income operation add error" << std::endl;
        return false;
    }
    // Expenses
    idCat = db.addExpenseCategory("Food", "All food");
    if (idCat==-1) {
        std::cerr << "Expense category add error" << std::endl;
        return false;
    }
    idSubcat = db.addExpenseSubCategory(idCat, "Bread", "");
    if (idSubcat==-1) {
        std::cerr << "Expense subcategory add error" << std::endl;
        return false;
    }
    if (!db.addExpenseOp(QDateTime::currentDateTime(),
          15, 100, idAcc, idCur, idSubcat, idUnit, -1, 0, false, "")) {
        std::cerr << "Expense operation add error" << std::endl;
        return false;
    }
    return true;
}
