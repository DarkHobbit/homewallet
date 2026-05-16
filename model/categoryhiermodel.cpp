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

CategoryHierModel::CategoryHierModel(bool isExpense, HwDatabase* db, QObject *parent) :
    QAbstractItemModel(parent),
    m_isExpense(isExpense),
    m_db(db),
    m_showOperations(false)
{
    Q_ASSERT(m_db != nullptr);
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

void CategoryHierModel::setOperationShow(bool show)
{
    if (m_showOperations == show)
        return;
    
    m_showOperations = show;
    refresh();
}

QString CategoryHierModel::formatAmount(int amountInLowUnits) const
{
    // Convert from low units (cents/kopeks) to main units
    double amountInMainUnits = amountInLowUnits / 100.0;
    
    // Format with exactly 2 decimal places
    return QLocale().toString(amountInMainUnits, 'f', 2);
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
    return 1;  // Only one column for name
}

QVariant CategoryHierModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return QVariant();
    
    const CategoryItem &item = m_items[itemIndex];
    
    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            if (item.isOperation) {
                // For operations: show date, description, and formatted amount
                return QString("%1 - %2: %3")
                    .arg(item.operationDate.toString("dd.MM.yyyy"))
                    .arg(item.name)
                    .arg(formatAmount(item.amount));
            }
            return item.name;
            
        case Qt::ToolTipRole:
            if (item.isOperation) {
                return QString("%1\nAmount: %2\nQuantity: %3")
                    .arg(item.name)
                    .arg(formatAmount(item.amount))
                    .arg(item.quantity);
            }
            if (!item.description.isEmpty())
                return item.description;
            return item.name;
            
        case Qt::UserRole + 1: // ID
            return item.id;
            
        case Qt::UserRole + 2: // Type (0=category,1=subcategory,2=operation)
            if (item.isCategory)
                return 0;
            else if (item.isOperation)
                return 2;
            else
                return 1;
            
        default:
            return QVariant();
    }
}

Qt::ItemFlags CategoryHierModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant CategoryHierModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0) {
        return QString(S_COL_CATEGORY);
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
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return -1;
    
    return m_items[itemIndex].id;
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