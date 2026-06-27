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

#ifndef SIMPLEDICDIALOG_H
#define SIMPLEDICDIALOG_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class SimpleDictDialog;
}

class HwDatabase;

class SimpleDictDialog : public QDialog
{
    Q_OBJECT

public:
    // tableName - name of dictionary table with columns: id, name, descr
    // isEdit = true -> edit existing record, false -> add new record
    // db - pointer to HwDatabase object (cannot be nullptr)
    explicit SimpleDictDialog(const QString &tableName, const QString &entityName, bool isEdit, HwDatabase* db, QWidget *parent = nullptr);
    ~SimpleDictDialog();

    // Set data for editing existing record
    void setEditData(int id, const QString &name, const QString &description);

    // Getters for entered data
    QString getName() const;
    QString getDescription() const;
    
    // Add record to database
    // Shows dialog, validates input, and performs insertion
    // If defaultName is provided, it will be pre-filled in the name field
    // Returns ID of added record or -1 on error or cancellation
    int addRecord(const QString &defaultName = QString());

private:
    Ui::SimpleDictDialog *ui;
    HwDatabase* m_db;            // pointer to database object (not owner)
    QString m_tableName;         // name of dictionary table
    bool m_isEdit;               // true - edit mode, false - add mode
    int m_currentId;             // ID of edited record (-1 for new)

    bool isDuplicate() const;    // check for duplicate (unique name)
    bool validateInput();        // validate input values, show error messages if invalid
};

#endif // SIMPLEDICDIALOG_H
