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
    ui(new Ui::AliasDialog),
    db(_db), alType(HwDatabase::Account)
{
    ui->setupUi(this);
}

AliasDialog::~AliasDialog()
{
    delete ui;
}

void AliasDialog::addAlias(HwDatabase::AliasType _alType, const QString &alS, const QString &alHint)
{
    alType = _alType;
    ui->leAlias->setText(alS);
    setType(alType);
    // For subcategories, set category candidate
    if (!alHint.isEmpty()) {
        int hintPos = ui->cbDict->findText(alHint);
        if (hintPos>-1)
            ui->cbDict->setCurrentIndex(hintPos);
    }
    // Tune combo to similar text (after hint set!)
    if (ui->cbSubDict->isEnabled())
        setSimilarComboText(ui->cbSubDict, alS);
    else
        setSimilarComboText(ui->cbDict, alS);
    // Drive!
    exec();
    if (result()==QDialog::Accepted) {
        // TODO сделать некий intExecQuery() на базе GenericDatabase::execQuery
        // (интерактивный, с выводом ошибок prepare() и execQuery()
        bool res = false;
        int idDict = ui->cbSubDict->isEnabled() ? getComboCurrentId(ui->cbSubDict)
                                                : getComboCurrentId(ui->cbDict);
        QString al = ui->leAlias->text();
        QString des = ui->teToDescr->toPlainText();
        res = db->addAlias(al, des, alType, idDict);
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
    ui->teToDescr->setEnabled(false);
    switch (alType) {
    case HwDatabase::Account:
        setWindowTitle(tr("Account alias"));
        setSubdictEnabled(false);
        ui->lbDict->setText(tr("Account"));
        ui->lbSubDict->setText(tr(""));
        db->collectDict(coll, "hw_account");
        break;
    case HwDatabase::Currency:
        setWindowTitle(tr("Currency alias"));
        setSubdictEnabled(false);
        ui->lbDict->setText(tr("Currency"));
        ui->lbSubDict->setText(tr(""));
        db->collectDict(coll, "hw_currency", "full_name");
        break;
    case HwDatabase::Unit:
        setWindowTitle(tr("Unit alias"));
        setSubdictEnabled(false);
        ui->lbDict->setText(tr("Unit"));
        ui->lbSubDict->setText(tr(""));
        db->collectDict(coll, "hw_unit");
        break;
    case HwDatabase::IncomeCat:
    case HwDatabase::ExpenseCat: {
        setWindowTitle(tr("Category alias"));
        setSubdictEnabled(false);
        ui->lbDict->setText(tr("Category"));
        ui->lbSubDict->setText(tr(""));
        if (alType==HwDatabase::IncomeSubCat)
            db->collectDict(coll, "hw_in_cat");
        else
            db->collectDict(coll, "hw_ex_cat");
        break;
    }
    case HwDatabase::IncomeSubCat:
    case HwDatabase::ExpenseSubCat: {
        setWindowTitle(tr("Subcategory alias"));
        setSubdictEnabled(true);
        ui->teToDescr->setEnabled(true);
        ui->lbDict->setText(tr("Category"));
        ui->lbSubDict->setText(tr("Subcategory"));
        if (alType==HwDatabase::IncomeSubCat) {
            db->collectDict(coll, "hw_in_cat");
            db->collectSubDict(coll, subCats, "hw_in_subcat", "name", "id", "id_icat");
        }
        else {
            db->collectDict(coll, "hw_ex_cat");
            db->collectSubDict(coll, subCats, "hw_ex_subcat", "name", "id", "id_ecat");
        }
        break;
    }
    default:
        return;
    }
    fillComboByDict(ui->cbDict, coll, false);
}

void AliasDialog::on_cbDict_currentTextChanged(const QString &catName)
{
    ui->cbSubDict->clear();
    if (!catName.isEmpty() && !subCats.isEmpty())
        fillComboByDict(ui->cbSubDict, subCats[catName], false);
}


void AliasDialog::on_btnCopyToDescr_clicked()
{
    QString prevText = ui->teToDescr->toPlainText();
    if (!prevText.isEmpty())
        prevText += " ";
    ui->teToDescr->setText(prevText+ui->leAlias->text());
}
