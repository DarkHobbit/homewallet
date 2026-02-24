/* Home Wallet
 *
 * Module: Credit (lend and borrow) query SQL model
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QSqlError>
#include <QSqlQuery>

#include "globals.h"
#include "creditmodel.h"

CreditModel::CreditModel(QObject *parent, bool _isLend)
    : FilteredQueryModel(parent), isLend(_isLend)
{
    mainTableName = "hw_credit";
    visibleFieldNames
        << "crd.op_date" << "crd.close_date" << "crd.remind_date" << "crs.name"
        << lowUnitFunction("crd.amount", "cur.abbr")
        << lowUnitFunction("crd.down_pay", "cur.abbr")
        << lowUnitFunction("crd.money_back", "cur.abbr")
        << lowUnitFunction("crd.money_remaining_debt", "cur.abbr")
        << "cur.abbr" << "ac.name"
        << "rate" // TODO HBK shows rate_onetime in same column
        << "period" // TODO HBK shows period unit in same column
        << QString::fromUtf8("case crd.is_closed when 1 then '✔' else '' end") // U+2714 Heavy Check Mark
        << "crd.descr";
    visibleFieldTypes
        << 'D' << 'D' << 'D' << 'G'
        << 'M' << 'M' << 'M' << 'M'
        << 'G' << 'G' // cur, acc
        << 'P' << 'G' << 'G' << 'G';
    columnHeaders
        << S_COL_DATE << S_COL_CLOSE_DATE << S_COL_REMIND_DATE
        << (isLend ? S_COL_DEBTOR : S_COL_CREDITOR)
        << S_COL_SUM << S_COL_DOWN_PAY << S_COL_MONEY_BACK << S_COL_REMAINING_DEBT
        << S_COL_CURRENCY << S_COL_ACCOUNT <<  S_COL_RATE <<  S_COL_PERIOD
        << S_COL_IS_CLOSED << S_COL_DESCRIPTION;
    deleteQuery = "delete from hw_credit where id=:id";
}

void CreditModel::setDefaultVisibleColumns()
{
    visibleColumns.clear();
    visibleColumns << 0 << 1 << 3 << 4 << 5 << 9 << 10 << 12 << 13;
}

void CreditModel::update()
{
    QString sql = \
        "select crd.id, %1" \
        " from " \
        "   hw_credit crd, " \
        "   hw_account ac, hw_currency cur, hw_correspondent crs" \
        "   where crd.id_ac=ac.id" \
        "   and crd.id_cur=cur.id" \
        "   and crd.id_crs=crs.id" \
        "   %2" \
        "   and is_lend=" + QString::number(isLend)
        + " order by op_date desc;";
    updateData(sql, false);
}

QString CreditModel::localizedName()
{
    return isLend ? tr("Lend") : tr("Borrow");
}

QString CreditModel::recordLabel(const QModelIndex &recIndex)
{
    int id = QSqlQueryModel::data(index(recIndex.row(), 0), Qt::DisplayRole).toInt();
    QString label = "";
    QString sql = QString(
            "select crs.name, crd.op_date" \
            " from hw_credit crd, hw_correspondent crs" \
            " where crd.id_crs=crs.id" \
            " and crd.id=%1").arg(id);
    QSqlQuery q;
    if (q.prepare(sql)) {
        if (q.exec()) {
            q.first();
            label = QString("%1, %2")
                .arg(q.value(0).toString())
                .arg(q.value(1).toDateTime().toString(gd.dateFormat));
        }
    }
    return label;
}

QSqlQueryModel *CreditModel::createRepaymentModelForRecord(const QModelIndex &recIndex)
{
    int id = QSqlQueryModel::data(index(recIndex.row(), 0), Qt::DisplayRole).toInt();
    // TODO convert date format string from Qt to SQL notation
    QString sql = QString(
         "select strftime('%d.%m.%Y', crp.op_date), %1, %2 ac.name, crp.descr" \
         " from hw_repayment crp" \
         "   left join hw_account ac on crp.id_ac=ac.id" \
         "   left join hw_currency cur on crp.id_cur=cur.id" \
         " where id_crd=%3 order by op_date")
         .arg(FilteredQueryModel::lowUnitFunction("crp.amount", "cur.abbr"))
         .arg(gd.showSumsWithCurrency ? "" : "cur.abbr, ")
         .arg(id);
    // TODO если в lowUnitFunction отключен вывод валюты, подсовывать отдельный столбец
    QSqlQuery q;
    if (!q.prepare(sql)) {
        emit modelError(q.lastError().text());
        return 0;
    }
    q.exec();
    QSqlQueryModel* m = new QSqlQueryModel(this);
    m->setQuery(sql);
    m->setHeaderData(0, Qt::Horizontal, S_COL_DATE);
    m->setHeaderData(1, Qt::Horizontal, S_COL_SUM);
    if (gd.showSumsWithCurrency) {
        m->setHeaderData(2, Qt::Horizontal, S_COL_ACCOUNT);
        m->setHeaderData(3, Qt::Horizontal, S_COL_DESCRIPTION);
    }
    else {
        m->setHeaderData(2, Qt::Horizontal, S_COL_CURRENCY);
        m->setHeaderData(3, Qt::Horizontal, S_COL_ACCOUNT);
        m->setHeaderData(4, Qt::Horizontal, S_COL_DESCRIPTION);
    }
    return m;
}
