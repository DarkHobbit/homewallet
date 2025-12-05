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

#include <algorithm>
#include <iostream>
#include <sqlite3.h>

#include <QSqlError>
#include <QSqlQuery>

#include "filteredquerymodel.h"
#include "globals.h"

FilteredQueryModel::FilteredQueryModel(QObject *parent)
    : QSqlQueryModel(parent),
      deleteQuery(""),
      dtFrom(QDate()), dtTo(QDate())
{
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
        else { // main case
            if (section>visibleColumns.count())
                return "Err1";
            else {
                int visIndex = visibleColumns[section-1];
                if (visIndex>=columnHeaders.count())
                    return "Err2";
                else
                    return columnHeaders[visIndex];
            }
        }
    }
    else
        return QSqlQueryModel::headerData(section, orientation, role);
}

QVariant FilteredQueryModel::data(const QModelIndex &index, int role) const
{
    int col = index.column();
    if (visibleColumns.isEmpty() || col==0 || col > visibleColumns.count())
        return QSqlQueryModel::data(index, role);
    QVariant res = QSqlQueryModel::data(index, Qt::DisplayRole);
    int visIndex = visibleColumns[col-1];
    char fmt = (visIndex<visibleFieldTypes.count()) ? visibleFieldTypes[visIndex] : 'G';
    if (role==Qt::DisplayRole) {
        switch (fmt) {
        case 'D': {
            QDateTime opDT = res.toDateTime();
            return gd.useSystemDateTimeFormat ? opDT.toString() : opDT.toString(gd.dateFormat);
        }
        case 'M':
            return res.toString().replace('.', ',');
        case 'P': {
            if (res.isNull())
                return res;
            else
                return res.toString() + "%";
        }
        default:
            return res;
        }
    }
    else if (role==SortStringRole) {
        switch (fmt) {
        case 'D': {
            QDateTime opDT = res.toDateTime();
            return opDT.toString(Qt::ISODate);
        }
        case 'M': {
            QString sRes = res.toString();
            if (!sRes[sRes.length()-1].isDigit())
                sRes.remove(sRes.length()-1, 1);
            return sRes.toDouble();
        }
        default:
            return res;
        }
    }
    else return QSqlQueryModel::data(index, role);
}

void FilteredQueryModel::setVisibleColumns(const QStringList &names)
{
    if (names.isEmpty())
        return;
    visibleColumns.clear();
    for (const QString& colName: names) {
        if (columnHeaders.contains(colName))
                visibleColumns << columnHeaders.indexOf(colName);
    }
}

QStringList FilteredQueryModel::getVisibleColumns()
{
    QStringList res;
    for (short index: visibleColumns) {
        if (index<columnHeaders.count())
            res << columnHeaders[index];
        else
            std::cerr << "Internal error: index=" << index << ", size=" << columnHeaders.count() <<std::endl;
    }
    return res;
}

QStringList FilteredQueryModel::getAllColumns()
{
    return columnHeaders;
}

bool FilteredQueryModel::removeAnyRows(QModelIndexList &indices)
{
    std::sort(indices.begin(), indices.end());
    // foreach not usable here - reverse order needed
    beginRemoveRows (QModelIndex(), 0, indices.count()-1);
    for (int i=indices.count()-1; i>=0; i--) {
        int id = QSqlQueryModel::data(index(indices[i].row(), 0), Qt::DisplayRole).toInt();
        if (!removeById(id))
            return false;
    }
    endRemoveRows();
    return true;
}

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
    if (q.lastError().type()==QSqlError::NoError) {
        while (canFetchMore())
            fetchMore();
    }
    else
        emit modelError(q.lastError().text());
}

QString FilteredQueryModel::lowUnitFunction(const QString& fieldName, const QString& currFieldName)
{
    QString res;
    QString drvName = QSqlDatabase::database().driverName();
    if (drvName=="QSQLITE") {
        int dbmsVer = sqlite3_libversion_number();
        if (dbmsVer>=3038000) // SQLite 3.38+
            res = QString("format('%.2f', ") + fieldName + "/100.00)";
        else if (dbmsVer>=3008003) // SQLite 3.8.3+
            res = QString("printf('%.2f', ") + fieldName + "/100.00)";
        else // very old SQLite (e.g. 3.7.17 from Debian Wheezy)
            // TODO very slow on 40000 records; m.b. use QLocale::toCurrencyString() on client side in model...
            res = QString("substr(cast(") + fieldName + " as text), 1, length(cast(" + fieldName + " as text))-2)||'.'"
                + "||substr(cast(" + fieldName + " as text), length(cast(" + fieldName + " as text))-1, 2)";
    }
    else // other DBMSes, e.g. PostgreSQL
        res = QString("format('%.2f', ") + fieldName + "/100.00)";
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

// Default implementation
// For some models cascade remove or additional check required
// TODO for expenses, check receipts, already check splits, etc
bool FilteredQueryModel::removeById(int id)
{
    if (deleteQuery.isEmpty()) {
        emit modelError(tr("Can't remove records from this table"));
        return false;
    }
    QSqlQuery q;
    if (!q.prepare(deleteQuery)) {
        emit modelError(QString("Deletion prepare error: ")+q.lastError().text());
        return false;
    }
    q.bindValue(":id", id);
    if (!q.exec()) {
        emit modelError(QString("Deletion execute error: ")+q.lastError().text());
        return false;
    }
    return true;
}
