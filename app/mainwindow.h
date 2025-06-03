/* Home Wallet
 *
 * Module: Main window
 *
 * Copyright 2023 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QTableView>

#include "formats/formatfactory.h"
#include "hwdatabase.h"
#include "expensemodel.h"
#include "incomemodel.h"
#include "transfermodel.h"
#include "currconvmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void openDb(const QString& path);
    void updateViews();
    void resizeViews();
    HwDatabase db;
    FormatFactory factory;

protected:
    virtual void showEvent(QShowEvent*);
    virtual void resizeEvent(QResizeEvent* e);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_action_About_triggered();
    void on_actionAbout_Qt_triggered();
    void on_action_DbDebug_triggered();
    void on_actionE_xit_triggered();
    void on_action_Import_triggered();
    void on_action_Settings_triggered();
    void on_leQuickFilter_textChanged(const QString&);
    void on_actionFilter_triggered();
    void on_btn_Quick_Filter_Apply_clicked();
    void on_btn_Filter_Apply_clicked();
    void on_btn_Filter_Reset_clicked();
    void on_cbDateFrom_stateChanged(int);
    void on_cbDateTo_stateChanged(int);
    void on_tabWidgetMain_currentChanged(int);
    void on_tabWidgetTransferAndExchange_currentChanged(int);
    void on_cbCategory_activated(int);
    void on_Model_Error(const QString& message);

private:
    Ui::MainWindow *ui;
    ExpenseModel* mdlExpenses;
    IncomeModel* mdlIncomes;
    TransferModel* mdlTransfer;
    CurrConvModel* mdlCurrConv;
    QSortFilterProxyModel
        *proxyExpenses, *proxyIncomes, *proxyTransfer, *proxyCurrConv;

    QLabel *lbCounts;
    enum ActiveTab {
        atExpenses,
        atIncomes,
        atTransfer,
        atExchange,
        atAccounts // m.b. atNone?
    };
    void prepareModel(FilteredQueryModel* source, QSortFilterProxyModel *proxy, QTableView* view, const QString& nameForDebug);
    void updateOneModel(FilteredQueryModel* source);
    void updateConfig();
    void updateTabsAndFilters();
    ActiveTab activeTab();
};

#endif // MAINWINDOW_H
