/* Home Wallet
 *
 * Module: Dialog for dictionary record add/edit
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

#include "simpledictdialog.h"
#include "ui_simpledictdialog.h"
#include "hwdatabase.h"
#include "globals.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

SimpleDictDialog::SimpleDictDialog(const QString &tableName, const QString &entityName, bool isEdit, HwDatabase* db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SimpleDictDialog),
    m_db(db),
    m_tableName(tableName),
    m_isEdit(isEdit),
    m_currentId(-1)
{
    ui->setupUi(this);
    
    // Setup window title based on operation mode
    QString action = m_isEdit ? S_ACT_EDIT : S_ACT_ADD;
    setWindowTitle(QString("%1 %2").arg(action, entityName));
}

SimpleDictDialog::~SimpleDictDialog()
{
    delete ui;
}

void SimpleDictDialog::setEditData(int id, const QString &name, const QString &description)
{
    m_currentId = id;
    ui->lineEditName->setText(name);
    ui->lineEditDescription->setText(description);
}

QString SimpleDictDialog::getName() const
{
    return ui->lineEditName->text().trimmed();
}

QString SimpleDictDialog::getDescription() const
{
    return ui->lineEditDescription->text().trimmed();
}

bool SimpleDictDialog::isDuplicate() const
{
    QString currentName = getName();
    
    QSqlQuery query;
    query.prepare(QString("SELECT id FROM %1 WHERE name = :name").arg(m_tableName));
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

bool SimpleDictDialog::validateInput()
{
    if (getName().isEmpty()) {
        QMessageBox::critical(nullptr, S_ERROR, S_EMPTY_NAME);
        return false;
    }
    
    // Check for duplicate
    if (isDuplicate()) {
        QMessageBox::critical(nullptr, S_ERROR,
            tr("Record with this name already exists in %1.").arg(m_tableName));
        return false;
    }
    
    return true;
}

int SimpleDictDialog::addRecord(const QString &defaultName)
{
    // Set default name if provided and no existing data is being edited
    if (!defaultName.isEmpty() && m_currentId == -1 && ui->lineEditName->text().isEmpty()) {
        ui->lineEditName->setText(defaultName);
    }
    
    // Execute dialog and check result
    if (exec() != QDialog::Accepted)
        return -1;  // User canceled
    
    // Validate input
    if (!validateInput())
        return -1;

    // Add new record
    QSqlQuery query;
    query.prepare(QString("INSERT INTO %1 (name, descr) VALUES (:name, :descr)").arg(m_tableName));
    query.bindValue(":name", getName());
    query.bindValue(":descr", getDescription());
    
    if (!query.exec()) {
        qDebug() << "Failed to add record to" << m_tableName << ":" << query.lastError().text();
        QMessageBox::critical(nullptr, S_ERROR, 
            tr("Failed to add record to %1.").arg(m_tableName));
        return -1;
    }
    
    return query.lastInsertId().toInt();
}
