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

#include <QSqlError>
#include <QSqlQuery>

#include "filteredquerymodel.h"
#include "globals.h"

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
        if (section>columnHeaders.count() || section==0) // fallback
            return QSqlQueryModel::headerData(section, orientation, role);
        else if (visibleColumns.isEmpty()) // first time
            return columnHeaders[section-1];
        else // main case
            return columnHeaders[visibleColumns[section-1]];
    }
    else
        return QSqlQueryModel::headerData(section, orientation, role);
}

/* TODO Restore if need custom formatting
QVariant FilteredQueryModel::data(const QModelIndex &index, int role) const
{
    if (visibleColumns.isEmpty() || index.column()==0)
        return QSqlQueryModel::data(index, role);
    else {
        QModelIndex ind = this->index(index.row(), visibleColumns[index.column()-1]);
        return QSqlQueryModel::data(ind, role);
    }
}
*/

void FilteredQueryModel::updateData(const QString &sql, bool insertWhere)
{
    // Fields
    QString fields;
    if (visibleColumns.isEmpty())
        fields = visibleFieldNames.join(", ");
    else {
        fields = visibleFieldNames[visibleColumns.first()];
        for (int i=1; i<visibleColumns.count(); i++) {
            int fieldIndex = visibleColumns[i];
            if (fieldIndex>=visibleFieldNames.count()) {
                fieldIndex = 0; // failback if incorrect settings
                emit modelError(tr("Incorrect column index found, see Settings->Columns"));
            }
            fields += ", " + visibleFieldNames[fieldIndex];
        }
    }
    // Filter
    QString fAdd = "";
    makeFilter();
    if (!filters.isEmpty()) {
        fAdd = insertWhere ? "where " : "and ";
        fAdd += filters.join(" and ");
    }
    // All together
    QSqlQuery q(sql.arg(fields).arg(fAdd));
    setQuery(q);
    QString qe = q.lastError().text();
    if (qe.isEmpty()) {
        while (canFetchMore())
            fetchMore();
    }
    else
        emit modelError(qe);
}

QString FilteredQueryModel::lowUnitFunction(const QString& fieldName, const QString& currFieldName)
{
    QString res;
    if (0) // SQLite 3.38+ and other databases
        res = QString("format('%.2f', ") + fieldName + "/100.00)";
    else // old SQLite
        res = QString("printf('%.2f', ") + fieldName + "/100.00)";
    // TODO m.b. use QLocale::toCurrencyString() on client side in model...
    if (!currFieldName.isEmpty() && gd.showSumsWithCurrency)
        res += "||" + currFieldName;
    return res;
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
