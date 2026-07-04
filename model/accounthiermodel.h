/* Home Wallet
 *
 * Module: Model for accounts and account operations
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

#ifndef ACCOUNTHIERMODEL_H
#define ACCOUNTHIERMODEL_H

#include <QVector>
#include <QDate>

#include "hiermodelbase.h"

class HwDatabase;

// Structure for storing account/operation data
struct AccountItem {
    int id;                 // Record ID
    int parentId;           // Parent ID (-1 for root accounts)
    QString name;           // Display name
    QString description;    // Description (for accounts)
    bool isOperation;       // true - operation, false - account
    double quantity;        // Quantity (for operations)
    int amount;             // Amount in low units (for operations)
    QDate operationDate;    // Operation date
    int operationType;      // Type of operation (for display)
    QString operationDetails; // Additional details
    QVector<int> children;  // Child item indices in flat list

    AccountItem() : id(-1), parentId(-1), isOperation(false),
                    quantity(0), amount(0), operationType(0) {}
};

class AccountHierModel : public HierModelBase
{
    Q_OBJECT

public:
    explicit AccountHierModel(HwDatabase* db, QObject *parent = nullptr);
    ~AccountHierModel();

    // QAbstractItemModel interface implementation

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // HierModelBase interface implementation

    void refresh() override;
    int getId(const QModelIndex &index) const override;
    QString getName(const QModelIndex &index) const override;

    bool isCategory(const QModelIndex &index) const override;     // Always false
    bool isSubcategory(const QModelIndex &index) const override; // True for accounts
    bool isOperation(const QModelIndex &index) const override;   // True for operations

    int getParentCategoryId(const QModelIndex &index) const override; // Not applicable, returns -1
    int getParentSubcategoryId(const QModelIndex &index) const override; // Returns account ID for operations

    double getQuantity(const QModelIndex &index) const override;
    int getAmount(const QModelIndex &index) const override;
    QDate getOperationDate(const QModelIndex &index) const override;

    bool removeAnyRows(QModelIndexList &indices) override;
    bool moveSelectedNodes(HierModelBase* opposite,
                           QModelIndexList& indices,
                           QModelIndex& oppositeIndex) override;
    bool mergeSelectedNodes(HierModelBase* opposite,
                            QModelIndex& index,
                            QModelIndex& oppositeIndex) override;

private:
    QVector<AccountItem> m_items;    // flat list of all items
    QVector<int> m_rootItems;        // indices of root (top-level) items

    void loadData();                 // load data from database
    void loadAccounts();             // load accounts (level 0)
    void loadOperations();           // load all operations (level 1)
    void buildHierarchy();           // build parent-child relationships
    QModelIndex indexFromItem(int itemIndex, int column) const;
    int findItemIndex(int id) const;
    void clearData();

    // Helper methods
    QString formatAmount(int amountInLowUnits) const;
    QString getOperationTypeName(int type) const;
};

#endif // ACCOUNTHIERMODEL_H