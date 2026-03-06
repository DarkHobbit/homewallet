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
#include "currencywindow.h"
#include "miscmodels.h"
#include "ui_currencywindow.h"

CurrencyWindow::CurrencyWindow(QWidget *parent, HwDatabase& db)
    : QWidget(parent)
    , ui(new Ui::CurrencyWindow)
{
    ui->setupUi(this);
    ui->tvRates->setModel(new CurrencyRateModel(this, db));
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
