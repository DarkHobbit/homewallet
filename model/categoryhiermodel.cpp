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

CategoryHierModel::CategoryHierModel(bool isExpense, HwDatabase* db, QObject *parent) :
    QAbstractItemModel(parent),
    m_isExpense(isExpense),
    m_db(db)
{
    Q_ASSERT(m_db != nullptr);
    loadData();
}

CategoryHierModel::~CategoryHierModel()
{
}

void CategoryHierModel::loadData()
{
    beginResetModel();
    
    // Clear existing data
    m_items.clear();
    m_rootItems.clear();
    
    // Load categories
    QString catTable = m_isExpense ? "hw_ex_cat" : "hw_in_cat";
    QSqlQuery catQuery(QString("SELECT id, name, descr FROM %1 ORDER BY name").arg(catTable));
    
    while (catQuery.next()) {
        CategoryItem item;
        item.id = catQuery.value(0).toInt();
        item.parentId = -1;
        item.name = catQuery.value(1).toString();
        item.description = catQuery.value(2).toString();
        item.defaultUnitId = -1;
        item.isCategory = true;
        
        m_items.append(item);
    }
    
    // Load subcategories
    QString subcatTable = m_isExpense ? "hw_ex_subcat" : "hw_in_subcat";
    QString idCategoryField = m_isExpense ? "id_ecat" : "id_icat";
    QSqlQuery subcatQuery(QString("SELECT id, %1, name, descr, id_un_default FROM %2 ORDER BY name")
                          .arg(idCategoryField, subcatTable));
    
    while (subcatQuery.next()) {
        CategoryItem item;
        item.id = subcatQuery.value(0).toInt();
        item.parentId = subcatQuery.value(1).toInt();
        item.name = subcatQuery.value(2).toString();
        item.description = subcatQuery.value(3).toString();
        item.defaultUnitId = subcatQuery.value(4).toInt();
        item.isCategory = false;
        
        m_items.append(item);
    }
    
    // Build parent-child relationships
    buildHierarchy();
    
    endResetModel();
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
            // Find parent category index
            int parentIndex = findItemIndex(m_items[i].parentId, true);
            if (parentIndex != -1) {
                m_items[parentIndex].children.append(i);
            }
        }
    }
}

int CategoryHierModel::findItemIndex(int id, bool isCategory) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id && m_items[i].isCategory == isCategory) {
            return i;
        }
    }
    return -1;
}

void CategoryHierModel::refresh()
{
    loadData();
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
    
    // Child item (subcategory)
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
        return QModelIndex();  // Top-level item has no parent
    
    // Find parent item index
    int parentIndex = findItemIndex(parentId, true);
    if (parentIndex == -1)
        return QModelIndex();
    
    // Find parent's row among root items or its parent's children
    int row = -1;
    if (m_items[parentIndex].parentId == -1) {
        // Parent is a root category
        row = m_rootItems.indexOf(parentIndex);
    } else {
        // Parent is a subcategory (should not happen, but handle gracefully)
        int grandParentIndex = findItemIndex(m_items[parentIndex].parentId, true);
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
        // Top level - count root items
        return m_rootItems.size();
    }
    
    int parentIndex = parent.internalId();
    if (parentIndex < 0 || parentIndex >= m_items.size())
        return 0;
    
    // Return number of children for this item
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
            return item.name;
            
        case Qt::ToolTipRole:
            if (!item.description.isEmpty())
                return item.description;
            return item.name;
            
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
        // This is a root category
        row = m_rootItems.indexOf(itemIndex);
    } else {
        // This is a subcategory - find its row within parent's children
        int parentIndex = findItemIndex(item.parentId, true);
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

int CategoryHierModel::getParentCategoryId(const QModelIndex &index) const
{
    if (!index.isValid())
        return -1;
    
    int itemIndex = index.internalId();
    if (itemIndex < 0 || itemIndex >= m_items.size())
        return -1;
    
    return m_items[itemIndex].parentId;
}