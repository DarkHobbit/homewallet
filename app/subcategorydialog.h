/* Home Wallet
 *
 * Module: Dialog for subcategories add/edit
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

#ifndef SUBCATEGORYDIALOG_H
#define SUBCATEGORYDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlRecord>

namespace Ui {
class SubCategoryDialog;
}

class HwDatabase;

class SubCategoryDialog : public QDialog
{
    Q_OBJECT

public:
    // isExpense = true  -> work with hw_ex_subcat (expenses)
    // isExpense = false -> work with hw_in_subcat (incomes)
    // isEdit = true -> edit existing subcategory, false -> add new subcategory
    // db - pointer to HwDatabase object (cannot be nullptr)
    explicit SubCategoryDialog(bool isExpense, bool isEdit, HwDatabase* db, QWidget *parent = nullptr);
    ~SubCategoryDialog();

    // Set data for editing existing subcategory
    void setEditData(int id, int categoryId, const QString &name, 
                     const QString &description, int defaultUnitId);

    // Getters for entered data
    int getCategoryId() const;
    QString getName() const;
    QString getDescription() const;
    int getDefaultUnitId() const;
    
    // Add subcategory to database (similar to AliasDialog::addAlias())
    // Shows dialog, validates input, and performs insertion
    // If defaultName is provided, it will be pre-filled in the name field
    // Returns ID of added record or -1 on error or cancellation
    int addSubCategory(const QString &defaultName = QString());

private:
    Ui::SubCategoryDialog *ui;
    HwDatabase* m_db;            // pointer to database object (not owner)
    bool m_isExpense;            // true - expenses, false - incomes
    bool m_isEdit;               // true - edit mode, false - add mode
    int m_currentId;             // ID of edited record (-1 for new)
    int m_originalCategoryId;    // for duplicate check

    void loadCategories();       // load categories from hw_in_cat or hw_ex_cat
    void loadUnits();            // load units from hw_unit
    bool isDuplicate() const;    // check for duplicate (unique pair category_id + name)
    bool validateInput();        // validate input values, show error messages if invalid
};

#endif // SUBCATEGORYDIALOG_H