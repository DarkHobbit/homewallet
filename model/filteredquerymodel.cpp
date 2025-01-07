/* Home Wallet
 *
 * Module: Base class for filtered SQL models
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
#include "filteredquerymodel.h"

FilteredQueryModel::FilteredQueryModel(QObject *parent)
    : QSqlQueryModel(parent),
      dtFrom(QDate()), dtTo(QDate())
{
}

void FilteredQueryModel::setFilterDates(const QDate &_dtFrom, const QDate &_dtTo)
{
    dtFrom = _dtFrom;
    dtTo = _dtTo;
}

void FilteredQueryModel::updateFilter(const QString &sql, bool insertWhere)
{
    QString fAdd = "";
    makeFilter();
    if (!filters.isEmpty()) {
        fAdd = insertWhere ? "where " : "and ";
        fAdd += filters.join(" and ");
    }
    QSqlQuery q(sql.arg(fAdd));
    setQuery(q);
    while (canFetchMore())
        fetchMore();
}

void FilteredQueryModel::makeFilter()
{
    // child classes can add field
    filters.clear();
    if (dtFrom.isValid())
        filters <<
            QString("strftime('%Y%m%d', op_date)>='")+dtFrom.toString("yyyyMMdd")+"'";
    if (dtTo.isValid())
        filters <<
            QString("strftime('%Y%m%d', op_date)<='")+dtTo.toString("yyyyMMdd")+"'";
}
