/* Home Wallet
 *
 * Module: Dialog for interactive import
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>

#include "adddefaultunitdialog.h"
#include "aliasdialog.h"
#include "formats/commonexpimpdef.h"
#include "globals.h"
#include "helpers.h"
#include "postimportdialog.h"
#include "ui_postimportdialog.h"

PostImportDialog::PostImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PostImportDialog),
    mSet(0), activeModel(0), activeView(0)
{
    ui->setupUi(this);
    ui->tableExpenses->insertAction(0, ui->actExpCandState);
    ui->tableExpenses->insertAction(0, ui->actAddAlias);
    ui->tableExpenses->insertAction(0, ui->actAddDefaultUnit);
}

PostImportDialog::~PostImportDialog()
{
    delete ui;
}

void PostImportDialog::setData(InteractiveFormat* _intFile, HwDatabase* _db)
{
    intFile = _intFile;
    db = _db;
    mSet = new ImportModelSet(&_intFile->candidates, this);
    ui->tableExpenses->setModel(mSet->mdlExpense);
    ui->tableIncomes->setModel(mSet->mdlIncome);
    updateStat();
    ui->btnAddAlias->setShortcut(Qt::Key_Insert);
    /*
    connect(mSet->mdlExpense,
            SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector)),
            this, SLOT(setAddAliasAccessibility()));
    // TODO transfer, debt, cred
    */
}

void PostImportDialog::showEvent(QShowEvent*)
{
    updateTableConfig(ui->tableExpenses);
    updateTableConfig(ui->tableIncomes);
    updateOneView(ui->tableExpenses, false);
    updateOneView(ui->tableIncomes, false);
    ui->tableExpenses->resizeColumnsToContents();
    ui->tableIncomes->resizeColumnsToContents();
    // Try to reuce source string
    ui->tableExpenses->setColumnWidth(1, ui->tableExpenses->columnWidth(5));
    ui->tableIncomes->setColumnWidth(1, ui->tableIncomes->columnWidth(5));
}

void PostImportDialog::updateView()
{
    intFile->analyzeCandidates(*db);
    activeModel->update();
    updateStat();
    // TODO if works, write ImportModelSet::updateModels() and move to
    // (for accs, units, currs need updates all)
}

void PostImportDialog::updateStat()
{
    ui->lbStat->setText(mSet->stat());
    setOkAccessibility();
    //setAddAliasAccessibility();
}

PostImportDialog::ActiveTab PostImportDialog::activeTab()
{
    QWidget* curW = ui->tabWidget->currentWidget();
    if (curW==ui->tabExpenses) {
        activeModel = mSet->mdlExpense;
        activeView = ui->tableExpenses;
        return atExpenses;
    }
    else if (curW==ui->tabIncomes) {
        activeModel = mSet->mdlIncome;
        activeView = ui->tableIncomes;
        return atIncomes;
    }
    // TODO other tabs
    else {
        activeModel = mSet->mdlTransfer;
        activeView = 0; //===>
        return atTransfer;
    }
}

void PostImportDialog::setOkAccessibility()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(mSet->canImport());
}

void PostImportDialog::setAddAliasAccessibility()
{
    activeTab();
    if (!activeModel || !activeView)
        return;
    int r = activeView->currentIndex().row();
    ui->btnAddAlias->setEnabled(activeModel->cand(r)->needAddAlias());
}

void PostImportDialog::on_actExpCandState_triggered()
{
    activeTab();
    int r = activeView->currentIndex().row();
    ImpRecCandidate* c = activeModel->cand(r);
    QMessageBox::information(0, S_INFORM,
        tr("Row %1: source line %2\%3\nState: %4")
        .arg(r).arg(c->lineNumber).arg(c->source)
        .arg(c->stateString[c->state]));
}

void PostImportDialog::on_btnAddAlias_clicked()
{
    // Select alias type
    HwDatabase::AliasType alType;
    QString alS;
    activeTab();
    int r = activeView->currentIndex().row();
    ImpRecCandidate* c = activeModel->cand(r);
    switch(c->state) {
    case ImpRecCandidate::UnknownAccount:
        alType = HwDatabase::Account;
        alS = c->accName;
        break;
    case ImpRecCandidate::UnknownCurrency:
        alType = HwDatabase::Currency;
        alS = c->currName;
        break;
    case ImpRecCandidate::UnknownUnit:
        alType = HwDatabase::Unit;
        alS = c->unitName;
        break;
        // TODO category, subcategory (check c->type)
    default:
        QMessageBox::critical(0, S_ERROR,
            tr("No unknown aliases in this row"));
        return;
    }
    // Ask user
    AliasDialog* d = new AliasDialog(0, db);
    d->addAlias(alType, alS);
    delete d;
    // Update candidates and its view
    updateView();
}

void PostImportDialog::on_actAddAlias_triggered()
{
    on_btnAddAlias_clicked();
}

void PostImportDialog::on_actAddDefaultUnit_triggered()
{
    activeTab();
    int r = activeView->currentIndex().row();
    ImpRecCandidate* c = activeModel->cand(r);
    if (c->subcatName.isEmpty()) {
        QMessageBox::critical(0, S_ERROR, S_ERR_NO_SUBCAT);
        return;
    }
    if (!c->unitName.isEmpty()) {
        QMessageBox::critical(0, S_ERROR, S_ERR_UNIT_ISNT_EMPTY);
        return;
    }
    if (c->type!=ImpRecCandidate::Income && c->type!=ImpRecCandidate::Expense) {
        QMessageBox::critical(0, S_ERROR, S_ERR_NOT_INC_NOT_EXP);
        return;
    }
    auto d = new AddDefaultUnitDialog(0, db);
    d->addDefaultUnit(c->idSubcat, c->subcatName, c->type==ImpRecCandidate::Expense);
    // Update candidates and its view
    updateView();
}

