/* Home Wallet
 *
 * Module: Reports data creators and structure
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef REPORTSDATA_H
#define REPORTSDATA_H

#include <QDate>
#include <QString>
#include <QVector>

#include "hwdatabase.h"

#define DB_CHK(action) \
{ \
        if (!(action)) \
    { \
            _fatalError = db.lastError(); \
            return false; \
    } \
}

class ReportsData
{
public:
    struct DupInfo {
        int sum;
        QString descr, src;
    };
    struct DupSet {
        QDate d;
        QString catName, subcatName;
        QVector<DupInfo> dups;
    };
    struct DupVector: public QVector<DupSet> {
        bool hasSubCat, showSrc;
        int totalCount();
    };
    struct Duplicates {
        DupVector expenses, incomes, transfer;
        QString stat();
    };

    ReportsData();
    bool findDuplicates(HwDatabase& db, const QDate& dFrom, const QDate& dTo,
        int amountDelta, bool showSrc, Duplicates& duplicates);
    QString fatalError();
protected:
    QString _fatalError;
    bool findOneDuplicatesKind(HwDatabase& db, const QDate& dFrom, const QDate& dTo, int amountDelta,
        const QString& sqlOrig, const QString& sqlDups,
        const HwDatabase::RevDictColl& collCat, const HwDatabase::RevDictColl& collSubCat,
        const HwDatabase::RevDictColl& collImp, DupVector& dupVec);
};

inline QString ReportsData::fatalError()
{
    return _fatalError;
}

#endif // REPORTSDATA_H
