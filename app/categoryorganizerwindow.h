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

#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QWidget>

#include "categoryhiermodel.h"
#include "helpers.h"
#include "hierfilterproxymodel.h"
#include "hwdatabase.h"

namespace Ui {
class CategoryOrganizerWindow;
}

class CategoryOrganizerWindow : public QWidget, public SelecTables
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
    void on_btn_Delete_clicked();
    void on_actFilter_triggered();
    void treeEntered(const QModelIndex &);

    void on_cbShowOperations_toggled(bool checked);

private:
    Ui::CategoryOrganizerWindow *ui;
    CategoryHierModel *mdlExpCatLeft, *mdlExpCatRight,  *mdlIncCatLeft, *mdlIncCatRight;
    HierFilterProxyModel *proxyExpCatLeft, *proxyExpCatRight,  *proxyIncCatLeft, *proxyIncCatRight;
    // Potentially unsafe pointers (covered by checkActiveTree() in all changed)
    QAbstractItemModel* activeModel;
    QTreeView* activeView;
    // End of potentially unsafe pointers

};

#endif // CATEGORYORGANIZERWINDOW_H
