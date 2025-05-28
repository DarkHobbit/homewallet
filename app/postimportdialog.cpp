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
#include <QPushButton>

#include "helpers.h"
#include "postimportdialog.h"
#include "ui_postimportdialog.h"

PostImportDialog::PostImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PostImportDialog),
    mSet(0)
{
    ui->setupUi(this);
}

PostImportDialog::~PostImportDialog()
{
    delete ui;
}

void PostImportDialog::setData(ImpCandidates* _cands)
{
    cands = _cands;
    mSet = new ImportModelSet(cands, this);
    ui->tableExpenses->setModel(mSet->mdlExpense);
    ui->tableIncomes->setModel(mSet->mdlIncome);
    ui->lbStat->setText(mSet->stat());
    setOkAccessibility();
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

void PostImportDialog::setOkAccessibility()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(mSet->canImport());
}
