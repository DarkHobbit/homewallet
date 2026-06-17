/* Home Wallet
 *
 * Module: Base class for hierarchical "dictionary+operations" models
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 * This class is written using DeepSeek chat
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef HIERMODELBASE_H
#define HIERMODELBASE_H

#include <QAbstractItemModel>
#include <QString>

class HwDatabase;

class HierModelBase : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit HierModelBase(HwDatabase* db, QObject *parent = nullptr);

    // --- Pure virtual methods ---

    // Refresh model data from database
    virtual void refresh() = 0;

    // Get data by model index
    virtual int getId(const QModelIndex &index) const = 0;
    virtual QString getName(const QModelIndex &index) const = 0;

    // Check type of item
    virtual bool isCategory(const QModelIndex &index) const = 0;
    virtual bool isSubcategory(const QModelIndex &index) const = 0;
    virtual bool isOperation(const QModelIndex &index) const = 0;
    virtual bool isExpense() const = 0;

    // Get parent IDs
    virtual int getParentCategoryId(const QModelIndex &index) const = 0;
    virtual int getParentSubcategoryId(const QModelIndex &index) const = 0;

    // Get operation data
    virtual double getQuantity(const QModelIndex &index) const = 0;
    virtual int getAmount(const QModelIndex &index) const = 0;
    virtual QDate getOperationDate(const QModelIndex &index) const = 0;

    // Node operations
    virtual bool removeAnyRows(QModelIndexList &indices) = 0;
    virtual bool mergeSelectedNodes(HierModelBase* opposite,
        QModelIndex& index, QModelIndex& oppositeIndex) = 0;

    // --- Implemented methods ---

    // Enable/disable operation display (third level)
    void setOperationShow(bool show);
    bool isOperationShow() const { return m_showOperations; }
    QString lastError() const { return m_lastError; }

signals:
    void infoForUser(const QString& message);

protected:
    HwDatabase* m_db;           // pointer to database object (not owner)
    bool m_showOperations;      // show third level (operations)
    QString m_lastError;
};

#endif // HIERMODELBASE_H
