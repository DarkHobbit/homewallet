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

#include <QMessageBox>

#include "aliasdialog.h"
#include "globals.h"
#include "helpers.h"
#include "ui_aliasdialog.h"

AliasDialog::AliasDialog(QWidget *parent, HwDatabase* _db) :
    QDialog(parent),
    ui(new Ui::AliasDialog), db(_db)
{
    ui->setupUi(this);
}

AliasDialog::~AliasDialog()
{
    delete ui;
}

void AliasDialog::addAlias(HwDatabase::AliasType alType, const QString &alS)
{
    ui->leAlias->setText(alS);
    setType(alType);
    exec();
    if (result()==QDialog::Accepted) {
        // TODO сделать некий intExecQuery() на базе GenericDatabase::execQuery
        // (интерактивный, с выводом ошибок prepare() и execQuery()
        bool res = false;
        int idDict = getComboCurrentId(ui->cbDict);
        switch (alType) {
        case HwDatabase::Account:
            // TODO
            break;
        case HwDatabase::Currency:
            // TODO
            break;
        case HwDatabase::Unit:
            res = db->addAlias(ui->leAlias->text(), ui->teToDescr->toPlainText(), 0, 0, idDict);
            break;
            // TODO category, subcategory, alias
        default:
            return;
        }
        if (!res)
            QMessageBox::critical(0, S_ERROR, db->lastError());
    }
}

void AliasDialog::setSubdictEnabled(bool state)
{
    ui->lbSubDict->setEnabled(state);
    ui->cbSubDict->setEnabled(state);
}

void AliasDialog::setType(HwDatabase::AliasType alType)
{
    GenericDatabase::DictColl coll;
    switch (alType) {
    case HwDatabase::Account:
        setWindowTitle(tr("Account alias"));
        setSubdictEnabled(false);
        break;
    case HwDatabase::Currency:
        setWindowTitle(tr("Currency alias"));
        setSubdictEnabled(false);
        break;
    case HwDatabase::Unit:
        setWindowTitle(tr("Unit alias"));
        setSubdictEnabled(false);
        ui->lbDict->setText(tr("Unit"));
        db->collectDict(coll, "hw_unit");
        break;
        // TODO category, subcategory, alias
    default:
        return;
    }
    fillComboByDict(ui->cbDict, coll, false);
    if (ui->cbSubDict->isEnabled()) {
        // TODO inc/exp subcat
    }
}
