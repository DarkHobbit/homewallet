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

#include "accounthiermodel.h"
#include "globals.h"
#include "hwdatabase.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QLocale>

AccountHierModel::AccountHierModel(HwDatabase* db, QObject *parent)
    : HierModelBase(db, parent)
{
    loadData();
}

AccountHierModel::~AccountHierModel()
{
}

void AccountHierModel::clearData()
{
    m_items.clear();
    m_rootItems.clear();
}

void AccountHierModel::loadData()
{
    beginResetModel();
    clearData();

    loadAccounts();

    if (m_showOperations) {
        loadOperations();
    }

    buildHierarchy();

    endResetModel();
}

void AccountHierModel::loadAccounts()
{
    QSqlQuery query("SELECT id, name, descr FROM hw_account ORDER BY name");

    while (query.next()) {
        AccountItem item;
        item.id = query.value(0).toInt();
        item.parentId = -1;
        item.name = query.value(1).toString();
        item.description = query.value(2).toString();
        item.isOperation = false;

        m_items.append(item);
    }
}
#include <iostream>
void AccountHierModel::loadOperations()
{
    // Unified query with UNION for all operation types
    // This single query replaces 7 separate methods
    QSqlQuery query;
    bool res = query.prepare(R"(
        SELECT
            id,
            op_date,
            account_id,
            amount,
            quantity,
            description,
            operation_type,
            is_credit
        FROM (
            -- Incomes
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                amount,
                quantity,
                descr AS description,
                1 AS operation_type,
                0 AS is_credit
            FROM hw_in_op

            UNION ALL

            -- Expenses
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                amount,
                quantity,
                descr AS description,
                2 AS operation_type,
                0 AS is_credit
            FROM hw_ex_op

            UNION ALL

            -- Receipts
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                total_amount AS amount,
                0 AS quantity,
                note AS description,
                3 AS operation_type,
                0 AS is_credit
            FROM hw_receipt

            UNION ALL

            -- Transfers (outgoing)
            SELECT
                id,
                op_date,
                id_ac_out AS account_id,
                -amount AS amount,  -- Negative for outgoing
                0 AS quantity,
                descr AS description,
                4 AS operation_type,  -- Transfer Out
                0 AS is_credit
            FROM hw_transfer

            UNION ALL

            -- Transfers (incoming)
            SELECT
                id,
                op_date,
                id_ac_in AS account_id,
                amount AS amount,
                0 AS quantity,
                descr AS description,
                5 AS operation_type,  -- Transfer In
                0 AS is_credit
            FROM hw_transfer

            UNION ALL

            -- Currency exchanges
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                amount_out AS amount,
                0 AS quantity,
                descr AS description,
                6 AS operation_type,
                0 AS is_credit
            FROM hw_curr_exch

            UNION ALL

            -- Credits
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                amount,
                0 AS quantity,
                descr AS description,
                CASE WHEN is_lend = 1 THEN 7 ELSE 8 END AS operation_type,
                is_lend AS is_credit
            FROM hw_credit

            UNION ALL

            -- Repayments
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                amount,
                0 AS quantity,
                descr AS description,
                9 AS operation_type,
                0 AS is_credit
            FROM hw_repayment
        ) AS all_operations
        ORDER BY op_date DESC
    )");
    if (!res) {
        std::cerr << "Failed to load operations 1:" << query.lastError().text().toUtf8().data() << std::endl;
        m_lastError = query.lastError().text();
        return;
    }

    if (!query.exec()) {
        std::cerr << "Failed to load operations 2:" << query.lastError().text().toUtf8().data() << std::endl;
        m_lastError = query.lastError().text();
        return;
    }


    while (query.next()) {
        AccountItem item;
        item.id = query.value("id").toInt();
        item.parentId = query.value("account_id").toInt();
        item.operationDate = query.value("op_date").toDate();
        item.amount = query.value("amount").toInt();
        item.quantity = query.value("quantity").toDouble();
        item.isOperation = true;
        item.operationType = query.value("operation_type").toInt();

        // Build display name
        QString descr = query.value("description").toString();
        if (descr.isEmpty()) {
            descr = getOperationTypeName(item.operationType)
               + QString(" #%1").arg(item.id);
        }
        item.name = descr;

        m_items.append(item);
    }
}

void AccountHierModel::buildHierarchy()
{
    // Clear existing children lists
    for (int i = 0; i < m_items.size(); ++i) {
        m_items[i].children.clear();
    }

    // Build children lists
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].parentId == -1) {
            // Root item (account)
            m_rootItems.append(i);
        } else {
            // Find parent account index
            int parentIndex = findItemIndex(m_items[i].parentId);
            if (parentIndex != -1) {
                m_items[parentIndex].children.append(i);
            }
        }
    }
}

int AccountHierModel::findItemIndex(int id) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id && !m_items[i].isOperation) {
            return i;
        }
    }
    return -1;
}

void AccountHierModel::refresh()
{
    loadData();
}

// QAbstractItemModel implementation

QModelIndex AccountHierModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        // Top-level item (account)
        if (row < 0 || row >= m_rootItems.size())
            return QModelIndex();
        return indexFromItem(m_rootItems[row], column);
    }

    // Child item (operation)
    int parentIndex = parent.internalId();
    if (parentIndex < 0 || parentIndex >= m_items.size())
        return QModelIndex();

    if (row < 0 || row >= m_items[parentIndex].children.size())
        return QModelIndex();

    int childIndex = m_items[parentIndex].children[row];
    return indexFromItem(childIndex, column);
}

