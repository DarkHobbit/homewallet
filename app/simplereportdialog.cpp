/* Home Wallet
 *
 * Module: Dialog for date-based reports
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QFileDialog>
#include <QMessageBox>

#include "configmanager.h"
#include "globals.h"
#include "reports.h"
#include "simplereportdialog.h"
#include "ui_simplereportdialog.h"

SimpleReportDialog::SimpleReportDialog(const QString& title, HwDatabase* db,  QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SimpleReportDialog)
{
    ui->setupUi(this);
    setWindowTitle(title);
    ui->lePath->setText(configManager.lastExportedFile());
    QDateTime dtFrom, dtTo;
    if (!db->testDateRange(dtFrom, dtTo)) {
        QMessageBox::critical(0, S_ERROR, db->lastError());
        return;
    }
    ui->deFrom->setDate(dtFrom.date());
    ui->deTo->setDate(dtTo.date());
}

SimpleReportDialog::~SimpleReportDialog()
{
    delete ui;
}

QBoxLayout *SimpleReportDialog::mainLayout()
{
    return ui->verticalLayout;
}

QWidget *SimpleReportDialog::lastBaseWidget()
{
    return ui->deTo;
}

void SimpleReportDialog::getData(QString& path, QDate &dtMin, QDate &dtMax)
{
    path = ui->lePath->text();
    dtMin = ui->deFrom->date();
    dtMax = ui->deTo->date();
}

void SimpleReportDialog::changeEvent(QEvent *e)
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

void SimpleReportDialog::on_btnSetPath_clicked()
{
    Reports r;
    QString path = ui->lePath->text();
    QString selectedFilter;
    path = QFileDialog::getSaveFileName(0, tr("Select report file"),
        path, r.makeDocumentFormatFilters(), &selectedFilter);
    ui->lePath->setText(path);
}

