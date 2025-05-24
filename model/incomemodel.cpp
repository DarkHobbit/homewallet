/* Home Wallet
 *
 * Module: Incomes query SQL model
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include "globals.h"
#include "incomemodel.h"

IncomeModel::IncomeModel(QObject *parent)
    : CategoriesBasedQueryModel{parent}
{
    visibleFieldNames
        << "strftime('%d.%m.%Y', i.op_date)" // SQLite-ism!
        << "c.name" << "sc.name" << "i.quantity" << "u.name"
        << lowUnitFunction("i.amount", "cur.abbr")
        << "cur.abbr" << "a.name"
        << "case i.attention when 1 then '*' else '' end"
        << "i.descr";
    columnHeaders
        << S_COL_DATE
        << S_COL_CATEGORY << S_COL_SUBCATEGORY << S_COL_QUANTITY <<S_COL_UNIT
        << S_COL_SUM
        << S_COL_CURRENCY << S_COL_ACCOUNT
        << S_COL_ATTENTION << S_COL_DESCRIPTION;
    // TODO read from settings
    visibleColumns << 0 << 1 << 2 << 3 << 4 << 5 << 7 << 8 << 9;
}

void IncomeModel::update()
{
    QString sql = \
        "select i.id, %1" \
        " from " \
        "   hw_in_op i " \
        "   left join hw_unit u on i.id_un=u.id," \
        "   hw_in_cat c, hw_in_subcat sc, hw_account a, hw_currency cur" \
        " where i.id_isubcat=sc.id" \
        " and sc.id_icat=c.id" \
        " and i.id_ac=a.id" \
        " and i.id_cur=cur.id" \
        " %2" \
        " order by op_date desc;";
    updateData(sql, false);
}
