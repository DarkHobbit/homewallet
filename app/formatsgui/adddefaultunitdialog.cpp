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

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "globals.h"
#include "formats/commonexpimpdef.h"

#include "adddefaultunitdialog.h"
#include "helpers.h"
#include "ui_adddefaultunitdialog.h"

AddDefaultUnitDialog::AddDefaultUnitDialog(QWidget *parent, HwDatabase* _db)
    : QDialog(parent)
    , ui(new Ui::AddDefaultUnitDialog), db(_db)
{
    ui->setupUi(this);
}

AddDefaultUnitDialog::~AddDefaultUnitDialog()
{
    delete ui;
}
#include <iostream>
void AddDefaultUnitDialog::addDefaultUnit(int idSubCat, const QString& subCatName, bool isExpense)
{
    if (!isExpense) {
        QMessageBox::critical(0, S_ERROR, "For income categories, default units are not implemented. Contact author");
        return;
    }
    QSqlQuery q(db->sqlDbRef());
    GUI_DB_CHK(q.prepare(SQL_GET_DEF_EXP_UNIT), q)
    q.bindValue(":id", idSubCat);
    GUI_DB_CHK(q.exec(), q)
    // If already exists...
    if (db->queryRecCount(q)>0) {
        q.first();
        int res = QMessageBox::question(0, S_CONFIRM,
            S_CONFIRM_DEF_UNIT_EXIST.arg(subCatName).arg(q.value(0).toString()));
        if (res==QMessageBox::No)
            return;
    }
    setWindowTitle(windowTitle()+subCatName);
    // Recent used units
    QSqlQuery qRecent;
    GUI_DB_CHK(qRecent.prepare(SQL_UNIT_COUNTS), qRecent)
    qRecent.bindValue(":id_sc", idSubCat);
    GUI_DB_CHK(qRecent.exec(), qRecent)
    if (qRecent.first()) {
        ui->cbRecentUsed->clear();
        do {
            ui->cbRecentUsed->addItem(QString("%1 (%2)")
                .arg(qRecent.value(0).toString())
                .arg(qRecent.value(1).toInt()));
        } while (qRecent.next());
    }
    // Other
    QSqlQuery qAllUnits;
    GUI_DB_CHK(qAllUnits.prepare(SQL_ALL_UNITS), qAllUnits)
    GUI_DB_CHK(qAllUnits.exec(), qAllUnits)
    if (qAllUnits.first()) {
        ui->cbOther->clear();
        do {
            ui->cbOther->addItem(qAllUnits.value(0).toString());
        } while (qAllUnits.next());
    }
    if (db->queryRecCount(qRecent)>0)
        ui->rbRecentUsed->setChecked(true);
    else if (db->queryRecCount(qAllUnits)>0)
        ui->rbOther->setChecked(true);
    else {
        QMessageBox::critical(0, S_ERROR, S_ERR_NO_UNIT);
        return;
    }
    enableCombos();
    exec();
    if (result()==QDialog::Accepted) {
        QString unit;
        if (ui->rbRecentUsed->isChecked()) {
            unit = ui->cbRecentUsed->currentText();
            int quaPos = unit.indexOf(" (");
            if (quaPos>=0)
                unit = unit.left(quaPos);
        }
        else
            unit = ui->cbOther->currentText();
        qAllUnits.first();
        do {
            if (unit==qAllUnits.value(0).toString()) {
                int idUn = qAllUnits.value(1).toInt();
                QSqlQuery qSetUnit;
                GUI_DB_CHK(qSetUnit.prepare(SQL_SET_DEF_EXP_UNIT), qSetUnit)
                qSetUnit.bindValue(":id_sc", idSubCat);
                qSetUnit.bindValue(":id_un", idUn);
                GUI_DB_CHK(qSetUnit.exec(), qSetUnit)
                return;
            }
        } while (qAllUnits.next());
        QMessageBox::critical(0, S_ERROR, S_INTERNAL_ERR);
    }
}

void AddDefaultUnitDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void AddDefaultUnitDialog::enableCombos()
{
    ui->cbRecentUsed->setEnabled(ui->rbRecentUsed->isChecked());
    ui->cbOther->setEnabled(ui->rbOther->isChecked());
}

void AddDefaultUnitDialog::on_rbRecentUsed_clicked()
{
    enableCombos();
}

void AddDefaultUnitDialog::on_rbOther_clicked()
{
    enableCombos();
}
