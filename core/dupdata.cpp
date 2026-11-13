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

#include "dupdata.h"
#include "globals.h"

int DupVector::totalCount() const
{
    int res = 0;
    for (const DupSet& ds: *this)
        res += ds.dups.count();
    return res;
}

QString Duplicates::stat() const
{
    QString s = S_DK_EXPENSES + " %1/%2 "
                + S_DK_INCOMES  + " %3/%4 "
                + S_DK_TRANSFER + " %5/%6 "
        ;
    return s.arg(expenses.count()).arg(expenses.totalCount())
        .arg(incomes.count()).arg(incomes.totalCount())
        .arg(transfer.count()).arg(transfer.totalCount())
        ;
}
