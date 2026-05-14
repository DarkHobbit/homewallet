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

#include <QKeyEvent>

#include "categoryorganizerwindow.h"
#include "hierfilterproxymodel.h"
#include "ui_categoryorganizerwindow.h"

CategoryOrganizerWindow::CategoryOrganizerWindow(HwDatabase* db,  QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CategoryOrganizerWindow)
    , activeView(0)
{
    ui->setupUi(this);
    // Models
    mdlExpCatLeft = new CategoryHierModel(true, db, this);
    proxyExpCatLeft = new HierFilterProxyModel(this);
    mdlExpCatRight = new CategoryHierModel(true, db, this);
    proxyExpCatRight = new HierFilterProxyModel(this);
    mdlIncCatLeft = new CategoryHierModel(false, db, this);
    proxyIncCatLeft = new HierFilterProxyModel(this);
    mdlIncCatRight = new CategoryHierModel(false, db, this);
    proxyIncCatRight = new HierFilterProxyModel(this);
    prepareBaseModel(mdlExpCatLeft, proxyExpCatLeft, ui->tvExpCatLeft, "ExpCatLeft", false);
    prepareBaseModel(mdlExpCatRight, proxyExpCatRight, ui->tvExpCatRight, "ExpCatRight", false);
    prepareBaseModel(mdlIncCatLeft, proxyIncCatLeft, ui->tvIncCatLeft, "IncCatLeft", false);
    prepareBaseModel(mdlIncCatRight, proxyIncCatRight, ui->tvIncCatRight, "IncCatRight", false);
    // Provide track active tree
    connect(ui->tvExpCatLeft, SIGNAL(activated(QModelIndex)), this, SLOT(treeEntered(QModelIndex)));
    connect(ui->tvExpCatRight, SIGNAL(activated(QModelIndex)), this, SLOT(treeEntered(QModelIndex)));
    connect(ui->tvIncCatLeft, SIGNAL(activated(QModelIndex)), this, SLOT(treeEntered(QModelIndex)));
    connect(ui->tvIncCatRight, SIGNAL(activated(QModelIndex)), this, SLOT(treeEntered(QModelIndex)));
    activeView = ui->tvExpCatLeft;
    // Filter
    ui->leQuickFilter->installEventFilter(this);
    addAction(ui->actFilter);
}

CategoryOrganizerWindow::~CategoryOrganizerWindow()
{
    delete ui;
}

void CategoryOrganizerWindow::checkActiveTree()
{
    QWidget* curW = ui->tabWidget->currentWidget();
    // activeView also can be setted by widget activate
    if (curW==ui->tabExpenseCats) {
        activeModel = mdlExpCatRight;
        if (activeView!=ui->tvExpCatRight) {
            activeView = ui->tvExpCatLeft;
            activeModel = mdlExpCatLeft;
        }
    }
    else if (curW==ui->tabIncomeCats) {
        activeModel = mdlIncCatRight;
        if (activeView!=ui->tvIncCatRight) {
            activeView = ui->tvIncCatLeft;
            activeModel = mdlIncCatLeft;
        }
    }
    else {
        activeView = 0;
        activeModel = 0;
    }
}

void CategoryOrganizerWindow::changeEvent(QEvent *e)
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

bool CategoryOrganizerWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *kEv = static_cast<QKeyEvent *>(event);
        if (kEv->key()==Qt::Key_Enter || kEv->key()==Qt::Key_Return) {
            if (focusWidget()==ui->leQuickFilter)
                on_btn_Quick_Filter_Apply_clicked();
        }
        else
            return false;
        return true;
    } else // default
        return QObject::eventFilter(obj, event);
}

void CategoryOrganizerWindow::on_btn_Quick_Filter_Apply_clicked()
{
    checkActiveTree();
    if (!activeView)
        return;
    HierFilterProxyModel* proxy = dynamic_cast<HierFilterProxyModel*>(activeView->model());
    if (!proxy)
        return;
    proxy->setFilterWildcard(ui->leQuickFilter->text());
}


void CategoryOrganizerWindow::on_btn_Delete_clicked()
{
    checkActiveTree();
    if (!checkSelection()) return;
    // TODO implement removeAnyRows(selection) in CategoryHierModel?
    //mdlExpCatLeft->getId(ind1)
    //mdlExpCatLeft->getParentCategoryId(ind)
}


void CategoryOrganizerWindow::on_actFilter_triggered()
{
    ui->leQuickFilter->setFocus();
}

void CategoryOrganizerWindow::treeEntered(const QModelIndex &)
{
    QTreeView* s = dynamic_cast<QTreeView*>(sender());
    if (s)
        activeView = s;
}

