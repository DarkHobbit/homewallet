/* Home Wallet
 *
 * Module: Dialog for adding default units for various expense/income subcategories
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef ADDDEFAULTUNITDIALOG_H
#define ADDDEFAULTUNITDIALOG_H

#include <QDialog>

#include "hwdatabase.h"

namespace Ui {
class AddDefaultUnitDialog;
}

class AddDefaultUnitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddDefaultUnitDialog(QWidget *parent, HwDatabase* _db);
    ~AddDefaultUnitDialog();
    void addDefaultUnit(int idSubCat, const QString& subCatName, bool isExpense);

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_rbRecentUsed_clicked();

    void on_rbOther_clicked();

private:
    Ui::AddDefaultUnitDialog *ui;
    HwDatabase* db;
    void enableCombos();
};

#endif // ADDDEFAULTUNITDIALOG_H
