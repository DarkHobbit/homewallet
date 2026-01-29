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
