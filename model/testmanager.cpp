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

bool TestManager::createTestData(HwDatabase &db, int numberOfExpenses)
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
    QDateTime testDate = QDateTime::currentDateTime();
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
    if (!db.addIncomeOp(testDate,
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
    int idSubcat1 = db.addExpenseSubCategory(idCat, "Butter", "");
    if (idSubcat1==-1) {
        std::cerr << "Expense subcategory add error" << std::endl;
        return false;
    }
    int idSubcat2 = db.addExpenseSubCategory(idCat, "Bread", "");
    if (idSubcat2==-1) {
        std::cerr << "Expense subcategory add error" << std::endl;
        return false;
    }
    double quantity = 5;
    for (int i=0; i<numberOfExpenses; i++) {
        if (!db.addExpenseOp(testDate,
                quantity, 100, idAcc, idCur,
                i%2 ? idSubcat1 : idSubcat2,
                idUnit, -1, 0, false, "")) {
            std::cerr << "Expense operation add error" << std::endl;
            return false;
        }
        if (!((i+1)%500))
            std::cout << "Record " << i+1 << " from " << numberOfExpenses << " inserted" << std::endl;
        if (!(i%5))
            testDate = testDate.addDays(1);
        quantity += 0.01;
    }
    return true;
}
