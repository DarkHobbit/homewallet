/* Home Wallet
 *
 * Module: Currency conversion (exchange) query SQL model
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
#include "currconvmodel.h"

CurrConvModel::CurrConvModel(QObject *parent)
    : FilteredQueryModel(parent)
{
    visibleFieldNames
        << "e.op_date" << "ac.name"
        << lowUnitFunction("e.amount_out", "curo.abbr") << "curo.abbr"
        << lowUnitFunction("e.amount_in", "curi.abbr") << "curi.abbr"
        << "e.descr";
    visibleFieldTypes
        << 'D' << 'G' << 'M' << 'G' << 'M' << 'G' << 'G';
    columnHeaders
        << S_COL_DATE << S_COL_ACCOUNT
        << S_COL_SUM+' '+S_COL_FROM << S_COL_FROM
        << S_COL_SUM+' '+S_COL_TO << S_COL_TO
        << S_COL_DESCRIPTION;
    // TODO read from settings
    visibleColumns << 0 << 1 << 2 << 3 << 4 << 5 << 6;
}

void CurrConvModel::update()
{
    QString sql = \
        "select e.id, %1" \
        " from " \
    " hw_curr_exch e, " \
    " hw_account ac, hw_currency curi, hw_currency curo" \
    " where e.id_ac=ac.id" \
    " and e.id_cur_in=curi.id" \
    " and e.id_cur_out=curo.id" \
        "   %2" \
        " order by op_date desc;";
    updateData(sql, false);
}
