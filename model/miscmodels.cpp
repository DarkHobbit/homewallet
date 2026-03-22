/* Home Wallet
 *
 * Module: Miscellaneous models
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
#include <QSqlError>
#include "globals.h"
#include "miscmodels.h"

SimpleQueryModel::SimpleQueryModel(QObject *parent)
    :FilteredQueryModel(parent), _error("")
{}

bool SimpleQueryModel::isValid()
{
    return _error.isEmpty();
}

QString SimpleQueryModel::lastError()
{
    return _error;
}

void SimpleQueryModel::setDefaultVisibleColumns()
{
    visibleColumns.clear();
    for (int i=0; i<visibleFieldNames.count(); i++)
        visibleColumns << i;
}

void SimpleQueryModel::update()
{
    updateData(mainQuery, false);
}

QString SimpleQueryModel::localizedName()
{
    return "";
}

void SimpleQueryModel::reportError(const QString& msg)
{
    _error = msg;
}

#define SQL_CURR \
"select" \
"   id, seq_order, full_name, short_name, abbr, code," \
"   case is_main when 1 then '✔' else '' end as main," \
"   case is_unit when 1 then '✔' else '' end as unit," \
"   descr" \
" from hw_currency order by seq_order"

CurrencyModel::CurrencyModel(QObject *parent, HwDatabase &db)
    :SimpleQueryModel(parent)
{
    mainQuery = SQL_CURR;
    visibleFieldNames
        << "seq_order" << "full_name" << "short_name"
        << "abbr" << "code" << "main" << "unit" << "descr";
    visibleFieldTypes
        << "G" << "G" << "G" << "G" << "G" << "G" << "G" << "G";
    columnHeaders
        << S_COL_ORDER_NUM << S_COL_NAME<< S_COL_SHORT_NAME
        << QObject::tr("Abbr.") << QObject::tr("Code")
        << QObject::tr("Mn.") << S_COL_UNIT << S_COL_DESCRIPTION;
    deleteQuery = "delete from hw_currency where id=:id";
}

#define SQL_RATE_PROTO \
"select strftime('%d.%m.%Y', d.ch_date), r2.rate as rate2, r3.rate as rate3, r4.rate as rate4" \
" from" \
" hw_curr_rate_session d" \
" left join (select id_css, rate from hw_curr_rate where id_cur_unit=2) r2 on d.id=r2.id_css" \
" left join (select id_css, rate from hw_curr_rate where id_cur_unit=3) r3 on d.id=r3.id_css" \
" left join (select id_css, rate from hw_curr_rate where id_cur_rated=4) r4 on d.id=r4.id_css" \
" order by d.ch_date"

#define SQL_RATE \
    "select d.id, strftime('%d.%m.%Y', d.ch_date) as dt, %1" \
    " from" \
    " hw_curr_rate_session d" \
    " %2" \
    " order by d.ch_date"

#define SQL_MAIN_CURRENCY \
    "select abbr from hw_currency where is_main=1"

#define SQL_OTHER_CURRENCY \
    "select id, abbr, is_unit from hw_currency where (is_main<>1 || is_main is null) order by seq_order"

CurrencyRateModel::CurrencyRateModel(QObject* parent, HwDatabase &db)
    :SimpleQueryModel(parent)
{
    // Main currency
    QSqlQuery qMainC(db.sqlDbRef());
    if (!qMainC.prepare(SQL_MAIN_CURRENCY)) {
        _error = qMainC.lastError().text();
        return;
    }
    qMainC.exec();
    if (db.queryRecCount(qMainC)==0) {
        _error =  QObject::tr("Main currency not found");
        return;
    }
    QString abbrMainCurr = qMainC.value(0).toString();
    // Generate query (rated currency quantity is defined by user)
    QSqlQuery qRatedC(db.sqlDbRef());
    if (!qRatedC.prepare(SQL_OTHER_CURRENCY)) {
        _error = qRatedC.lastError().text();
        return;
    }
    if (!qRatedC.exec()) {
        _error = qRatedC.lastError().text();
        return;
    }
    if (db.queryRecCount(qRatedC)==0)
        return;
    QStringList fields, tables;
    qRatedC.first();
    int cNum = 2;
    visibleFieldNames << "dt";
    visibleFieldTypes << "D";
    columnHeaders << S_COL_DATE;
    while (qRatedC.isValid()) {
        int idRatedCurr = qRatedC.value(0).toInt();
        QString abbrRatedCurr = qRatedC.value(1).toString();
        bool isUnit = qRatedC.value(2).toBool();
        if (isUnit) {
            fields << QString("r%1.rate||'%2' as rate%3").arg(cNum).arg(abbrMainCurr).arg(cNum);
            tables << QString(
                "left join (select id_css, rate from hw_curr_rate where id_cur_unit=%1) r%2 on d.id=r%3.id_css")
                .arg(idRatedCurr).arg(cNum).arg(cNum);
            visibleFieldNames << QString("rate%1").arg(cNum);
            visibleFieldTypes << 'G';
            columnHeaders << QString("%1 (1%2=...)").arg(abbrRatedCurr).arg(abbrRatedCurr);
        }
        else {
            fields << QString("r%1.rate||'%2' as rate%3").arg(cNum).arg(abbrRatedCurr).arg(cNum);
            tables << QString(
                "left join (select id_css, rate from hw_curr_rate where id_cur_rated=%1) r%2 on d.id=r%3.id_css")
                .arg(idRatedCurr).arg(cNum).arg(cNum);
            visibleFieldNames << QString("rate%1").arg(cNum);
            visibleFieldTypes << 'G';
            columnHeaders << QString("%1 (1%2=...)").arg(abbrRatedCurr).arg(abbrMainCurr);
        }
        qRatedC.next();
        cNum++;
    }
    // Query!
    mainQuery = QString(SQL_RATE).arg(fields.join(", ")).arg(tables.join("\n "));
    deleteQuery = "delete from hw_curr_rate_session where id=:id";
}

bool CurrencyRateModel::removeById(int id)
{
    // At first, remove rate records
    QSqlQuery q;
    QString sql = "delete from hw_curr_rate where id_css=:id_css";
    if (!q.prepare(sql)) {
        _error = S_PREP_ERR.arg(q.lastError().text());
        return false;
    }
    q.bindValue(":id_css", id);
    if (!q.exec()) {
        _error = S_EXEC_ERR.arg(q.lastError().text());
        return false;
    }
    // At second, remove session record
    return SimpleQueryModel::removeById(id);
}
