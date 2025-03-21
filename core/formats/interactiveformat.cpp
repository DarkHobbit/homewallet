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

void InteractiveFormat::analyzeCandidates(HwDatabase &db)
{
  // TODO расширить State
    // TODO
}

bool InteractiveFormat::postImport(HwDatabase& db)
{
    _totalRecordsCount = candidates.count();
    _importedRecordsCount = 0;
    for (const ImpRecCandidate&c: candidates) {
        if (c.state!=ImpRecCandidate::ReadyToImport)
            return false;
        int idCurActual = (c.idCur==-1) ? candidates.idCurDefault : c.idCur;
        switch (c.type) {
        case ImpRecCandidate::Expense:
            if (!db.addExpenseOp(c.opDT, c.quantity, c.amount, c.idAcc, idCurActual,
                    c.idSubcat, c.idUnit, -1, 0, false, c.descr, _idImp, c.uid))
                return false;
            break;
        case ImpRecCandidate::Income:
            if (!db.addIncomeOp(c.opDT, c.quantity, c.amount, c.idAcc, idCurActual,
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
        _importedRecordsCount++;
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
    idAcc = idAccTo = idCat = idSubcat = idCur = idUnit = 0;
    amount = 0;
    quantity = 0.0;
    alias = catName = subcatName = accName = unitName = currName = descr = "";
}
