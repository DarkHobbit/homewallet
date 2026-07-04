/* Home Wallet
 *
 * Module: Category organizer window
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
#ifndef CATEGORYORGANIZERWINDOW_H
#define CATEGORYORGANIZERWINDOW_H

#include <QMenu>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QWidget>

#include "accounthiermodel.h"
#include "categoryhiermodel.h"
#include "helpers.h"
#include "hierfilterproxymodel.h"
#include "hiermodelbase.h"
#include "hwdatabase.h"
#include "transfertypehiermodel.h"

namespace Ui {
class CategoryOrganizerWindow;
}

class CategoryOrganizerWindow : public QWidget, public SelecTablesPair
{
    Q_OBJECT

public:
    explicit CategoryOrganizerWindow(HwDatabase* db,  QWidget *parent = nullptr);
    ~CategoryOrganizerWindow();

protected:
    void checkActiveTree();
    void changeEvent(QEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);
    void showEvent(QShowEvent* e);

private slots:
    void on_btn_Quick_Filter_Apply_clicked();
    void on_btn_Move_clicked();
    void on_btn_Merge_clicked();
    void addCategory();
    void addSubcategory();
    void on_btn_Edit_clicked();
    void on_btn_Delete_clicked();
    void on_btn_Refresh_clicked();
    void on_actFilter_triggered();
    void treeEntered(const QModelIndex &);
    void selectionChanged();
    void on_cbShowOperations_toggled(bool checked);
    void showUserInfo(const QString& message);

private:
    Ui::CategoryOrganizerWindow *ui;
    HwDatabase* _db;
    // Models
    AccountHierModel
        *mdlAccountsLeft, *mdlAccountsRight;
    CategoryHierModel
        *mdlExpCatLeft, *mdlExpCatRight,
        *mdlIncCatLeft, *mdlIncCatRight;
    TransferTypeHierModel
        *mdlTransTypeLeft, *mdlTransTypeRight;
    HierFilterProxyModel
        *proxyAccountsLeft, *proxyAccountsRight,
        *proxyExpCatLeft, *proxyExpCatRight,
        *proxyIncCatLeft, *proxyIncCatRight,
        *proxyTransTypeLeft, *proxyTransTypeRight;
    // Potentially unsafe pointers (covered by checkActiveTree() in all changed)
    QWidget* curW;
    HierModelBase *activeModel, *oppositeModel;
    // End of potentially unsafe pointers
    QMenu* menuAdd;
    void prepareModel(HierModelBase *source, QSortFilterProxyModel *proxy, QTreeView *tree, const QString &nameForDebug);
};

#endif // CATEGORYORGANIZERWINDOW_H
