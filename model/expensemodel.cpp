/* Home Wallet
 *
 * Module: Expenses query SQL model
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
#include "expensemodel.h"

ExpenseModel::ExpenseModel(QObject *parent)
    : CategoriesBasedQueryModel(parent)
{
    visibleFieldNames
        << "strftime('%d.%m.%Y', e.op_date)" // SQLite-ism!
        << "c.name" << "sc.name" << "e.quantity" << "u.name"
        << lowUnitFunction("e.amount")
        << "cur.abbr" << "a.name"
        << "case e.attention when 1 then '*' else '' end"
        << "e.descr";
    columnHeaders
        << S_COL_DATE
        << S_COL_CATEGORY << S_COL_SUBCATEGORY << S_COL_QUANTITY <<S_COL_UNIT
        << S_COL_SUM
        << S_COL_CURRENCY << S_COL_ACCOUNT
        << S_COL_ATTENTION << S_COL_DESCRIPTION;
    // visibleColumns << 2 << 3 << 0; // demo
}

void ExpenseModel::update()
{
    QString sql = \
        "select e.id, %1" \
        " from " \
        "   hw_ex_op e " \
        "   left join hw_unit u on e.id_un=u.id," \
        "   hw_ex_cat c, hw_ex_subcat sc, hw_account a, hw_currency cur" \
        " where e.id_esubcat=sc.id" \
        " and sc.id_ecat=c.id" \
        " and e.id_ac=a.id" \
        " and e.id_cur=cur.id" \
        " %2" \
        " order by op_date desc;";
    updateData(sql, false);
}

