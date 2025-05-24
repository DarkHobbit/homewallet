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
#ifndef FILTEREDQUERYMODEL_H
#define FILTEREDQUERYMODEL_H

#include <QDate>
#include <QSqlQueryModel>
#include <QStringList>

/* This model assume that all tables have integer one-field primary key (id);
 * first column contains this id and is invisible!
 */

class ModelColumnList: public QList<short>
{};

class FilteredQueryModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit FilteredQueryModel(QObject *parent = nullptr);
    virtual void update() = 0;
    void updateVisibleColumns(const ModelColumnList& _visibleColumns);
    void setFilterDates(const QDate& _dtFrom=QDate(), const QDate& _dtTo=QDate());
    // Base model implementation methods
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    // QVariant data(const QModelIndex &index, int role) const;
    // TODO flags(), if needed
    // TODO field list for Qt::AlignRight (number)
protected:
    ModelColumnList visibleColumns;
    QStringList visibleFieldNames;
    QStringList columnHeaders;
    QStringList filters;
    QDate dtFrom, dtTo;
    virtual void makeFilter();
    void updateData(const QString& sql, bool insertWhere);
    QString lowUnitFunction(const QString& fieldName, const QString& currFieldName = "");
signals:
    void modelError(const QString& message);
};

#endif // FILTEREDQUERYMODEL_H
