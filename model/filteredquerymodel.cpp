/* Home Wallet
 *
 * Module: Base class for filtered SQL models with columns tuning
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

void FilteredQueryModel::updateVisibleColumns(const ModelColumnList& _visibleColumns)
{
    beginResetModel();
    visibleColumns.clear();
    for (const short col: _visibleColumns)
        visibleColumns.push_back(col);
    endResetModel();
}

void FilteredQueryModel::setFilterDates(const QDate &_dtFrom, const QDate &_dtTo)
{
    dtFrom = _dtFrom;
    dtTo = _dtTo;
}

int FilteredQueryModel::columnCount(const QModelIndex &parent) const
{
    if (visibleColumns.isEmpty())
        return QSqlQueryModel::columnCount(parent);
    else
        return visibleColumns.count()+1;
}

QVariant FilteredQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (orientation==Qt::Horizontal)) {
        if (visibleColumns.isEmpty() || section==0)
            return QSqlQueryModel::headerData(section, orientation, role);
        else
            return columnHeaders[visibleColumns[section-1]];
    }
    else
        return QSqlQueryModel::headerData(section, orientation, role);
}

QVariant FilteredQueryModel::data(const QModelIndex &index, int role) const
{
    if (visibleColumns.isEmpty() || index.column()==0)
        return QSqlQueryModel::data(index, role);
    else {
        QModelIndex ind = this->index(index.row(), visibleColumns[index.column()-1]);
        return QSqlQueryModel::data(ind, role);
    }
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
