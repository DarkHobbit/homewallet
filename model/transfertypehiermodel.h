/* Home Wallet
 *
 * Module: transfer types model
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

#ifndef TRANSFERTYPEHIERMODEL_H
#define TRANSFERTYPEHIERMODEL_H

#include <QVector>
#include <QDate>

#include "hiermodelbase.h"

class HwDatabase;

// Structure for storing transfer type/transfer data
struct TransferItem {
    int id;                 // Record ID
    int parentId;           // Parent ID (-1 for root transfer types)
    QString name;           // Display name (type name or transfer description)
    QString description;    // Description (for transfer types)
    bool isOperation;       // true - transfer (operation), false - transfer type
    double quantity;        // Quantity (for transfers, not used)
    int amount;             // Amount in low units (for transfers)
    QDate operationDate;    // Transfer date
    QVector<int> children;  // Child item indices in flat list

    TransferItem() : id(-1), parentId(-1), isOperation(false),
                     quantity(0), amount(0) {}
};

class TransferTypeHierModel : public HierModelBase
{
    Q_OBJECT

public:
    explicit TransferTypeHierModel(HwDatabase* db, QObject *parent = nullptr);
    ~TransferTypeHierModel();

    // QAbstractItemModel interface implementation

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // HierModelBase interface implementation

    // Refresh model data from database
    void refresh() override;

    // Get data by model index
    int getId(const QModelIndex &index) const override;
    QString getName(const QModelIndex &index) const override;

    // Check type of item
    bool isCategory(const QModelIndex &index) const override;     // Always false
    bool isSubcategory(const QModelIndex &index) const override; // True for transfer types
    bool isOperation(const QModelIndex &index) const override;   // True for transfers

    // Get parent IDs
    int getParentCategoryId(const QModelIndex &index) const override; // Not applicable, returns -1
    int getParentSubcategoryId(const QModelIndex &index) const override; // Returns parent type ID for transfers

    // Get operation data
    double getQuantity(const QModelIndex &index) const override;
    int getAmount(const QModelIndex &index) const override;
    QDate getOperationDate(const QModelIndex &index) const override;

    // Node operations
    bool removeAnyRows(QModelIndexList &indices) override;
    bool moveSelectedNodes(HierModelBase* opposite,
                           QModelIndexList& indices,
                           QModelIndex& oppositeIndex) override;
    bool mergeSelectedNodes(HierModelBase* opposite,
                            QModelIndex& index,
                            QModelIndex& oppositeIndex) override;

private:
    QVector<TransferItem> m_items;    // flat list of all items
    QVector<int> m_rootItems;         // indices of root (top-level) items

    void loadData();                  // load data from database
    void loadTransferTypes();         // load transfer types (level 0)
    void loadTransfers();             // load transfers (level 1)
    void buildHierarchy();            // build parent-child relationships
    QModelIndex indexFromItem(int itemIndex, int column) const;
    int findItemIndex(int id, bool isTransferType) const;
    void clearData();

    // Helper methods
    QString formatAmount(int amountInLowUnits) const;
    bool removeTransferType(int typeId);
    bool removeTransfer(int transferId);
};

#endif // TRANSFERTYPEHIERMODEL_H