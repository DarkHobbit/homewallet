/* Home Wallet
 *
 * Module: Test utilities
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
#include "testmanager.h"

TestManager::TestManager()
{

}

QSqlQueryModel *TestManager::dbDebug(const QString &queryText, GenericDatabase &db)
{
    QSqlQuery q;
    q.prepare(queryText);
    q.exec();
    QSqlQueryModel* m = new QSqlQueryModel();
    m->setQuery(q);
    while (m->canFetchMore())
        m->fetchMore();
    return m;
}
