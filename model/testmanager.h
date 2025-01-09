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

#ifndef TESTMANAGER_H
#define TESTMANAGER_H

#include <QSqlQueryModel>
#include <QString>

#include "genericdatabase.h"
#include "hwdatabase.h"

class TestManager
{
public:
    TestManager();
    static QSqlQueryModel* dbDebug(const QString& queryText, GenericDatabase& db);
    static bool createTestData(HwDatabase& db);
};

#endif // TESTMANAGER_H
