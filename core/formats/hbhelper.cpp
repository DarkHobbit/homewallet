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

#include "commonexpimpdef.h"
#include "hbhelper.h"

HbHelper::HbHelper(QString* fatalError)
    : _fatalError(fatalError),
    hbMoneySum("^([0-9\\s]*[.,]?[0-9\\s]+)(\\S+)$") // 11 900,00Є
{}

bool HbHelper::isValid()
{
    if (!hbMoneySum.isValid()) {
        *_fatalError = hbMoneySum.errorString();
        return false;
    }
    else return true;
}

int HbHelper::importOneMoneyAttr(const QString &val, QString& sCur)
{
    if (!hbMoneySum.exactMatch(val)) {
        *_fatalError = QObject::tr("Money sum doesn't match: %1").arg(val);
        return 0;
    }
    QString sSum = prepareDoubleImport(hbMoneySum.cap(1));
    sCur = hbMoneySum.cap(2);
    bool ok;
    int sum = sSum.toFloat(&ok)*100;
    if (!ok) {
        *_fatalError = S_ERR_AMO_IMP.arg(sSum);
        return 0;
    }
    return sum;
}

int HbHelper::importCurrencyByChar(const QString &moneyChar, HwDatabase &db)
{
    QString _moneyChar = moneyChar;
    if (_moneyChar==QString::fromUtf8("р"))
        _moneyChar = QString::fromUtf8("₽"); // exception for old RUR symbol, appears in HB files
    int idCur = db.currencyIdByAbbr(_moneyChar);
    if (idCur==-1) {
        *_fatalError = S_ERR_CUR_NOT_FOUND.arg(_moneyChar);
        return -1;
    }
    return idCur;
}

