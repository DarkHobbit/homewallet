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

#include "subcategorydialog.h"
#include "ui_subcategorydialog.h"
#include "hwdatabase.h"
#include "globals.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

SubCategoryDialog::SubCategoryDialog(bool isExpense, bool isEdit, HwDatabase* db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubCategoryDialog),
    m_db(db),
    m_isExpense(isExpense),
    m_isEdit(isEdit),
    m_currentId(-1),
    m_originalCategoryId(-1)
{
    ui->setupUi(this);
    // Setup window title based on operation mode and type
    QString action = m_isEdit ? S_ACT_EDIT : S_ACT_ADD;
    QString type = m_isExpense ? tr("Expense Subcategory") : tr("Income Subcategory");
    setWindowTitle(QString("%1 %2").arg(action, type));

    loadCategories();
    loadUnits();
}

SubCategoryDialog::~SubCategoryDialog()
{
    delete ui;
}

void SubCategoryDialog::loadCategories()
{
    QString tableName = m_isExpense ? "hw_ex_cat" : "hw_in_cat";
    QSqlQuery query(QString("SELECT id, name FROM %1 ORDER BY name").arg(tableName));
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        ui->comboCategory->addItem(name, id);
    }
}

void SubCategoryDialog::loadUnits()
{
    QSqlQuery query("SELECT id, name FROM hw_unit ORDER BY name");
    ui->comboDefaultUnit->addItem(tr(""), QVariant());  // empty value (NULL)
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        ui->comboDefaultUnit->addItem(name, id);
    }
}

void SubCategoryDialog::setEditData(int id, int categoryId, const QString &name,
                                    const QString &description, int defaultUnitId)
{
    m_currentId = id;
    m_originalCategoryId = categoryId;

    // Select category in combobox
    int index = ui->comboCategory->findData(categoryId);
    if (index >= 0)
        ui->comboCategory->setCurrentIndex(index);

    ui->lineEditName->setText(name);
    ui->lineEditDescription->setText(description);

    // Select default unit
    index = ui->comboDefaultUnit->findData(defaultUnitId);
    if (index >= 0)
        ui->comboDefaultUnit->setCurrentIndex(index);
    else
        ui->comboDefaultUnit->setCurrentIndex(0); // empty value
}

int SubCategoryDialog::getCategoryId() const
{
    return ui->comboCategory->currentData().toInt();
}

QString SubCategoryDialog::getName() const
{
    return ui->lineEditName->text().trimmed();
}

QString SubCategoryDialog::getDescription() const
{
    return ui->lineEditDescription->text().trimmed();
}

int SubCategoryDialog::getDefaultUnitId() const
{
    QVariant data = ui->comboDefaultUnit->currentData();
    return data.isValid() ? data.toInt() : -1;  // -1 means NULL
}

bool SubCategoryDialog::isDuplicate() const
{
    QString tableName = m_isExpense ? "hw_ex_subcat" : "hw_in_subcat";
    QString idCategoryField = m_isExpense ? "id_ecat" : "id_icat";
    int currentCategoryId = getCategoryId();
    QString currentName = getName();

    QSqlQuery query;
    query.prepare(QString("SELECT id FROM %1 WHERE %2 = :catId AND name = :name")
                  .arg(tableName, idCategoryField));
    query.bindValue(":catId", currentCategoryId);
    query.bindValue(":name", currentName);
    
    if (!query.exec()) {
        qDebug() << "Duplicate check failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        int existingId = query.value(0).toInt();
        // If this is edit operation and found record is the same one, it's not a duplicate
        if (m_currentId == existingId)
            return false;
        return true;
    }
    return false;
}

bool SubCategoryDialog::validateInput()
{
    if (getName().isEmpty()) {
        QMessageBox::critical(nullptr, S_ERROR, S_EMPTY_NAME);
        return false;
    }
    // Check for duplicate
    if (isDuplicate()) {
        QMessageBox::critical(nullptr, S_ERROR,
            tr("Subcategory with this name already exists in the selected category."));
        return false;
    }
    
    return true;
}
#include <iostream>
int SubCategoryDialog::addSubCategory(const QString &defaultName)
{
    // Set default name if provided and no existing data is being edited
    if (!defaultName.isEmpty() && m_currentId == -1 && ui->lineEditName->text().isEmpty()) {
        ui->lineEditName->setText(defaultName);
    }
    
    // Execute dialog and check result
    if (exec() != QDialog::Accepted)
        return -1;  // User canceled
    std::cout << "asc1" << std::endl;
    // Validate input
    if (!validateInput())
        return -1;

    std::cout << "asc2" << std::endl;
    // Add new subcategory
    int newId = -1;
    
    if (m_isExpense) {
        std::cout << "asc3" << std::endl;
        newId = m_db->addExpenseSubCategory(getCategoryId(), getName(),
            getDescription(), getDefaultUnitId());
    } else {
        newId = m_db->addIncomeSubCategory(getCategoryId(), getName(), 
            getDescription(), getDefaultUnitId());
    }
    
    std::cout << "asc4" << std::endl;
    if (newId == -1) {
        QString errorMsg = m_db->lastError();
        if (!errorMsg.isEmpty()) {
            QMessageBox::critical(nullptr, S_ERROR, errorMsg);
        }
        return -1;
    }
    
    return newId;
}