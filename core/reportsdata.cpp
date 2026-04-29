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

#include <iostream>
#include "globals.h"
#include "reportsdata.h"

ReportsData::ReportsData()
    :_fatalError("")
{}

#include <QTime>
bool ReportsData::findDuplicates(HwDatabase &db, const QDate &dFrom, const QDate &dTo,
    int amountDelta, bool showSrc, Duplicates &duplicates)
{
    QTime tmFind;
    tmFind.start();
    if (dFrom.isNull() || dTo.isNull() || dTo<dFrom)
        return false;
    HwDatabase::RevDictColl collCat, collSubCat, collImp;
    if (showSrc)
        DB_CHK(db.collectRevDict(collImp, "hw_imp_file", "filename"))

    // Expenses
    DB_CHK(db.collectParentRevDict(collCat, "hw_ex_cat", "hw_ex_subcat", "id_ecat"))
    DB_CHK(db.collectRevDict(collSubCat, "hw_ex_subcat"))
    if (!findOneDuplicatesKind(db, dFrom, dTo, amountDelta,
        "select id, op_date, amount, id_esubcat from hw_ex_op where op_date>=:dmin and op_date<=:dmax order by op_date",
        "select id, amount, descr, id_imp, uid_imp from hw_ex_op where op_date>=:dc and op_date<:dn" \
        " and id_esubcat=:id_subcat and amount>=:am_min and amount<=:am_max",
        collCat, collSubCat, collImp, duplicates.expenses))
        return false;

    // Incomes
    collCat.clear();
    collSubCat.clear();
    DB_CHK(db.collectParentRevDict(collCat, "hw_in_cat", "hw_in_subcat", "id_icat"))
    DB_CHK(db.collectRevDict(collSubCat, "hw_in_subcat"))
    if (!findOneDuplicatesKind(db, dFrom, dTo, amountDelta,
        "select id, op_date, amount, id_isubcat from hw_in_op where op_date>=:dmin and op_date<=:dmax order by op_date",
        "select id, amount, descr, id_imp, uid_imp from hw_in_op where op_date>=:dc and op_date<:dn" \
        " and id_isubcat=:id_subcat and amount>=:am_min and amount<=:am_max",
        collCat, collSubCat, collImp, duplicates.incomes))
        return false;

    // Transfer
    collCat.clear();
    collSubCat.clear();
    DB_CHK(db.collectRevDict(collCat, "hw_transfer_type"))
    if (!findOneDuplicatesKind(db, dFrom, dTo, amountDelta,
        "select id, op_date, amount, id_tt from hw_transfer where op_date>=:dmin and op_date<=:dmax order by op_date",
        "select id, amount, descr, id_imp, uid_imp from hw_transfer where op_date>=:dc and op_date<:dn" \
        " and id_tt=:id_subcat and amount>=:am_min and amount<=:am_max",
        collCat, collSubCat, collImp, duplicates.transfer))
        return false;

    std::cout << "Elapsed " << tmFind.elapsed() << " ms" << std::endl;
    return true;
}

bool ReportsData::findOneDuplicatesKind(HwDatabase &db, const QDate &dFrom, const QDate &dTo, int amountDelta,
    const QString &sqlOrig, const QString &sqlDups,
    const HwDatabase::RevDictColl& collCat, const HwDatabase::RevDictColl& collSubCat,
    const HwDatabase::RevDictColl& collImp, DupVector &dupVec)
{
    dupVec.clear();
    dupVec.hasSubCat = !collSubCat.isEmpty();
    dupVec.showSrc = !collImp.isEmpty();
    QVector<int> doneIds;
    QSqlQuery qOrig(db.sqlDbRef());
    DB_CHK(db.prepQuery(qOrig, sqlOrig))
    qOrig.bindValue(":dmin", dFrom);
    qOrig.bindValue(":dmax", dTo);
    DB_CHK(db.execQuery(qOrig))
    if (db.queryRecCount(qOrig)==0)
        return true;
    int cnt = 0;
    qOrig.first();
    while (qOrig.isValid()) {
        cnt++;
        // Read each record
        int idOrig = qOrig.value(0).toInt();
        if (!doneIds.contains(idOrig)) {
            int sum = qOrig.value(2).toInt();
            int sumMin = sum-amountDelta;
            int sumMax = sum+amountDelta;
            QDate d = qOrig.value(1).toDate();
            if (cnt%500==0)
                std::cerr << d.toString().toUtf8().data() << std::endl;
            // Table-specific
            int idSubCat = qOrig.value(3).toInt();
            QSqlQuery qCandDup(db.sqlDbRef());
            DB_CHK(db.prepQuery(qCandDup, sqlDups))
            qCandDup.bindValue(":id_subcat", idSubCat);
            qCandDup.bindValue(":am_min", sumMin);
            qCandDup.bindValue(":am_max", sumMax);
            qCandDup.bindValue(":dc", d);
            qCandDup.bindValue(":dn", d.addDays(1));
            DB_CHK(db.execQuery(qCandDup))
            // Search all records with same attributes
            if (db.queryRecCount(qCandDup)>1) {
                DupSet dupSet;
                dupSet.d = d;
                dupSet.catName = collCat[idSubCat];
                dupSet.subcatName = "";
                if (!collSubCat.isEmpty())
                    dupSet.subcatName = collSubCat[idSubCat];
                qCandDup.first();
                while (qCandDup.isValid()) {
                    doneIds << qCandDup.value(0).toInt();
                    DupInfo dup;
                    dup.sum = qCandDup.value(1).toInt();
                    dup.descr = qCandDup.value(2).toString();
                    if (dupVec.showSrc) {
                        int idImp = qCandDup.value(3).toInt();
                        if (idImp>0)
                            dup.src = collImp[idImp] + ":" + qCandDup.value(4).toString();
                    }
                    dupSet.dups <<dup;
                    qCandDup.next();
                }
                dupVec << dupSet;
            }
        }
        qOrig.next();
    }
    return true;
}

int ReportsData::DupVector::totalCount()
{
    int res = 0;
    for (const DupSet& ds: *this)
        res += ds.dups.count();
    return res;
}

QString ReportsData::Duplicates::stat()
{
    QString s = S_DK_EXPENSES + " %1/%2 "
              + S_DK_INCOMES  + " %3/%4 "
              + S_DK_TRANSFER + " %5/%6 "
        ;
    return s.arg(expenses.count()).arg(expenses.totalCount())
            .arg(incomes.count()).arg(incomes.totalCount())
            .arg(transfer.count()).arg(transfer.totalCount())
        ;
}
