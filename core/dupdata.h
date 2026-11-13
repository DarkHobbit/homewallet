/* Home Wallet
 *
 * Module: Duplicates search and collect data
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef DUPDATA_H
#define DUPDATA_H

#include <QDate>
#include <QString>
#include <QVector>

// Unified duplicate search queries
// 1) inside db, 2) db vs db 3) db vs files, 4) db vs import candidates...
#define SQL_FIND_EXPENSE_DUP \
 "select id, amount, descr, id_imp, uid_imp from hw_ex_op where op_date>=:dc and op_date<:dn" \
 " and id_esubcat=:id_subcat and amount>=:am_min and amount<=:am_max"

#define SQL_FIND_INCOME_DUP \
 "select id, amount, descr, id_imp, uid_imp from hw_in_op where op_date>=:dc and op_date<:dn" \
 " and id_isubcat=:id_subcat and amount>=:am_min and amount<=:am_max"

#define SQL_FIND_TRANSFER_DUP \
 "select id, amount, descr, id_imp, uid_imp from hw_transfer where op_date>=:dc and op_date<:dn" \
 " and id_tt=:id_subcat and amount>=:am_min and amount<=:am_max"


struct DupInfo {
    int sum;
    QString descr, src;
};
struct DupSet {
    QDate d;
    QString catName, subcatName;
    QVector<DupInfo> dups;
};
struct DupVector: public QVector<DupSet> {
    bool hasSubCat, showSrc;
    int totalCount() const;
};
struct Duplicates {
    DupVector expenses, incomes, transfer;
    QString stat() const;
};

#endif // DUPDATA_H