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

#ifndef CATEGORYHIERMODEL_H
#define CATEGORYHIERMODEL_H

#include <QAbstractItemModel>
#include <QSqlDatabase>
#include <QVector>

class HwDatabase;

// Structure for storing category/subcategory data
struct CategoryItem {
    int id;                 // category or subcategory ID
    int parentId;           // parent category ID (-1 for top-level categories)
    QString name;           // display name
    QString description;    // optional description
    int defaultUnitId;      // default unit ID (for subcategories, -1 if not set)
    bool isCategory;        // true for category, false for subcategory
    QVector<int> children;  // indices of child items in the flat list
    
    CategoryItem() : id(-1), parentId(-1), defaultUnitId(-1), isCategory(true) {}
};

class CategoryHierModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    // isExpense = true  -> work with expense categories (hw_ex_cat, hw_ex_subcat)
    // isExpense = false -> work with income categories (hw_in_cat, hw_in_subcat)
    explicit CategoryHierModel(bool isExpense, HwDatabase* db, QObject *parent = nullptr);
    ~CategoryHierModel();

    // QAbstractItemModel interface implementation
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Refresh model data from database
    void refresh();

    // Get category/subcategory ID by model index
    int getId(const QModelIndex &index) const;
    
    // Check if index points to a category
    bool isCategory(const QModelIndex &index) const;
    
    // Get parent category ID for a subcategory
    int getParentCategoryId(const QModelIndex &index) const;

private:
    bool m_isExpense;           // true - expenses, false - incomes
    HwDatabase* m_db;           // pointer to database object (not owner)
    QVector<CategoryItem> m_items;  // flat list of all items
    QVector<int> m_rootItems;   // indices of root (top-level) items
    
    void loadData();            // load data from database
    void buildHierarchy();      // build parent-child relationships
    QModelIndex indexFromItem(int itemIndex, int column) const;
    int findItemIndex(int id, bool isCategory) const;
};

#endif // CATEGORYHIERMODEL_H