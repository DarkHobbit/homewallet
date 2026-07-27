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
        item.isCurrency = false;

        m_items.append(item);
    }
}

void AccountHierModel::loadOperations()
{
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
            parent_id_for_history
        FROM (
            -- Currencies by account (level 1)
            SELECT
                ai.id AS id,
                NULL AS op_date,
                ai.id_ac AS account_id,
                ai.init_sum AS amount,
                1 AS quantity,
                cur.full_name AS description,
                10 AS operation_type,  -- Start balance
                -1 AS parent_id_for_history
            FROM hw_acc_init ai
            JOIN hw_currency cur ON ai.id_cur = cur.id

            UNION ALL

            -- History of currencies (level 2)
            SELECT
                ah.id AS id,
                ah.ch_date AS op_date,
                ah.id_ai AS account_id,  -- References hw_acc_init
                ah.sum_calc AS amount,
                0 AS quantity,
                'History record' AS description,
                11 AS operation_type,  -- History record
                ah.id_ai AS parent_id_for_history
            FROM hw_acc_hist ah

            UNION ALL

            -- Incomes
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                amount,
                quantity,
                COALESCE(descr, '') AS description,
                1 AS operation_type,
                -1 AS parent_id_for_history
            FROM hw_in_op

            UNION ALL

            -- Expenses
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                amount,
                quantity,
                COALESCE(descr, '') AS description,
                2 AS operation_type,
                -1 AS parent_id_for_history
            FROM hw_ex_op

            UNION ALL

            -- Receipts
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                total_amount AS amount,
                0 AS quantity,
                COALESCE(note, '') AS description,
                3 AS operation_type,
                -1 AS parent_id_for_history
            FROM hw_receipt

            UNION ALL

            -- Transfers (outgoing)
            SELECT
                id,
                op_date,
                id_ac_out AS account_id,
                -amount AS amount,
                0 AS quantity,
                COALESCE(descr, '') AS description,
                4 AS operation_type,
                -1 AS parent_id_for_history
            FROM hw_transfer

            UNION ALL

            -- Transfers (incoming)
            SELECT
                id,
                op_date,
                id_ac_in AS account_id,
                amount AS amount,
                0 AS quantity,
                COALESCE(descr, '') AS description,
                5 AS operation_type,
                -1 AS parent_id_for_history
            FROM hw_transfer

            UNION ALL

            -- Currency exchanges
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                amount_out AS amount,
                0 AS quantity,
                COALESCE(descr, '') AS description,
                6 AS operation_type,
                -1 AS parent_id_for_history
            FROM hw_curr_exch

            UNION ALL

            -- Credits
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                amount,
                0 AS quantity,
                COALESCE(descr, '') AS description,
                CASE WHEN is_lend = 1 THEN 7 ELSE 8 END AS operation_type,
                -1 AS parent_id_for_history
            FROM hw_credit

            UNION ALL

            -- Repayments
            SELECT
                id,
                op_date,
                id_ac AS account_id,
                amount,
                0 AS quantity,
                COALESCE(descr, '') AS description,
                9 AS operation_type,
                -1 AS parent_id_for_history
            FROM hw_repayment
        ) AS all_operations
        ORDER BY 
            CASE WHEN operation_type = 10 THEN 0 ELSE 1 END,  -- Currencies first
            op_date DESC
    )");
    if (!res) {
        qDebug() << "Failed to load operations:" << query.lastError().text();
        m_lastError = query.lastError().text();
        return;
    }

    if (!query.exec()) {
        qDebug() << "Failed to load operations:" << query.lastError().text();
        m_lastError = query.lastError().text();
        return;
    }

    while (query.next()) {
        AccountItem item;
        item.id = query.value("id").toInt();
        item.operationDate = query.value("op_date").toDate();
        item.amount = query.value("amount").toInt();
        item.quantity = query.value("quantity").toDouble();
        item.isOperation = true;
        item.operationType = query.value("operation_type").toInt();

        int accountId = query.value("account_id").toInt();
        int parentForHistory = query.value("parent_id_for_history").toInt();

        if (item.operationType == 11) {
            // History record: parentId = id of hw_acc_init record
            item.parentId = parentForHistory;
            item.isCurrency = true;
            item.name = tr("History: %1")
                .arg(item.operationDate.toString("dd.MM.yyyy"));
        } else {
            // Currencies and regular operations: parentId = account ID
            item.parentId = accountId;
            item.isCurrency = (item.operationType == 10);
            QString descr = query.value("description").toString();
            if (descr.isEmpty()) {
                descr = getDefaultOperationName(item.operationType, item.id);
            }
            item.name = descr;
        }

        m_items.append(item);
    }
}

