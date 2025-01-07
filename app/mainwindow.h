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

#include "hwdatabase.h"
#include "expensemodel.h"

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
    HwDatabase db;

protected:
    void resizeEvent(QResizeEvent* e);
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

    void on_tabWidget_currentChanged(int);

    void on_cbCategory_activated(int);

private:
    Ui::MainWindow *ui;
    ExpenseModel* mdlExpenses;
    QSortFilterProxyModel *proxyExpenses;
    QLabel *lbCounts;
    enum ActiveTab {
        atExpenses,
        atIncomes,
        atAccounts
    };
    void prepareModel(QAbstractItemModel* source, QSortFilterProxyModel *proxy, QTableView* view);
    void updateOneView(QTableView* view);
    ActiveTab activeTab();
};

#endif // MAINWINDOW_H
