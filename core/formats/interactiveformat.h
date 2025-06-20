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
        Initial,
        ParseError,
        UnknownAlias,
        UnknownCategory,   // or subcategory
        AmbiguousCategory, // or subcategory
        UnknownAccount,
        UnknownUnit,
        UnknownCurrency,
        PossiblyDup,
        ReadyToImport
    } state;
    enum Type {
        Unknown, // if state==ParseError or Initial
        Income,
        Expense,
        ReceiptStart,
        ReceiptEnd,
        Transfer,
        Debtor,
        Creditor/*,
        IncomePlan,
        ExpensePlan*/
    } type;
    // Parsed info
    QDateTime opDT;
    int idAcc, idCat, idSubcat, idCur, idUnit;
    int idAccTo; // for transfers
    int amount;
    double quantity;
    QString alias, catName, subcatName, accName, accToName, unitName, currName, descr;
    ImpCandidates* children; // for receipts
    ImpRecCandidate* parent; // for receipt items
    ImpRecCandidate(const QString& _source, const QString& _uid, int _lineNumber, const QDateTime& _opDT);
};

struct ImpCandidates: public QList<ImpRecCandidate>
{
    int idCurDefault, idAccDefault;
    QString accDefault, curDefault;
    bool readyToImport();
};

class InteractiveFormat : public FileFormat
{
public:
    InteractiveFormat();
    virtual void clear();
    virtual bool isDialogRequired();
    bool analyzeCandidates(HwDatabase &db);
    virtual bool postImport(HwDatabase& db);
public:
    ImpCandidates candidates;
private:
    bool findAccount(HwDatabase &db, ImpRecCandidate& c, QString &accName, int &idAcc);
    bool findCurrency(HwDatabase &db, ImpRecCandidate& c, QString &currAbbr, int &idCurr);
    bool findUnit(HwDatabase &db, ImpRecCandidate& c, QString& name, int& idUn);
};

#endif // INTERACTIVEFORMAT_H
