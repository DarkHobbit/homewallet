/* Home wallet
 *
 * Module: Semi-Abstract class for alias-based file formats
 * (interactive learning required)
 * Copyright 2016 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QSqlError>
#include "commonexpimpdef.h"
#include "interactiveformat.h"

bool ImpCandidates::readyToImport()
{
    if (isEmpty())
        return false;
    for (const ImpRecCandidate&c: *this) {
        if (c.state!=ImpRecCandidate::ReadyToImport)
            return false;
    }
    return true;
}

InteractiveFormat::InteractiveFormat()
    :FileFormat()
{}

void InteractiveFormat::clear()
{
    FileFormat::clear();
    candidates.clear();
}

bool InteractiveFormat::isDialogRequired()
{
    return !candidates.readyToImport();
}

bool InteractiveFormat::analyzeCandidates(HwDatabase &db)
{
    // Defaults
    candidates.curDefault = QString::fromUtf8("₽");
    // TODO default currency either from DB, or from lua script
    candidates.accDefault = QString::fromUtf8("Наличные");
    // TODO default account either from DB, or from lua script

    candidates.idCurDefault = db.currencyIdByAbbr(candidates.curDefault);
    if (candidates.idCurDefault == -1) {
        _fatalError = S_ERR_CUR_NOT_FOUND.arg(candidates.curDefault);
        return false;
    }
    candidates.idAccDefault = db.accountId(candidates.accDefault);
    if (candidates.idAccDefault == -1) {
        _fatalError = S_ERR_ACC_NOT_FOUND.arg(candidates.accDefault);
        return false;
    }
    // Dictionaries
    candidates.collAcc.clear();
    db.collectDict(candidates.collAcc, "hw_alias", "pattern", "id_ac", "where id_ac is not null");
    candidates.collCurr.clear();
    db.collectDict(candidates.collCurr, "hw_alias", "pattern", "id_cur", "where id_cur is not null");
    candidates.collUnit.clear();
    db.collectDict(candidates.collUnit, "hw_alias", "pattern", "id_un", "where id_un is not null");
    // TODO other dicts

    // See candidates
    for (ImpRecCandidate& c: candidates) {
        switch(c.type) {
        case ImpRecCandidate::Unknown:
            continue;
        case ImpRecCandidate::Expense:
            if (!findAccount(db, c, c.accName, c.idAcc))
                continue;
            // TODO name to abbr!!!
            if (!findCurrency(db, c, c.currName, c.idCur))
                continue;
            if (!findUnit(db, c, c.unitName, c.idUnit))
                continue;
            if (!c.subcatName.isEmpty()) { // Full-qualified subcategory
                c.idCat = db.expenseCategoryId(c.catName);
                if (c.idCat==-1) {
                    // TODO alias for category
                    c.state = ImpRecCandidate::UnknownCategory;
                    continue;
                }
                c.idSubcat = db.expenseSubCategoryId(c.idCat, c.subcatName);
                if (c.idSubcat==-1) {
                    // TODO alias for subcategory
                    c.state = ImpRecCandidate::UnknownCategory;
                    continue;
                }
                // Correct upper/low case
                c.catName = db.expenseCategoryById(c.idCat);
                c.subcatName = db.expenseSubCategoryById(c.idSubcat);
                c.state = ImpRecCandidate::ReadyToImport;
            }
            else { // Partially-qualified subcategory or alias
                QSqlQuery q;
                DB_CHK(q.prepare("select id, id_ecat from hw_ex_subcat where upper(name)=:name"));
                q.bindValue(":name", c.alias.toUpper());
                DB_CHK(q.exec());
                // TODO save ecats here for future ask?
                bool needSearchAlias = false;
                switch(db.queryRecCount(q)) {
                case 0:
                    needSearchAlias = true;
                    break;
                case 1:
                    q.first();
                    c.idCat = q.value("id_ecat").toInt();
                    c.idSubcat = q.value("id").toInt();
                    // Find category name and correct case
                    DB_CHK(q.prepare("select name from hw_ex_cat where id=:id"));
                    q.bindValue(":id", c.idCat);
                    DB_CHK(q.exec());
                    q.first();
                    c.catName = q.value("name").toString();
                    c.subcatName = db.expenseSubCategoryById(c.idSubcat);
                    c.state = ImpRecCandidate::ReadyToImport;
                    break;
                default:
                    c.state = ImpRecCandidate::AmbiguousCategory;
                }
                if (needSearchAlias) { // Alias!

                    // TODO
                    c.state = ImpRecCandidate::UnknownAlias; //===>
                }
            }
            break;
        case ImpRecCandidate::Income:
            // TODO
            break;
        case ImpRecCandidate::Transfer:
            if (!findAccount(db, c, c.accName, c.idAcc))
                continue;
            if (!findAccount(db, c, c.accToName, c.idAccTo))
                continue;
            // TODO
            break;
        default:
            continue;
        }

        // TODO
    }
    return true;
}

bool InteractiveFormat::postImport(HwDatabase& db)
{
    _totalRecordsCount = candidates.count();
    _processedRecordsCount = 0;
    for (const ImpRecCandidate&c: candidates)
        if (c.state!=ImpRecCandidate::ReadyToImport)
            return false;
    for (const ImpRecCandidate&c: candidates) {
        int idCurActual = /*(c.idCur==-1) ? candidates.idCurDefault : */c.idCur;
        int idAccActual = /*(c.idAcc==-1) ? candidates.idAccDefault : */c.idAcc;
        switch (c.type) {
        case ImpRecCandidate::Expense:
            if (!db.addExpenseOp(c.opDT, c.quantity, c.amount, idAccActual, idCurActual,
                    c.idSubcat, c.idUnit, -1, 0, false, c.descr, _idImp, c.uid))
                return false;
            break;
        case ImpRecCandidate::Income:
            if (!db.addIncomeOp(c.opDT, c.quantity, c.amount, idAccActual, idCurActual,
                    c.idSubcat, c.idUnit, false, c.descr, _idImp, c.uid))
                return false;
            break;
        case ImpRecCandidate::ReceiptStart:
            // TODO addReceipt, then for children
            break;
        case ImpRecCandidate::ReceiptEnd:
            // TODO check anything?
            break;
        case ImpRecCandidate::Transfer:
            // TODO addTransfer
            break;
            // TODO other recordtypes
        default:
            return false;
        }
        _processedRecordsCount++;
    }
    return true;
}