void AccountHierModel::buildHierarchy()
{
    // Clear existing children lists
    for (int i = 0; i < m_items.size(); ++i) {
        m_items[i].children.clear();
    }
    m_rootItems.clear();

    // Build hierarchy
    for (int i = 0; i < m_items.size(); ++i) {
        AccountItem &item = m_items[i];

        if (item.parentId == -1) {
            // Root items (accounts)
            m_rootItems.append(i);
        } else if (item.operationType == 11) {
            // History records (level 2): parent is currency record (operationType == 10)
            bool found = false;
            for (int j = 0; j < m_items.size(); ++j) {
                if (m_items[j].operationType == 10 && m_items[j].id == item.parentId) {
                    m_items[j].children.append(i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                qDebug() << "Warning: History record" << item.id
                         << "has no parent currency record" << item.parentId;
            }
        } else {
            // Currencies and regular operations (level 1): parent is account
            int parentIndex = findItemIndex(item.parentId);
            if (parentIndex != -1) {
                m_items[parentIndex].children.append(i);
            } else {
                qDebug() << "Warning: Operation" << item.id
                         << "has no parent account" << item.parentId;
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

    // Child item
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

    // Find parent item
    int parentIndex = -1;

    // For history records, parent is currency record (operationType == 10)
    if (m_items[childIndex].operationType == 11) {
        for (int i = 0; i < m_items.size(); ++i) {
            if (m_items[i].operationType == 10 && m_items[i].id == parentId) {
                parentIndex = i;
                break;
            }
        }
    } else {
        // For regular operations and currencies, parent is account
        parentIndex = findItemIndex(parentId);
    }

    if (parentIndex == -1)
        return QModelIndex();

    // Find parent's row
    int row = -1;
    if (m_items[parentIndex].parentId == -1) {
        row = m_rootItems.indexOf(parentIndex);
    } else {
        // Parent is not root (e.g., currency has parent account)
        int grandParentIndex = findItemIndex(m_items[parentIndex].parentId);
        if (grandParentIndex != -1) {
            row = m_items[grandParentIndex].children.indexOf(parentIndex);
        }
    }

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
                if (item.isOperation) {
                    if (item.operationType == 11) {
                        // History record
                        return QString("%1 - %2: %3")
                            .arg(item.operationDate.toString("dd.MM.yyyy"))
                            .arg(item.name)
                            .arg(formatAmount(item.amount));
                    } else if (item.operationType == 10) {
                        // Currency
                        return item.name;
                    } else {
                        // Regular operation
                        return QString("%1 - %2")
                            .arg(item.operationDate.toString("dd.MM.yyyy"))
                            .arg(item.name);
                    }
                }
                return item.name;
            } else if (column == 1) {
                if (!item.isOperation) {
                    // Account: count of children
                    int count = item.children.size();
                    return count > 0 ? QString::number(count) : QVariant();
                } else if (item.operationType == 10) {
                    // Currency: count of history records
                    int count = item.children.size();
                    return count > 0 ? QString::number(count) : QVariant();
                } else if (item.operationType == 11) {
                    // History: amount change
                    return formatAmount(item.amount);
                } else {
                    // Regular operation: amount
                    return formatAmount(item.amount);
                }
            }
            break;

        case Qt::ToolTipRole:
            if (column == 0) {
                if (item.isOperation) {
                    QString typeName = getOperationTypeName(item.operationType);
                    if (item.operationType == 11) {
                        return tr("History record for currency\nDate: %1\nAmount: %2")
                            .arg(item.operationDate.toString("dd.MM.yyyy"))
                            .arg(formatAmount(item.amount));
                    }
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
                } else if (item.operationType == 10) {
                    int count = item.children.size();
                    if (count > 0) {
                        return tr("History records: %1").arg(count);
                    }
                }
            }
            break;

        case Qt::TextAlignmentRole:
            if (column == 1) {
                if (!item.isOperation || item.operationType == 10) {
                    return Qt::AlignCenter;
                } else {
                    return Qt::AlignRight;
                }
            }
            break;

        case Qt::UserRole + 1: // ID
            if (column == 0) {
                return item.id;
            }
            break;

        case Qt::UserRole + 2: // Type (0=category,1=subcategory,2=operation,3=currency/history)
            if (column == 0) {
                if (!item.isOperation)
                    return 1;  // Account
                else if (item.operationType == 10 || item.operationType == 11)
                    return 3;  // Currency or history
                else
                    return 2;  // Regular operation
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

    const AccountItem &item = m_items[itemIndex];
    
    // Accounts and currencies are considered "subcategories"
    return !item.isOperation || item.operationType == 10;
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

    if (item.operationType == 11) {
        // For history, return hw_acc_init ID
        return item.parentId;
    }

    return item.parentId;  // For regular operations, return account ID
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

    for (const QModelIndex &index : indices) {
        if (!index.isValid())
            continue;

        // For now, we only support removing operations
        // Account removal would need to handle cascading deletes
        if (isSubcategory(index)) {
            bool success = m_db->deleteAccount(getId(index));
            if (!success) {
                m_lastError = m_db->lastError();
                return false;
            }
        }
    }

    refresh();
    return true;;
}

bool AccountHierModel::moveSelectedNodes(HierModelBase* opposite,
                                         QModelIndexList& indices,
                                         QModelIndex& oppositeIndex)
{
    QStringList sqlsMove = QStringList() << ""
        << "update hw_in_op set id_ac=:id_ac where id=:id"
        << "update hw_ex_op set id_ac=:id_ac where id=:id"
        << "update hw_receipt set id_ac=:id_ac where id=:id"
        << "update hw_transfer set id_ac_out=:id_ac where id=:id"
        << "update hw_transfer set id_ac_in=:id_ac where id=:id"
        << "update hw_curr_exch set id_ac=:id_ac where id=:id"
        << "update hw_credit set id_ac=:id_ac where id=:id"
        << "update hw_credit set id_ac=:id_ac where id=:id"
        << "update hw_repayment set id_ac=:id_ac where id=:id"
        << "" // acc_init, if need - merge, not move
        << "" // acc_hist referense acc_init, not account
    ;
    int idNewParent = opposite->getId(oppositeIndex);
    for (const QModelIndex &index: indices) {
        if (isOperation(index)) {
            int opType = getOperationType(index);
            if (opType<1 || opType>=sqlsMove.count()) {
                m_lastError = QString("Unknown operation type: ").arg(opType);
                return false;
            }
            QString sql = sqlsMove[opType];
            if (sql.isEmpty())
                continue;
            QSqlQuery qMv(m_db->sqlDbRef());
            if (!m_db->prepQuery(qMv, sql)) {
                m_lastError = m_db->lastError();
                return false;
            }
            qMv.bindValue(":id_ac", idNewParent);
            qMv.bindValue(":id", getId(index));
            if (!m_db->execQuery(qMv)) {
                m_lastError = m_db->lastError();
                return false;
            }
        }
    }
    return true;
}

bool AccountHierModel::mergeSelectedNodes(HierModelBase* opposite,
                                          QModelIndex& index,
                                          QModelIndex& oppositeIndex)
{
    // TODO
    Q_UNUSED(opposite);
    Q_UNUSED(index);
    Q_UNUSED(oppositeIndex);
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
        // Child item - find parent
        int parentIndex = -1;
        
        if (item.operationType == 11) {
            // History record: parent is currency record
            for (int i = 0; i < m_items.size(); ++i) {
                if (m_items[i].operationType == 10 && m_items[i].id == item.parentId) {
                    parentIndex = i;
                    break;
                }
            }
        } else {
            // Regular operation or currency: parent is account
            parentIndex = findItemIndex(item.parentId);
        }
        
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

int AccountHierModel::getOperationType(const QModelIndex &index) const
{
    if (!index.isValid())
        return -1;

    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return -1;

    return m_items[itemIndex].operationType;
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
        case 10: return tr("Start balance");
        case 11: return tr("History record");
        default: return tr("Unknown");
    }
}

QString AccountHierModel::getDefaultOperationName(int type, int id) const
{
    return getOperationTypeName(type) + tr(" #%1").arg(id);
}