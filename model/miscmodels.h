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

#ifndef MISCMODELS_H
#define MISCMODELS_H

#include <QSqlQueryModel>
#include "hwdatabase.h"

class SimpleQueryModel: public QSqlQueryModel {
public:
    SimpleQueryModel(QObject* parent);
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool isValid();
    QString lastError();
protected:
    QString _error;
};

class CurrencyModel: public SimpleQueryModel {
public:
    CurrencyModel(QObject* parent, HwDatabase& db);
};

class CurrencyRateModel: public SimpleQueryModel {
public:
    CurrencyRateModel(QObject* parent, HwDatabase& db);
};

#endif // MISCMODELS_H
