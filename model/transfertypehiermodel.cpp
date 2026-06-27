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

#include "transfertypehiermodel.h"
#include "globals.h"
#include "hwdatabase.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QLocale>

TransferTypeHierModel::TransferTypeHierModel(HwDatabase* db, QObject *parent)
    : HierModelBase(db, parent)
{
    loadData();
}

TransferTypeHierModel::~TransferTypeHierModel()
{
}

void TransferTypeHierModel::clearData()
{
    m_items.clear();
    m_rootItems.clear();
}

void TransferTypeHierModel::loadData()
{
    beginResetModel();
    clearData();

    loadTransferTypes();

    if (m_showOperations) {
        loadTransfers();
    }

    buildHierarchy();

    endResetModel();
}

void TransferTypeHierModel::loadTransferTypes()
{
    QSqlQuery query("SELECT id, name, descr FROM hw_transfer_type ORDER BY name");

    while (query.next()) {
        TransferItem item;
        item.id = query.value(0).toInt();
        item.parentId = -1;
        item.name = query.value(1).toString();
        item.description = query.value(2).toString();
        item.isOperation = false;

        m_items.append(item);
    }
}

void TransferTypeHierModel::loadTransfers()
{
    QSqlQuery query("SELECT id, id_tt, op_date, amount, descr "
                    "FROM hw_transfer ORDER BY op_date DESC");

    while (query.next()) {
        TransferItem item;
        item.id = query.value(0).toInt();
        item.parentId = query.value(1).toInt();  // Transfer type ID
        item.name = query.value(4).toString();   // Transfer description
        if (item.name.isEmpty()) {
            item.name = tr("Transfer #%1").arg(item.id);
        }
        item.operationDate = query.value(2).toDate();
        item.amount = query.value(3).toInt();
        item.isOperation = true;

        m_items.append(item);
    }
}

void TransferTypeHierModel::buildHierarchy()
{
    // Clear existing children lists
    for (int i = 0; i < m_items.size(); ++i) {
        m_items[i].children.clear();
    }

    // Build children lists
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].parentId == -1) {
            // Root item (transfer type)
            m_rootItems.append(i);
        } else {
            // Find parent transfer type index
            int parentIndex = findItemIndex(m_items[i].parentId, true);
            if (parentIndex != -1) {
                m_items[parentIndex].children.append(i);
            }
        }
    }
}

int TransferTypeHierModel::findItemIndex(int id, bool isTransferType) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            if (isTransferType && !m_items[i].isOperation) {
                return i;
            } else if (!isTransferType && m_items[i].isOperation) {
                return i;
            }
        }
    }
    return -1;
}

void TransferTypeHierModel::refresh()
{
    loadData();
}

// QAbstractItemModel implementation

QModelIndex TransferTypeHierModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        // Top-level item (transfer type)
        if (row < 0 || row >= m_rootItems.size())
            return QModelIndex();
        return indexFromItem(m_rootItems[row], column);
    }

    // Child item (transfer)
    int parentIndex = parent.internalId();
    if (parentIndex < 0 || parentIndex >= m_items.size())
        return QModelIndex();

    if (row < 0 || row >= m_items[parentIndex].children.size())
        return QModelIndex();

    int childIndex = m_items[parentIndex].children[row];
    return indexFromItem(childIndex, column);
}

QModelIndex TransferTypeHierModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    int childIndex = child.internalId();
    if (childIndex < 0 || childIndex >= m_items.size())
        return QModelIndex();

    int parentId = m_items[childIndex].parentId;
    if (parentId == -1)
        return QModelIndex();  // Top-level item has no parent

    // Find parent item index
    int parentIndex = findItemIndex(parentId, true);
    if (parentIndex == -1)
        return QModelIndex();

    // Find parent's row among root items
    int row = m_rootItems.indexOf(parentIndex);
    if (row == -1)
        return QModelIndex();

    return createIndex(row, 0, parentIndex);
}

int TransferTypeHierModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_rootItems.size();
    }

    int parentIndex = parent.internalId();
    if (parentIndex < 0 || parentIndex >= m_items.size())
        return 0;

    return m_items[parentIndex].children.size();
}

int TransferTypeHierModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;  // Two columns: Name and Amount (for transfers)
}

QVariant TransferTypeHierModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QVariant();

    const TransferItem &item = m_items[itemIndex];
    int column = index.column();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (column == 0) {
            // First column: name
            if (item.isOperation) {
                return QString("%1 - %2")
                .arg(item.operationDate.toString("dd.MM.yyyy"))
                    .arg(item.name);
            }
            return item.name;
        } else if (column == 1) {
            // Second column: child count
            if (!item.isOperation) {
                // Transfer type - show number of transfers
                int count = item.children.size();
                if (count > 0) {
                    return QString::number(count);
                }
                return QVariant();  // Empty for types without transfers
            }
            // For transfers - empty
            return QVariant();
        }
        break;

    case Qt::ToolTipRole:
        if (column == 0) {
            if (item.isOperation) {
                return QString("%1\nAmount: %2")
                .arg(item.name)
                    .arg(formatAmount(item.amount));
            }
            if (!item.description.isEmpty())
                return item.description;
            return item.name;
        } else if (column == 1) {
            if (!item.isOperation) {
                int count = item.children.size();
                if (count > 0) {
                    return tr("Transfers: %1").arg(count);
                }
            }
        }
        break;

    case Qt::TextAlignmentRole:
        if (column == 1) {
            return Qt::AlignCenter;
        }
        break;

    case Qt::UserRole + 1: // ID
        if (column == 0) {
            return item.id;
        }
        break;

    case Qt::UserRole + 2: // Type (0=category,1=subcategory,2=operation)
        if (column == 0) {
            if (!item.isOperation)
                return 1;  // Transfer type (like subcategory)
            else
                return 2;  // Transfer (like operation)
        }
        break;

    default:
        return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags TransferTypeHierModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TransferTypeHierModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return S_COL_CATEGORY;
        } else if (section == 1) {
            return S_COL_REC_NUM;
        }
    }
    return QVariant();
}

