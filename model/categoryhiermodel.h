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
#include <QDate>

class HwDatabase;

// Structure for storing category/subcategory/operation data
struct CategoryItem {
    int id;                 // Record ID
    int parentId;           // Parent ID (-1 for root categories)
    QString name;           // Display name
    QString description;    // Description (for categories/subcategories)
    int defaultUnitId;      // Default unit ID (for subcategories)
    bool isCategory;        // true - category, false - subcategory or operation
    bool isOperation;       // true - operation (income/expense), false - category/subcategory
    double quantity;        // Quantity (for operations)
    int amount;             // Amount in low units (cents/kopeks, for operations)
    QDate operationDate;    // Operation date
    QVector<int> children;  // Child item indices in flat list
    
    CategoryItem() : id(-1), parentId(-1), defaultUnitId(-1), 
                     isCategory(true), isOperation(false), 
                     quantity(0), amount(0) {}
};

class CategoryHierModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    // isExpense = true  -> work with expenses (hw_ex_cat, hw_ex_subcat, hw_ex_op)
    // isExpense = false -> work with incomes (hw_in_cat, hw_in_subcat, hw_in_op)
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
    
    // Enable/disable operation display (third level)
    void setOperationShow(bool show);
    bool isOperationShow() const { return m_showOperations; }

    // Get category/subcategory/operation ID by model index
    int getId(const QModelIndex &index) const;
    
    // Check type of item
    bool isCategory(const QModelIndex &index) const;
    bool isSubcategory(const QModelIndex &index) const;
    bool isOperation(const QModelIndex &index) const;
    
    // Get parent category ID for a subcategory or operation
    int getParentCategoryId(const QModelIndex &index) const;
    
    // Get parent subcategory ID for an operation
    int getParentSubcategoryId(const QModelIndex &index) const;
    
    // Get operation data
    double getQuantity(const QModelIndex &index) const;
    int getAmount(const QModelIndex &index) const;
    QDate getOperationDate(const QModelIndex &index) const;

private:
    bool m_isExpense;           // true - expenses, false - incomes
    HwDatabase* m_db;           // pointer to database object (not owner)
    QVector<CategoryItem> m_items;  // flat list of all items
    QVector<int> m_rootItems;   // indices of root (top-level) items
    bool m_showOperations;      // show third level (operations)
    
    void loadData();            // load data from database
    void loadCategories();      // load categories (level 0)
    void loadSubcategories();   // load subcategories (level 1)
    void loadOperations();      // load operations (level 2)
    void buildHierarchy();      // build parent-child relationships
    QModelIndex indexFromItem(int itemIndex, int column) const;
    int findItemIndex(int id, bool isCategory, bool isSubcategory = false) const;
    void clearData();
    
    // Format amount from low units (cents/kopeks) to main units with 2 decimal places
    QString formatAmount(int amountInLowUnits) const;
    
    // Get child count for display (categories: subcategories count, subcategories: operations count)
    int getChildCountForDisplay(const CategoryItem &item) const;
};

#endif // CATEGORYHIERMODEL_H