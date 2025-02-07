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

#ifndef INTERACTIVEFORMAT_H
#define INTERACTIVEFORMAT_H

#include <QDateTime>
#include <QList>
#include "fileformat.h"

class ImpCandidates;
struct ImpRecCandidate
{
    // Source info
    QString source, uid;
    int lineNumber;
    enum State {
        ParseError,
        UnknownAlias,
        UnknownCategory,   // or subcategory
        AmbiguousCategory, // or subcategory
        PossiblyDup,
        ReadyToImport
    } state;
    enum Type {
        Unknown, // if state==ParseError
        Income,
        Expense,
        Receipt,
        Transfer,
        Debtor,
        Creditor,
        IncomePlan,
        ExpensePlan
    } type;
    // Parsed info
    QDateTime &opDT;
    int idAcc, idCat, idSubcat, idCur, idUnit;
    int idAccTo; // for transfers
    int amount;
    double quantity;
    QString catName, subcatName, curName, descr;
    ImpCandidates* children; // for receipts
};

struct ImpCandidates: public QList<ImpRecCandidate>
{
    int idCurDefault;
    QString defaultCurName;
    bool readyToImport();
};

class InteractiveFormat : public FileFormat
{
public:
    InteractiveFormat();
    virtual bool isDialogRequired();
    virtual bool postImport(HwDatabase& db);
public:
    ImpCandidates candidates;
};

#endif // INTERACTIVEFORMAT_H
