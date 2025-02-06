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

bool InteractiveFormat::isDialogRequired()
{
    return !candidates.readyToImport();
}

bool InteractiveFormat::postImport(HwDatabase& db)
{
    _totalRecordsCount = candidates.count();
    _importedRecordsCount = 0;
    for (const ImpRecCandidate&c: candidates) {
        if (c.state!=ImpRecCandidate::ReadyToImport)
            return false;
        switch (c.type) {
        case ImpRecCandidate::Expense:
            if (!db.addExpenseOp(c.opDT, c.quantity, c.amount, c.idAcc,
                    -1,  // TODO default currency or from file!
                    c.idSubcat, c.idUnit, -1, 0, false, c.descr, _idImp, c.uid))
                return false;
            break;
        case ImpRecCandidate::Income:
            if (!db.addIncomeOp(c.opDT, c.quantity, c.amount, c.idAcc,
                    -1,  // TODO default currency or from file!
                    c.idSubcat, c.idUnit, false, c.descr, _idImp, c.uid))
                return false;
            break;
        case ImpRecCandidate::Receipt:
            // TODO addReceipt, then for children
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

