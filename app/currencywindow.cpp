/* Home Wallet
 *
 * Module: Currency window
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

#include "currencywindow.h"
#include "globals.h"
#include "miscmodels.h"
#include "ui_currencywindow.h"

CurrencyWindow::CurrencyWindow(QWidget *parent, HwDatabase& db)
    : QWidget(parent)
    , ui(new Ui::CurrencyWindow)
{
    ui->setupUi(this);

    CurrencyModel* mdlCurr = new CurrencyModel(this, db);
    ui->tvCurrencies->setModel(mdlCurr);
    if (!mdlCurr->isValid())
        QMessageBox::critical(0, S_ERROR, mdlCurr->lastError());
    ui->tvCurrencies->resizeColumnsToContents();

    CurrencyRateModel* mdlRate = new CurrencyRateModel(this, db);
    ui->tvRates->setModel(mdlRate);
    if (!mdlRate->isValid())
        QMessageBox::critical(0, S_ERROR, mdlRate->lastError());
    ui->tvRates->resizeColumnsToContents();
}

CurrencyWindow::~CurrencyWindow()
{
    delete ui;
}

void CurrencyWindow::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
