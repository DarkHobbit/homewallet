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

#include "creditmodel.h"
#include "currconvmodel.h"
#include "filteredquerymodel.h"
#include "expensemodel.h"
#include "incomemodel.h"
#include "transfermodel.h"

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
    void on_action_Export_triggered();
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
    void on_tabWidgetLendAndBorrow_currentChanged(int);
    void on_cbCategory_activated(int);
    void processModelError(const QString& message);
    void on_btn_Add_clicked();
    void on_btn_Edit_clicked();
    void on_btn_Delete_clicked();    

private:
    Ui::MainWindow *ui;
    ExpenseModel* mdlExpenses;
    IncomeModel* mdlIncomes;
    TransferModel* mdlTransfer;
    CurrConvModel* mdlCurrConv;
    CreditModel *mdlLend, *mdlBorrow;
    QSortFilterProxyModel
        *proxyExpenses, *proxyIncomes, *proxyTransfer, *proxyCurrConv,
        *proxyLend, *proxyBorrow;
    FQMlist dbModels;
    // Potentially unsafe pointers (covered by activeTab() in all changed)
    QTableView* activeView;
    FilteredQueryModel* activeModel;
    // End of potentially unsafe pointers
    QModelIndexList selection;

    QLabel *lbCounts;
    enum ActiveTab {
        atExpenses,
        atIncomes,
        atTransfer,
        atExchange,
        atLend,
        atBorrow,
        atAccounts // m.b. atNone?
    };
    void prepareModel(FilteredQueryModel* source, QSortFilterProxyModel *proxy, QTableView* view, const QString& nameForDebug);
    void updateOneModel(FilteredQueryModel* source);
    void updateConfig();
    void updateTabsAndFilters();
    ActiveTab activeTab();
    bool checkSelection(bool errorIfNoSelected = true, bool onlyOneRowAllowed = false);
    void processImpExErrors(FileFormat* f, bool res, const QString& path);
    void processImpExSuccess(FileFormat* f);
};

#endif // MAINWINDOW_H
