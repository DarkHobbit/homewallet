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

#include <QSqlQuery>
#include "expensemodel.h"

ExpenseModel::ExpenseModel(QObject *parent)
{

}

void ExpenseModel::update()
{
    QString sql = \
        "select e.op_date, c.name, sc.name, e.quantity, u.name, e.amount, a.name, e.descr" \
        " from " \
        "   hw_ex_op e " \
        "   left join hw_unit u on e.id_un=u.id," \
        "   hw_ex_cat c, hw_ex_subcat sc, hw_account a" \
        " where e.id_esubcat=sc.id" \
        " and sc.id_ecat=c.id" \
        " and e.id_ac=a.id" \
        " %1" \
        " order by op_date desc;";
    //sql = "select * from hw_ex_op order by op_date desc;";
    updateFilter(sql, false);

    setHeaderData(0, Qt::Horizontal, tr("Date"));
    setHeaderData(1, Qt::Horizontal, tr("Category"));
}
