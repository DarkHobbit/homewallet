/* Home Wallet
 *
 * Module: Dialog for export details
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
#include "globals.h"
#include "exportdialog.h"
#include "ui_exportdialog.h"

ExportDialog::ExportDialog(FormatFactory* _factory, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExportDialog)
    , factory(_factory)
    , currentFormat(0)
{
    ui->setupUi(this);
    ui->cbFormat->addItems(factory->supportedFilters(QIODevice::WriteOnly, false));
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::changeEvent(QEvent *e)
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

void ExportDialog::on_cbFormat_currentIndexChanged(int)
{
    currentFormat = factory->formatByFilter(ui->cbFormat->currentText());
    if (!currentFormat) {
        QMessageBox::critical(0, S_ERROR, S_INTERNAL_ERR);
        return;
    }
    setSubTypeEnabled(ui->cbAccounts, FileFormat::AccountsInBrief);
    setSubTypeEnabled(ui->cbAliases, FileFormat::Aliases);
    setSubTypeEnabled(ui->cbCategories, FileFormat::Categories);
    setSubTypeEnabled(ui->cbExpenses, FileFormat::Expenses);
    setSubTypeEnabled(ui->cbIncomes, FileFormat::Incomes);
    setSubTypeEnabled(ui->cbTransfer, FileFormat::Transfer);
    // TODO other sc
}

void ExportDialog::setSubTypeEnabled(QCheckBox *cb, FileFormat::SubType typeFlag)
{
    cb->setEnabled(currentFormat->supportedExportSubTypes().testFlag(typeFlag));
    if (!cb->isEnabled())
        cb->setChecked(false);
}

void ExportDialog::selectSubTypeIfEnabled(QCheckBox *cb)
{
    if (cb->isEnabled())
        cb->setChecked(true);
}


void ExportDialog::on_btnSelectAll_clicked()
{
    selectSubTypeIfEnabled(ui->cbAccounts);
    selectSubTypeIfEnabled(ui->cbAliases);
    selectSubTypeIfEnabled(ui->cbCategories);
    selectSubTypeIfEnabled(ui->cbAliases);
    selectSubTypeIfEnabled(ui->cbIncomes);
    selectSubTypeIfEnabled(ui->cbTransfer);
    // TODO other sc
}

void ExportDialog::on_btnUnselectAll_clicked()
{
    ui->cbAccounts->setChecked(false);
    ui->cbAliases->setChecked(false);
    ui->cbCategories->setChecked(false);
    ui->cbAliases->setChecked(false);
    ui->cbIncomes->setChecked(false);
    ui->cbTransfer->setChecked(false);
    // TODO other sc
}


