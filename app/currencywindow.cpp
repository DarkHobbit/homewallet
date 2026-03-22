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
#include "ui_currencywindow.h"

CurrencyWindow::CurrencyWindow(QWidget *parent, HwDatabase& db)
    : QWidget(parent)
    , ui(new Ui::CurrencyWindow)
{
    ui->setupUi(this);

    mdlCurr = new CurrencyModel(this, db);
    QSortFilterProxyModel* proxyCurr = new QSortFilterProxyModel(this);
    prepareModel(mdlCurr, proxyCurr, ui->tvCurrencies, "Curr", true);
    mdlCurr->update();
    if (!mdlCurr->isValid())
        QMessageBox::critical(0, S_ERROR, mdlCurr->lastError());
    ui->tvCurrencies->resizeColumnsToContents();
    updateTableConfig(ui->tvCurrencies);
    updateOneView(ui->tvCurrencies, true);

    mdlRate = new CurrencyRateModel(this, db);
    QSortFilterProxyModel* proxyRate = new QSortFilterProxyModel(this);
    prepareModel(mdlRate, proxyRate, ui->tvRates, "Rate", true);
    mdlRate->update();
    if (!mdlRate->isValid())
        QMessageBox::critical(0, S_ERROR, mdlRate->lastError());
    ui->tvRates->resizeColumnsToContents();
    updateTableConfig(ui->tvRates);
    updateOneView(ui->tvRates, true);
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

void CurrencyWindow::activeTab()
{
    if (ui->tabWidget->currentWidget()==ui->tabCurrencies) {
        activeView = ui->tvCurrencies;
        activeModel = mdlCurr;
    }
    else {
        activeView = ui->tvRates;
        activeModel = mdlRate;
    }
}

void CurrencyWindow::on_btn_Delete_clicked()
{
    activeTab();
    if (!checkSelection()) return;
    if (QMessageBox::question(0, S_CONFIRM, S_REMOVE_CONFIRM,
                              QMessageBox::Yes, QMessageBox::No)==QMessageBox::Yes)
    {
        if (activeModel->removeAnyRows(selection)) {
            mdlCurr->update();
            mdlRate->update();
            // TODO restore prev position from removes
        }
    }
}

