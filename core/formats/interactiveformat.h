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

struct ImpCandidates;
struct ImpRecCandidate
{
    // Source info
    QString source, uid;
    int lineNumber;
    enum State {
        Initial,
        ParseError,
        UnknownAlias,
        UnknownAccount,
        UnknownCurrency,
        UnknownUnit,
        UnknownCategory,
        UnknownSubCategory,
        AmbiguousSubCategory,
        UnknownTransType,
        PossiblyDup,
        ReadyToImport,
        MaxState
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
    QString stateString[MaxState] = {
        QObject::tr("initial"),
        QObject::tr("parse error"),
        QObject::tr("unknown alias"),
        QObject::tr("unknown account"),
        QObject::tr("unknown currency"),
        QObject::tr("unknown unit"),
        QObject::tr("unknown category"),
        QObject::tr("unknown subcategory"),
        QObject::tr("ambiguous subcategory"),
        QObject::tr("unknown transfer type"),
        QObject::tr("possibly duplicate"),
        QObject::tr("ready to import")
    };
    // Parsed info
    QDateTime opDT;
    int idAcc, idCat, idSubcat, idCur, idUnit;
    int idAccTo; // for transfers
    int amount;
    double quantity;
    QString alias, catName, subcatName, accName, accToName, unitName, currName, descr;
    enum UnitSource {
        FromSource,
        FromAlias,
        FromDefaultValue
    } unitSource;
    ImpCandidates* children; // for receipts
    ImpRecCandidate* parent; // for receipt items
    ImpRecCandidate(const QString& _source, const QString& _uid, int _lineNumber, const QDateTime& _opDT);
    bool needAddAlias();
};

struct ImpCandidates: public QList<ImpRecCandidate>
{
    int idCurDefault, idAccDefault;
    QString accDefault, curDefault;
    GenericDatabase::DictColl collAcc, collCurr, collUnit;
    GenericDatabase::DictColl collInCat, collInAllSubCat, collExCat, collExAllSubCat;
    GenericDatabase::DictColl collInCatBySubcat, collExCatBySubcat;
    GenericDatabase::RevDictColl collInSubcatToDescr, collExSubcatToDescr;
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
    void completeDescr(ImpRecCandidate& c, const GenericDatabase::RevDictColl& collDescr, int idSubcat);
};

#endif // INTERACTIVEFORMAT_H