bool InteractiveFormat::findAccount(HwDatabase &db, ImpRecCandidate& c, QString &accName, int &idAcc)
{
    if (accName.isEmpty()) {
        accName = candidates.accDefault;
        idAcc = candidates.idAccDefault;
        return true;
    }
    idAcc = db.accountId(c.accName);
    if (idAcc==-1) {
        // Try alias
        if (candidates.collAcc.keys().contains(accName)) {
            idAcc = candidates.collAcc[accName];
            return true;
        }
        c.state = ImpRecCandidate::UnknownAccount;
        return false;
    }
    return true;
}

bool InteractiveFormat::findCurrency(HwDatabase &db, ImpRecCandidate &c, QString &currAbbr, int &idCurr)
{
    if (currAbbr.isEmpty()) {
        currAbbr = candidates.curDefault;
        idCurr = candidates.idCurDefault;
        return true;
    }
    idCurr = db.currencyIdByAbbr(currAbbr);
    if (idCurr==-1) {
        // Try alias
        if (candidates.collCurr.keys().contains(currAbbr)) {
            idCurr = candidates.collCurr[currAbbr];
            return true;
        }
        c.state = ImpRecCandidate::UnknownCurrency;
        return false;
    }
    return true;
}

bool InteractiveFormat::findUnit(HwDatabase &db, ImpRecCandidate &c, QString &name, int &idUn)
{
    /*
    if (name.isEmpty())
        return true;
*/
    idUn = db.unitId(name);
    if (idUn==-1) {
        idUn = db.unitId(name+".");
        if (idUn!=-1) {
            name = name+".";
            return true;
        }
        // Try alias
        if (candidates.collUnit.keys().contains(name)) {
            idUn = candidates.collUnit[name];
            return true;
        }
        c.state = ImpRecCandidate::UnknownUnit;
        return false;
    }
    return true;
}


ImpRecCandidate::ImpRecCandidate(const QString &_source, const QString &_uid, int _lineNumber, const QDateTime& _opDT)
    :source(_source), uid(_uid), lineNumber(_lineNumber), opDT(_opDT)
{
    children = 0;
    parent = 0;
    state = Initial;
    type = Unknown;
    idAcc = idAccTo = idCat = idSubcat = idCur = idUnit = -1;
    amount = 0;
    quantity = 0.0;
    alias = catName = subcatName = accName = unitName = currName = descr = "";
}

bool ImpRecCandidate::needAddAlias()
{
    return
        state==UnknownAccount
        || state==UnknownCurrency
        || state==UnknownUnit
        || state==UnknownAlias
        || state==UnknownCategory
        || state==UnknownTransType;
}
