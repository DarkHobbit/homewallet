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

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include "configmanager.h"
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
    QString path = configManager.lastExportedFile();
    ui->lePath->setText(configManager.lastExportedFile());
    if (QFileInfo(path).isDir())
        ui->rbDirectory->setChecked(true);
    else
        ui->rbSingleFile->setChecked(true);
    ui->cbFormat->addItems(factory->supportedFilters(QIODevice::WriteOnly, false));

    insertAction(0, ui->actionSelectAll);
    connect(ui->actionSelectAll, SIGNAL(triggered(bool)), this, SLOT(on_btnSelectAll_clicked()));
    insertAction(0, ui->actionUnselectAll);
    connect(ui->actionUnselectAll, SIGNAL(triggered(bool)), this, SLOT(on_btnUnselectAll_clicked()));
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

FileFormat *ExportDialog::getCurrentFormat()
{
    return currentFormat;
}

FileFormat::SubTypeFlags ExportDialog::getSubTypes()
{
    FileFormat::SubTypeFlags subTypes = FileFormat::Unknown;
    checkSubType(subTypes, ui->cbAccounts, FileFormat::AccountsInBrief);
    checkSubType(subTypes, ui->cbAliases, FileFormat::Aliases);
    checkSubType(subTypes, ui->cbCategories, FileFormat::Categories);
    checkSubType(subTypes, ui->cbExpenses, FileFormat::Expenses);
    checkSubType(subTypes, ui->cbIncomes, FileFormat::Incomes);
    checkSubType(subTypes, ui->cbTransfer, FileFormat::Transfer);
    // TODO other sc
    return subTypes;
}

QString ExportDialog::getPath()
{
    return ui->lePath->text();
}

bool ExportDialog::isDir()
{
    return ui->rbDirectory->isChecked();
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

void ExportDialog::checkSubType(FileFormat::SubTypeFlags& subTypes, QCheckBox *cb, FileFormat::SubType typeFlag)
{
    if (cb->isChecked())
        subTypes |= typeFlag; // WAS: setFlag
}


void ExportDialog::on_btnSelectAll_clicked()
{
    selectSubTypeIfEnabled(ui->cbAccounts);
    selectSubTypeIfEnabled(ui->cbAliases);
    selectSubTypeIfEnabled(ui->cbCategories);
    selectSubTypeIfEnabled(ui->cbExpenses);
    selectSubTypeIfEnabled(ui->cbIncomes);
    selectSubTypeIfEnabled(ui->cbTransfer);
    // TODO other sc
}

void ExportDialog::on_btnUnselectAll_clicked()
{
    ui->cbAccounts->setChecked(false);
    ui->cbAliases->setChecked(false);
    ui->cbCategories->setChecked(false);
    ui->cbExpenses->setChecked(false);
    ui->cbIncomes->setChecked(false);
    ui->cbTransfer->setChecked(false);
    // TODO other sc
}

void ExportDialog::accept()
{
    if (ui->cbAccounts->isChecked()
        ||ui->cbAliases->isChecked()
        ||ui->cbCategories->isChecked()
        ||ui->cbExpenses->isChecked()
        ||ui->cbIncomes->isChecked()
        ||ui->cbTransfer->isChecked()
        // TODO other sc
        ) {
        configManager.setLastExportedFile(ui->lePath->text());
        QDialog::accept();
    }
    else
        QMessageBox::information(0, S_ERROR, S_ERR_EMPTY_EXPORT_SUBTYPES);
}

void ExportDialog::on_btnSelectPath_clicked()
{
    QString path = ui->lePath->text();
    if (ui->rbDirectory->isChecked())
        path = QFileDialog::getExistingDirectory(0, tr("Select export directory"), path);
    else
        path = QFileDialog::getSaveFileName(0, tr("Select export file"),
            path, factory->supportedFilters(QIODevice::WriteOnly, false).join(";;"));
    ui->lePath->setText(path);
}