// HierModelBase interface implementation

int TransferTypeHierModel::getId(const QModelIndex &index) const
{
    if (!index.isValid())
        return -1;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return -1;

    return m_items[itemIndex].id;
}

QString TransferTypeHierModel::getName(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QString();

    return m_items[itemIndex].name;
}

bool TransferTypeHierModel::isCategory(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return false;  // No categories in this model
}

bool TransferTypeHierModel::isSubcategory(const QModelIndex &index) const
{
    if (!index.isValid())
        return false;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return false;

    return !m_items[itemIndex].isOperation;  // Transfer types are like subcategories
}

bool TransferTypeHierModel::isOperation(const QModelIndex &index) const
{
    if (!index.isValid())
        return false;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return false;

    return m_items[itemIndex].isOperation;
}

int TransferTypeHierModel::getParentCategoryId(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return -1;  // Not applicable
}

int TransferTypeHierModel::getParentSubcategoryId(const QModelIndex &index) const
{
    if (!index.isValid())
        return -1;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return -1;

    const TransferItem &item = m_items[itemIndex];

    if (!item.isOperation)
        return -1;

    return item.parentId;  // Returns transfer type ID for transfers
}

double TransferTypeHierModel::getQuantity(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return 0.0;  // Not applicable for transfers
}

int TransferTypeHierModel::getAmount(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return 0;

    return m_items[itemIndex].amount;
}

QDate TransferTypeHierModel::getOperationDate(const QModelIndex &index) const
{
    if (!index.isValid())
        return QDate();

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QDate();

    return m_items[itemIndex].operationDate;
}

bool TransferTypeHierModel::removeAnyRows(QModelIndexList &indices)
{
    if (indices.isEmpty())
        return true;

    // Remove from bottom to top to avoid index shifting issues
    std::sort(indices.begin(), indices.end(),
              [](const QModelIndex &a, const QModelIndex &b) {
                  return a.row() > b.row();
              });

    bool success = true;
    for (const QModelIndex &index : indices) {
        if (!index.isValid())
            continue;

        if (isSubcategory(index)) {
            // Remove transfer type (and all its transfers)
            if (!removeTransferType(getId(index))) {
                success = false;
            }
        } else if (isOperation(index)) {
            // Remove single transfer
            if (!removeTransfer(getId(index))) {
                success = false;
            }
        }
    }

    if (success) {
        refresh();
    }

    return success;
}

bool TransferTypeHierModel::moveSelectedNodes(HierModelBase* opposite,
                                              QModelIndexList& indices,
                                              QModelIndex& oppositeIndex)
{
    int idNewParent = opposite->getId(oppositeIndex);
    for (const QModelIndex &index: indices) {
        bool res;
        int idSrc = getId(index);
        if (!isOperation(index))
            continue;
        QString sqlM = "update hw_transfer set id_tt=%1 where id=%2";
        res = m_db->execSimpleQuery(sqlM.arg(idNewParent).arg(idSrc));
        if (!res)
            return false;
    }
    return true;
}

bool TransferTypeHierModel::mergeSelectedNodes(HierModelBase* opposite,
                                               QModelIndex& index,
                                               QModelIndex& oppositeIndex)
{
    int idSrc = getId(index);
    int idDest = opposite->getId(oppositeIndex);
    if (!isSubcategory(index))
        return false;
    QString sqlM = "update hw_transfer set id_tt=%1 where id_tt=%2";
    QString sqlD = "delete from hw_transfer_type where id=%1";
    // Merge children and delete old parent
    if (m_db->execSimpleQuery(sqlM.arg(idDest).arg(idSrc)))
        return m_db->execSimpleQuery(sqlD.arg(idSrc));
    else
        return false;
}

// Private helper methods

QModelIndex TransferTypeHierModel::indexFromItem(int itemIndex, int column) const
{
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QModelIndex();

    int row = -1;
    const TransferItem &item = m_items[itemIndex];

    if (item.parentId == -1) {
        // Root transfer type
        row = m_rootItems.indexOf(itemIndex);
    } else {
        // Transfer - find parent
        int parentIndex = findItemIndex(item.parentId, true);
        if (parentIndex != -1) {
            row = m_items[parentIndex].children.indexOf(itemIndex);
        }
    }

    if (row == -1)
        return QModelIndex();

    return createIndex(row, column, itemIndex);
}

QString TransferTypeHierModel::formatAmount(int amountInLowUnits) const
{
    double amountInMainUnits = amountInLowUnits / 100.0;
    return QLocale().toString(amountInMainUnits, 'f', 2);
}

bool TransferTypeHierModel::removeTransferType(int typeId)
{
    QSqlQuery query;
    // First remove all transfers of this type
    query.prepare("DELETE FROM hw_transfer WHERE id_tt = :typeId");
    query.bindValue(":typeId", typeId);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    // Then remove the type itself
    query.prepare("DELETE FROM hw_transfer_type WHERE id = :typeId");
    query.bindValue(":typeId", typeId);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

bool TransferTypeHierModel::removeTransfer(int transferId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM hw_transfer WHERE id = :transferId");
    query.bindValue(":transferId", transferId);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}
