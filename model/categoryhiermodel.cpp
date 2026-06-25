/* Home Wallet
 *
 * Module: Categories an subcategories model
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

#include "categoryhiermodel.h"
#include "hwdatabase.h"
#include "globals.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QLocale>

#define S_SUBCAT_MERGED QObject::tr("Subcategories merged: %1")

CategoryHierModel::CategoryHierModel(bool isExpense, HwDatabase* db, QObject *parent) :
    HierModelBase(db, parent),
    m_isExpense(isExpense)
{
    loadData();
}

CategoryHierModel::~CategoryHierModel()
{
}

void CategoryHierModel::clearData()
{
    m_items.clear();
    m_rootItems.clear();
}

bool CategoryHierModel::removeByIndex(const QModelIndex& indexToRemove)
{
    // This methods must be called BEFORE beginRemoveRows()
    int idToRemove = getId(indexToRemove);
    bool isC = isCategory(indexToRemove);
    bool isS = isSubcategory(indexToRemove);
    bool res = false;
    int removingRow = indexToRemove.row();
    QModelIndex parentIndex = indexToRemove.parent();

    // Physical remove from database
    if (m_isExpense) {
        if (isC)
            res = m_db->deleteExpenseCategory(idToRemove);
        else if (isS)
            res = m_db->deleteExpenseSubcategory(idToRemove);
    }
    else {
        if (isC)
            res = m_db->deleteIncomeCategory(idToRemove);
        else if (isS)
            res = m_db->deleteIncomeSubcategory(idToRemove);
    }
    if (!res)
        return false;

    // Remove from model and transfer to view
    beginRemoveRows(parentIndex, removingRow, removingRow);
    if (parentIndex.isValid()) // Subcategory, not category
        m_items[parentIndex.internalId()].children.remove(removingRow);
    int removeIndex = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == idToRemove) {
            removeIndex = i;
            break;
        }
    }
    if (removeIndex != -1)
        m_items.remove(removeIndex);
    // Indices correction
    for (int i = 0; i < m_items.size(); ++i) {
        for (int j = 0; j < m_items[i].children.size(); ++j) {
            if (m_items[i].children[j] > removeIndex) {
                m_items[i].children[j]--;
            }
        }
    }
    endRemoveRows();

    return true;
}

bool CategoryHierModel::moveSubcategory(int idSrc, int idNewParent, QStringList &mergedSubcategories)
{
    // Check for present same name!
    QString sqlCN = m_isExpense ?
        "select sc2.id, sc2.name" \
        " from hw_ex_subcat sc1, hw_ex_subcat sc2" \
        " where sc1.name=sc2.name and sc2.id_ecat=%1" \
        " and sc1.id=%2"
        :
        "select sc2.id, sc2.name" \
        " from hw_in_subcat sc1, hw_in_subcat sc2" \
        " where sc1.name=sc2.name and sc2.id_icat=%1" \
        " and sc1.id=%2";
    QString sqlM = m_isExpense
        ? "update hw_ex_subcat set id_ecat=%1 where id=%2"
        : "update hw_in_subcat set id_icat=%1 where id=%2";

    // If same name present in new parent - merge
    QSqlQuery qCN(m_db->sqlDbRef());
    if (!m_db->prepQuery(qCN, sqlCN.arg(idNewParent).arg(idSrc)))
        return false;
    if (!m_db->execQuery(qCN))
        return false;
    if (m_db->queryRecCount(qCN)>0) {
        qCN.first();
        int idDest = qCN.value(0).toInt();
        mergedSubcategories << qCN.value(1).toString();
        return mergeSubcategories(idSrc, idDest);
    }
    else // simply move
        return m_db->execSimpleQuery(sqlM.arg(idNewParent).arg(idSrc));
}

bool CategoryHierModel::moveOperation(int idSrc, int idNewParent)
{
    QString sqlM = m_isExpense
        ? "update hw_ex_op set id_esubcat=%1 where id=%2"
        : "update hw_in_op set id_isubcat=%1 where id=%2";
    return m_db->execSimpleQuery(sqlM.arg(idNewParent).arg(idSrc));
}

bool CategoryHierModel::mergeCategories(int idSrc, int idDest)
{
    QString sqlM = m_isExpense
        ? "update hw_ex_subcat set id_ecat=%1 where id_ecat=%2"
        : "update hw_in_subcat set id_icat=%1 where id_icat=%2";
    QString sqlD = m_isExpense
        ? "delete from hw_ex_cat where id=%1"
        : "delete from hw_in_cat where id=%1";
    // Subcategories name uniq
    if (!preMergeChildren(idSrc, idDest))
        return false;
    // Merge children and delete old parent
    if (m_db->execSimpleQuery(sqlM.arg(idDest).arg(idSrc)))
        return m_db->execSimpleQuery(sqlD.arg(idSrc));
    else
        return false;
}

bool CategoryHierModel::mergeSubcategories(int idSrc, int idDest)
{
    QString sqlM = m_isExpense
            ? "update hw_ex_op set id_esubcat=%1 where id_esubcat=%2"
            : "update hw_in_op set id_isubcat=%1 where id_isubcat=%2";
    QString sqlD = m_isExpense
        ? "delete from hw_ex_subcat where id=%1"
        : "delete from hw_in_subcat where id=%1";
    // Merge children and delete old parent
    if (m_db->execSimpleQuery(sqlM.arg(idDest).arg(idSrc)))
        return m_db->execSimpleQuery(sqlD.arg(idSrc));
    else
        return false;
}

bool CategoryHierModel::preMergeChildren(int idSrc, int idDest)
{
    QString sqlCU = m_isExpense ?
        "select sc1.id as id1, sc2.id as id2, sc1.name as nm" \
        " from hw_ex_subcat sc1, hw_ex_subcat sc2" \
        " where sc1.name=sc2.name" \
        " and sc1.id_ecat=%1 and sc2.id_ecat=%2"
        :
        "select sc1.id as id1, sc2.id as id2, sc1.name as nm" \
        " from hw_in_subcat sc1, hw_in_subcat sc2" \
        " where sc1.name=sc2.name" \
        " and sc1.id_icat=%1 and sc2.id_icat=%2";
    QSqlQuery qCU(m_db->sqlDbRef());
    if (!m_db->prepQuery(qCU, sqlCU.arg(idSrc).arg(idDest)))
        return false;
    if (!m_db->execQuery(qCU))
        return false;
    qCU.first();
    QStringList mergedSubcategories;
    while (qCU.isValid()) {
        mergeSubcategories(qCU.value(0).toInt(), qCU.value(1).toInt());
        mergedSubcategories << qCU.value(2).toString();
        qCU.next();
    }
    if (!mergedSubcategories.isEmpty())
        emit infoForUser(S_SUBCAT_MERGED.arg(mergedSubcategories.join(", ")));
    return true;
}

void CategoryHierModel::loadData()
{
    beginResetModel();
    clearData();
    
    loadCategories();
    loadSubcategories();
    
    if (m_showOperations) {
        loadOperations();
    }
    
    buildHierarchy();
    
    endResetModel();
}

void CategoryHierModel::loadCategories()
{
    QString catTable = m_isExpense ? "hw_ex_cat" : "hw_in_cat";
    QSqlQuery query(QString("SELECT id, name, descr FROM %1 ORDER BY name").arg(catTable));
    
    while (query.next()) {
        CategoryItem item;
        item.id = query.value(0).toInt();
        item.parentId = -1;
        item.name = query.value(1).toString();
        item.description = query.value(2).toString();
        item.defaultUnitId = -1;
        item.isCategory = true;
        item.isOperation = false;
        
        m_items.append(item);
    }
}

void CategoryHierModel::loadSubcategories()
{
    QString subcatTable = m_isExpense ? "hw_ex_subcat" : "hw_in_subcat";
    QString idCategoryField = m_isExpense ? "id_ecat" : "id_icat";
    QSqlQuery query(QString("SELECT id, %1, name, descr, id_un_default FROM %2 ORDER BY name")
                    .arg(idCategoryField, subcatTable));
    
    while (query.next()) {
        CategoryItem item;
        item.id = query.value(0).toInt();
        item.parentId = query.value(1).toInt();
        item.name = query.value(2).toString();
        item.description = query.value(3).toString();
        item.defaultUnitId = query.value(4).toInt();
        item.isCategory = false;
        item.isOperation = false;
        
        m_items.append(item);
    }
}

void CategoryHierModel::loadOperations()
{
    QString opTable = m_isExpense ? "hw_ex_op" : "hw_in_op";
    QString idSubcatField = m_isExpense ? "id_esubcat" : "id_isubcat";
    
    QSqlQuery query(QString("SELECT id, %1, op_date, quantity, amount, descr "
                           "FROM %2 ORDER BY op_date DESC")
                    .arg(idSubcatField, opTable));
    
    while (query.next()) {
        CategoryItem item;
        item.id = query.value(0).toInt();
        item.parentId = query.value(1).toInt();  // Subcategory ID
        item.name = query.value(5).toString();   // Operation description
        if (item.name.isEmpty()) {
            item.name = tr("Operation #%1").arg(item.id);
        }
        item.operationDate = query.value(2).toDate();
        item.quantity = query.value(3).toDouble();
        item.amount = query.value(4).toInt();
        item.isCategory = false;
        item.isOperation = true;
        
        m_items.append(item);
    }
}

void CategoryHierModel::buildHierarchy()
{
    // Clear existing children lists
    for (int i = 0; i < m_items.size(); ++i) {
        m_items[i].children.clear();
    }
    
    // Build children lists
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].parentId == -1) {
            // Root item (category)
            m_rootItems.append(i);
        } else {
            // Find parent item index
            int parentIndex = -1;
            
            if (m_items[i].isOperation) {
                // Operation's parent is subcategory
                parentIndex = findItemIndex(m_items[i].parentId, false, true);
            } else {
                // Subcategory's parent is category
                parentIndex = findItemIndex(m_items[i].parentId, true, false);
            }
            
            if (parentIndex != -1) {
                m_items[parentIndex].children.append(i);
            }
        }
    }
}

int CategoryHierModel::findItemIndex(int id, bool isCategory, bool isSubcategory) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            if (isCategory && m_items[i].isCategory && !m_items[i].isOperation) {
                return i;
            }
            if (isSubcategory && !m_items[i].isCategory && !m_items[i].isOperation) {
                return i;
            }
            if (!isCategory && !isSubcategory && m_items[i].isOperation) {
                return i;
            }
        }
    }
    return -1;
}

void CategoryHierModel::refresh()
{
    loadData();
}

QString CategoryHierModel::formatAmount(int amountInLowUnits) const
{
    // Convert from low units (cents/kopeks) to main units
    double amountInMainUnits = amountInLowUnits / 100.0;
    
    // Format with exactly 2 decimal places
    return QLocale().toString(amountInMainUnits, 'f', 2);
}

int CategoryHierModel::getChildCountForDisplay(const CategoryItem &item) const
{

    if (item.isCategory) {
        return item.children.size();  // Subcategories count
    }

    if (!item.isOperation && m_showOperations) {
        return item.children.size();  // Operations count
    }

    return 0;
}

QModelIndex CategoryHierModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    
    if (!parent.isValid()) {
        // Top-level item (category)
        if (row < 0 || row >= m_rootItems.size())
            return QModelIndex();
        return indexFromItem(m_rootItems[row], column);
    }
    
    // Child item (subcategory or operation)
    int parentIndex = parent.internalId();
    if (parentIndex < 0 || parentIndex >= m_items.size())
        return QModelIndex();
    
    if (row < 0 || row >= m_items[parentIndex].children.size())
        return QModelIndex();
    
    int childIndex = m_items[parentIndex].children[row];
    return indexFromItem(childIndex, column);
}

QModelIndex CategoryHierModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
    
    int childIndex = child.internalId();
    if (childIndex < 0 || childIndex >= m_items.size())
        return QModelIndex();
    
    int parentId = m_items[childIndex].parentId;
    if (parentId == -1)
        return QModelIndex();
    
    // Find parent item index
    int parentIndex = -1;
    
    if (m_items[childIndex].isOperation) {
        // Parent of operation is subcategory
        parentIndex = findItemIndex(parentId, false, true);
    } else {
        // Parent of subcategory is category
        parentIndex = findItemIndex(parentId, true, false);
    }
    
    if (parentIndex == -1)
        return QModelIndex();
    
    // Find parent's row
    int row = -1;
    if (m_items[parentIndex].parentId == -1) {
        row = m_rootItems.indexOf(parentIndex);
    } else {
        int grandParentIndex = findItemIndex(m_items[parentIndex].parentId, true, false);
        if (grandParentIndex != -1) {
            row = m_items[grandParentIndex].children.indexOf(parentIndex);
        }
    }
    
    if (row == -1)
        return QModelIndex();
    
    return createIndex(row, 0, parentIndex);
}

int CategoryHierModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_rootItems.size();
    }
    
    int parentIndex = parent.internalId();
    if (parentIndex < 0 || parentIndex >= m_items.size())
        return 0;
    
    return m_items[parentIndex].children.size();
}

int CategoryHierModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;  // Two columns: Name and Count
}

QVariant CategoryHierModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QVariant();
    
    const CategoryItem &item = m_items[itemIndex];
    int column = index.column();
    
    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            if (column == 0) {
                // First column: name
                if (item.isOperation) {
                    return QString("%1 - %2: %3")
                        .arg(item.operationDate.toString("dd.MM.yyyy"))
                        .arg(item.name)
                        .arg(formatAmount(item.amount));
                }
                return item.name;
            } else if (column == 1) {
                // Second column: child count
                int count = getChildCountForDisplay(item);
                if (count > 0) {
                    return QString::number(count);
                }
                return QVariant();  // Empty for items without children
            }
            break;
            
        case Qt::ToolTipRole:
            if (column == 0) {
                if (item.isOperation) {
                    return QString("%1\nAmount: %2\nQuantity: %3")
                        .arg(item.name)
                        .arg(formatAmount(item.amount))
                        .arg(item.quantity);
                }
                if (!item.description.isEmpty())
                    return item.description;
                return item.name;
            } else if (column == 1) {
                int count = getChildCountForDisplay(item);
                if (item.isCategory && count > 0) {
                    return tr("Subcategories: %1").arg(count);
                } else if (!item.isOperation && !item.isCategory && count > 0) {
                    return tr("Operations: %1").arg(count);
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
                if (item.isCategory)
                    return 0;
                else if (item.isOperation)
                    return 2;
                else
                    return 1;
            }
            break;
            
        default:
            return QVariant();
    }
    
    return QVariant();
}

Qt::ItemFlags CategoryHierModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant CategoryHierModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return QString(S_COL_CATEGORY);
        } else if (section == 1) {
            return tr("Records");
        }
    }
    return QVariant();
}

QModelIndex CategoryHierModel::indexFromItem(int itemIndex, int column) const
{
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QModelIndex();
    
    int row = -1;
    const CategoryItem &item = m_items[itemIndex];
    
    if (item.parentId == -1) {
        // Root category
        row = m_rootItems.indexOf(itemIndex);
    } else {
        // Find parent index
        int parentIndex = -1;
        if (item.isOperation) {
            parentIndex = findItemIndex(item.parentId, false, true);
        } else {
            parentIndex = findItemIndex(item.parentId, true, false);
        }
        
        if (parentIndex != -1) {
            row = m_items[parentIndex].children.indexOf(itemIndex);
        }
    }
    
    if (row == -1)
        return QModelIndex();
    
    return createIndex(row, column, itemIndex);
}

int CategoryHierModel::getId(const QModelIndex &index) const
{
    if (!index.isValid())
        return -1;
    
    // ID is stored in UserRole+1, but only for column 0
    QModelIndex nameIndex = index.sibling(index.row(), 0);
    int itemIndex = nameIndex.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return -1;
    
    return m_items[itemIndex].id;
}

QString CategoryHierModel::getName(const QModelIndex &index) const
{
    return data(index).toString();
}

bool CategoryHierModel::isCategory(const QModelIndex &index) const
{
    if (!index.isValid())
        return false;
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return false;
    
    return m_items[itemIndex].isCategory;
}

bool CategoryHierModel::isSubcategory(const QModelIndex &index) const
{
    if (!index.isValid())
        return false;
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return false;
    
    return !m_items[itemIndex].isCategory && !m_items[itemIndex].isOperation;
}

bool CategoryHierModel::isOperation(const QModelIndex &index) const
{
    if (!index.isValid())
        return false;
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return false;
    
    return m_items[itemIndex].isOperation;
}

bool CategoryHierModel::isExpense() const
{
    return m_isExpense;
}

int CategoryHierModel::getParentCategoryId(const QModelIndex &index) const
{
    if (!index.isValid())
        return -1;
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return -1;
    
    const CategoryItem &item = m_items[itemIndex];
    
    if (item.isCategory)
        return -1;
    
    if (item.isOperation) {
        // Find parent subcategory, then its parent category
        int subcatIndex = findItemIndex(item.parentId, false, true);
        if (subcatIndex != -1) {
            return m_items[subcatIndex].parentId;
        }
        return -1;
    }
    
    // Subcategory
    return item.parentId;
}

int CategoryHierModel::getParentSubcategoryId(const QModelIndex &index) const
{
    if (!index.isValid())
        return -1;
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return -1;
    
    const CategoryItem &item = m_items[itemIndex];
    
    if (!item.isOperation)
        return -1;
    
    return item.parentId;
}

double CategoryHierModel::getQuantity(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0.0;
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return 0.0;
    
    return m_items[itemIndex].quantity;
}

int CategoryHierModel::getAmount(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return 0;
    
    return m_items[itemIndex].amount;
}

QDate CategoryHierModel::getOperationDate(const QModelIndex &index) const
{
    if (!index.isValid())
        return QDate();
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QDate();
    
    return m_items[itemIndex].operationDate;
}

bool CategoryHierModel::removeAnyRows(QModelIndexList &indices)
{
    // Reverse sort
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    std::sort(indices.rbegin(), indices.rend());
#else
#warning Multiple remove don''t work in Qt before 5.6!
    if (indices.count()>1) {
        m_lastError = "Multiple remove don't work in Qt before 5.6, select strictly one record";
        return false;
    }
#endif
    for (QModelIndex& index: indices) {
        if (!removeByIndex(index)) {
            m_lastError = m_db->lastError();
            return false;
        }
    }
    return true;

}

bool CategoryHierModel::moveSelectedNodes(HierModelBase *opposite, QModelIndexList &indices, QModelIndex &oppositeIndex)
{
    int idNewParent = opposite->getId(oppositeIndex);

    QStringList mergedSubcategories;
    for (const QModelIndex &index: indices)
    {
        bool res;
        int idSrc = getId(index);
        if (isSubcategory(index))
            res = moveSubcategory(idSrc, idNewParent, mergedSubcategories);
        else if (isOperation(index))
            res = moveOperation(idSrc, idNewParent);
        if (!res)
            return false;
    }

    if (!mergedSubcategories.isEmpty())
        emit infoForUser(S_SUBCAT_MERGED.arg(mergedSubcategories.join(", ")));
    return true;
}

bool CategoryHierModel::mergeSelectedNodes(HierModelBase *opposite,
    QModelIndex& index, QModelIndex& oppositeIndex)
{
    int idSrc = getId(index);
    int idDest = opposite->getId(oppositeIndex);
    bool res = false;

    if (isCategory(index))
        res = mergeCategories(idSrc, idDest);
    else if (isSubcategory(index))
        res = mergeSubcategories(idSrc, idDest);

    if (!res)
        m_lastError = m_db->lastError();
    return res;
}
