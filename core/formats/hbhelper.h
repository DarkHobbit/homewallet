/* Home wallet
 *
 * Module: Home Bookkeeping (keepsoft.ru) helper tools
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef HBHELPER_H
#define HBHELPER_H

#include <QRegExp>
#include <QString>
#include "hwdatabase.h"

class HbHelper
{
public:
    HbHelper(QString* fatalError);
    bool isValid();
    int importOneMoneyAttr(const QString& val, QString& sCur);
    int importCurrencyByChar(const QString& moneyChar, HwDatabase& db);
private:
    QString* _fatalError;
    const QRegExp hbMoneySum;
};

#endif // HBHELPER_H
