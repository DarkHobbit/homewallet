/* Home Wallet
 *
 * Module: Inter-account transfer query SQL model
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
#include "transfermodel.h"

TransferModel::TransferModel(QObject *parent)
    : CategoriesBasedQueryModel(parent)
{
    visibleFieldNames
        << "t.op_date"
          << lowUnitFunction("t.amount", "cur.abbr")
          << "cur.abbr" << "ai.name" << "ao.name"
          << "c.name" << "t.descr";
    visibleFieldTypes
              << 'D' << 'M' << 'G' << 'G' << 'G' << 'G' << 'G';
    columnHeaders
        << S_COL_DATE << S_COL_SUM << S_COL_CURRENCY
        << S_COL_FROM << S_COL_TO
        << S_COL_CATEGORY << S_COL_DESCRIPTION;
    deleteQuery = "delete from hw_transfer where id=:id";
}

void TransferModel::setDefaultVisibleColumns()
{
    visibleColumns.clear();
    visibleColumns << 0 << 1 << 3 << 4 << 5 << 6;
}

void TransferModel::update()
{
    QString sql = \
        "select t.id, %1" \
        " from " \
        "   hw_transfer t, " \
        "   hw_account ai, hw_account ao, hw_currency cur, hw_transfer_type c" \
        "   where t.id_ac_in=ai.id" \
        "   and t.id_ac_out=ao.id" \
        "   and t.id_cur=cur.id" \
        "   and t.id_tt=c.id" \
        "   %2" \
        " order by op_date desc;";
    updateData(sql, false);
}

QString TransferModel::localizedName()
{
    return tr("Transfer");
}