QModelIndex AccountHierModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    int childIndex = child.internalId();
    if (childIndex < 0 || childIndex >= m_items.size())
        return QModelIndex();

    int parentId = m_items[childIndex].parentId;
    if (parentId == -1)
        return QModelIndex();

    // Find parent account index
    int parentIndex = findItemIndex(parentId);
    if (parentIndex == -1)
        return QModelIndex();

    // Find parent's row among root items
    int row = m_rootItems.indexOf(parentIndex);
    if (row == -1)
        return QModelIndex();

    return createIndex(row, 0, parentIndex);
}

int AccountHierModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_rootItems.size();
    }

    int parentIndex = parent.internalId();
    if (parentIndex < 0 || parentIndex >= m_items.size())
        return 0;

    return m_items[parentIndex].children.size();
}

int AccountHierModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;  // Two columns: Name and Count/Amount
}

QVariant AccountHierModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QVariant();

    const AccountItem &item = m_items[itemIndex];
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
                // Second column: child count for accounts, amount for operations
                if (!item.isOperation) {
                    // Account - show number of operations
                    int count = item.children.size();
                    if (count > 0) {
                        return QString::number(count);
                    }
                    return QVariant();
                } else {
                    // Operation - empty
                    return QVariant();
                }
            }
            break;

        case Qt::ToolTipRole:
            if (column == 0) {
                if (item.isOperation) {
                    QString typeName = getOperationTypeName(item.operationType);
                    return tr("%1\nType: %2\nAmount: %3")
                        .arg(item.name)
                        .arg(typeName)
                        .arg(formatAmount(item.amount));
                }
                if (!item.description.isEmpty())
                    return item.description;
                return item.name;
            } else if (column == 1) {
                if (!item.isOperation) {
                    int count = item.children.size();
                    if (count > 0) {
                        return tr("Operations: %1").arg(count);
                    }
                }
            }
            break;

        case Qt::TextAlignmentRole:
            if (column == 1) {
                if (!item.isOperation) {
                    return Qt::AlignCenter;
                }
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
                    return 1;  // Account (like subcategory)
                else
                    return 2;  // Operation
            }
            break;

        default:
            return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags AccountHierModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant AccountHierModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return S_COL_ACCOUNT;
        } else if (section == 1) {
            return S_COL_REC_NUM;
        }
    }
    return QVariant();
}

// HierModelBase interface implementation

int AccountHierModel::getId(const QModelIndex &index) const
{
    if (!index.isValid())
        return -1;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return -1;

    return m_items[itemIndex].id;
}

QString AccountHierModel::getName(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QString();

    return m_items[itemIndex].name;
}

bool AccountHierModel::isCategory(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return false;
}

bool AccountHierModel::isSubcategory(const QModelIndex &index) const
{
    if (!index.isValid())
        return false;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return false;

    return !m_items[itemIndex].isOperation;  // Accounts are like subcategories
}

bool AccountHierModel::isOperation(const QModelIndex &index) const
{
    if (!index.isValid())
        return false;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return false;

    return m_items[itemIndex].isOperation;
}

int AccountHierModel::getParentCategoryId(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return -1;
}

int AccountHierModel::getParentSubcategoryId(const QModelIndex &index) const
{
    if (!index.isValid())
        return -1;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return -1;

    const AccountItem &item = m_items[itemIndex];

    if (!item.isOperation)
        return -1;

    return item.parentId;  // Returns account ID for operations
}

double AccountHierModel::getQuantity(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0.0;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return 0.0;

    return m_items[itemIndex].quantity;
}

int AccountHierModel::getAmount(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return 0;

    return m_items[itemIndex].amount;
}

QDate AccountHierModel::getOperationDate(const QModelIndex &index) const
{
    if (!index.isValid())
        return QDate();

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QDate();

    return m_items[itemIndex].operationDate;
}

bool AccountHierModel::removeAnyRows(QModelIndexList &indices)
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

        // For now, we only support removing operations
        // Account removal would need to handle cascading deletes
        if (isOperation(index)) {
            // Remove operation from database
            // ... implementation depends on operation type
            // This is a placeholder
            success = false;
        }
    }

    if (success) {
        refresh();
    }

    return success;
}

bool AccountHierModel::moveSelectedNodes(HierModelBase* opposite,
                                         QModelIndexList& indices,
                                         QModelIndex& oppositeIndex)
{
    // TODO
    return false;
}

bool AccountHierModel::mergeSelectedNodes(HierModelBase* opposite,
                                          QModelIndex& index,
                                          QModelIndex& oppositeIndex)
{
    // TODO
    return false;
}

// Private helper methods

QModelIndex AccountHierModel::indexFromItem(int itemIndex, int column) const
{
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QModelIndex();

    int row = -1;
    const AccountItem &item = m_items[itemIndex];

    if (item.parentId == -1) {
        // Root account
        row = m_rootItems.indexOf(itemIndex);
    } else {
        // Operation - find parent
        int parentIndex = findItemIndex(item.parentId);
        if (parentIndex != -1) {
            row = m_items[parentIndex].children.indexOf(itemIndex);
        }
    }

    if (row == -1)
        return QModelIndex();

    return createIndex(row, column, itemIndex);
}

QString AccountHierModel::formatAmount(int amountInLowUnits) const
{
    double amountInMainUnits = amountInLowUnits / 100.0;
    return QLocale().toString(amountInMainUnits, 'f', 2);
}

QString AccountHierModel::getOperationTypeName(int type) const
{
    switch (type) {
        case 1: return tr("Income");
        case 2: return tr("Expense");
        case 3: return tr("Receipt");
        case 4: return tr("Transfer Out");
        case 5: return tr("Transfer In");
        case 6: return tr("Currency Exchange");
        case 7: return tr("Credit Given");
        case 8: return tr("Credit Taken");
        case 9: return tr("Repayment");
        default: return tr("Unknown");
    }
}
