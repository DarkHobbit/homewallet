/* Home Wallet
 *
 * Module: Dialog for aliases add/edit
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef ALIASDIALOG_H
#define ALIASDIALOG_H

#include <QDialog>
#include <QString>
#include "hwdatabase.h"

namespace Ui {
class AliasDialog;
}

class AliasDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AliasDialog(QWidget *parent, HwDatabase* _db);
    ~AliasDialog();
    void addAlias(HwDatabase::AliasType alType, const QString& alS);

private:
    Ui::AliasDialog *ui;
    HwDatabase* db;
    void setSubdictEnabled(bool state);
    void setType(HwDatabase::AliasType alType);
};

#endif // ALIASDIALOG_H
