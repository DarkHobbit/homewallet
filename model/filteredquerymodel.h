/* Home Wallet
 *
 * Module: Base class for filtered SQL modeld
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
#ifndef FILTEREDQUERYMODEL_H
#define FILTEREDQUERYMODEL_H

#include <QDate>
#include <QSqlQueryModel>
#include <QStringList>

class FilteredQueryModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit FilteredQueryModel(QObject *parent = nullptr);
    void setDates(const QDate& _dtFrom=QDate(), const QDate& _dtTo=QDate());
protected:
    QStringList filters;
    void updateFilter(const QString& sql, bool insertWhere);
private:
    QDate dtFrom, dtTo;
    virtual void makeFilter();
};

#endif // FILTEREDQUERYMODEL_H
