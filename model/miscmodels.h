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

#include "filteredquerymodel.h"
#include "hwdatabase.h"

class SimpleQueryModel: public FilteredQueryModel {
public:
    SimpleQueryModel(QObject* parent);
    bool isValid();
    QString lastError();
    virtual void setDefaultVisibleColumns();
    virtual void update();
    virtual QString localizedName();
protected:
    QString mainQuery;
    QString _error;
    virtual void reportError(const QString& msg);
};

class CurrencyModel: public SimpleQueryModel {
public:
    CurrencyModel(QObject* parent, HwDatabase& db);
};

class CurrencyRateModel: public SimpleQueryModel {
public:
    CurrencyRateModel(QObject* parent, HwDatabase& db);
protected:
    virtual bool removeById(int id);
};

#endif // MISCMODELS_H
