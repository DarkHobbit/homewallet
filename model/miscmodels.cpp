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
#include <iostream>

#include "globals.h"
#include "miscmodels.h"

#define SQL_RATE_PROTO \
"select d.ch_date, r2.rate as rate2, r3.rate as rate3, r4.rate as rate4" \
" from" \
" hw_curr_rate_session d" \
" left join (select id_css, rate from hw_curr_rate where id_cur_unit=2) r2 on d.id=r2.id_css" \
" left join (select id_css, rate from hw_curr_rate where id_cur_unit=3) r3 on d.id=r3.id_css" \
" left join (select id_css, rate from hw_curr_rate where id_cur_rated=4) r4 on d.id=r4.id_css" \
" order by d.ch_date"

CurrencyRateModel::CurrencyRateModel(QObject* parent, HwDatabase &db)
    :QSqlQueryModel(parent)
{
    QSqlQuery q(db.sqlDbRef());
    bool res = q.prepare(SQL_RATE_PROTO);
    std::cerr << "crm " << res << std::endl;
    std::cerr << q.lastError().text().toUtf8().data() << std::endl;
    res = q.exec();
    std::cerr << "crm2 " << res << std::endl;
    setQuery(q);
    /*
    if (q.lastError().type()==QSqlError::NoError) {
        while (canFetchMore())
            fetchMore();
    }
*/
    setHeaderData(0, Qt::Horizontal, S_COL_DATE);
}
