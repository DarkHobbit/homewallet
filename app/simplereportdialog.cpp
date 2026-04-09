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

#include <QMessageBox>

#include "globals.h"
#include "simplereportdialog.h"
#include "ui_simplereportdialog.h"

SimpleReportDialog::SimpleReportDialog(const QString& title, HwDatabase* db,  QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SimpleReportDialog)
{
    ui->setupUi(this);
    setWindowTitle(title);
    // TODO m.b. de instead dte
    QDateTime dtFrom, dtTo;
    if (!db->testDateRange(dtFrom, dtTo)) {
        QMessageBox::critical(0, S_ERROR, db->lastError());
        return;
    }
    ui->dteFrom->setDateTime(dtFrom);
    ui->dteTo->setDateTime(dtTo);
}

SimpleReportDialog::~SimpleReportDialog()
{
    delete ui;
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
