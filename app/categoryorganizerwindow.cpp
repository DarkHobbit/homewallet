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
    : QWidget(parent), SelecTablesPair()
    , ui(new Ui::CategoryOrganizerWindow)
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
    // Button access control
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(selectionChanged()));
    SetTreeSelectionHandler(ui->tvExpCatLeft);
    SetTreeSelectionHandler(ui->tvExpCatRight);
    SetTreeSelectionHandler(ui->tvIncCatLeft);
    SetTreeSelectionHandler(ui->tvIncCatRight);
    selectionChanged();
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
            oppositeView = ui->tvExpCatRight;
            activeModel = mdlExpCatLeft;
            oppositeModel = mdlExpCatRight;
        }
        else {
            oppositeView = ui->tvExpCatLeft;
            activeModel = mdlExpCatRight;
            oppositeModel = mdlExpCatLeft;
        }
    }
    else if (curW==ui->tabIncomeCats) {
        activeModel = mdlIncCatRight;
        if (activeView!=ui->tvIncCatRight) {
            activeView = ui->tvIncCatLeft;
            oppositeView = ui->tvIncCatRight;
            activeModel = mdlIncCatLeft;
            oppositeModel = mdlIncCatRight;
        }
        else {
            oppositeView = ui->tvIncCatLeft;
            activeModel = mdlIncCatRight;
            oppositeModel = mdlIncCatLeft;
        }
    }
    else {
        activeView = 0;
        oppositeView = 0;
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

void CategoryOrganizerWindow::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    int w = ui->tvExpCatLeft->width()*3/4; // Important for tvInc* - are hidden while showEvent() is calling
    ui->tvExpCatLeft->setColumnWidth(0, w);
    ui->tvExpCatRight->setColumnWidth(0, w);
    ui->tvIncCatLeft->setColumnWidth(0, w);
    ui->tvIncCatRight->setColumnWidth(0, w);
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

void CategoryOrganizerWindow::on_btn_Edit_clicked()
{
    checkActiveTree();
    if (!checkSelection()) return;

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

void CategoryOrganizerWindow::selectionChanged()
{
    checkActiveTree();
    ui->btn_Move->setEnabled(false);
    ui->btn_Merge->setEnabled(false);
    ui->btn_Add->setEnabled(false);
    ui->btn_Edit->setEnabled(false);
    ui->btn_Delete->setEnabled(false);
    if (activeView) {
        ui->btn_Add->setEnabled(true);
        // Current and opposite selection
        checkSelection(false, false);
        // Access
        if (!selection.isEmpty()) {
            CategoryHierModel* mdlHier = dynamic_cast<CategoryHierModel*>(activeModel);
            if (mdlHier) {
                bool isCatOrSubcat = mdlHier->isCategory(selection.first())
                                  || mdlHier->isSubcategory(selection.first());
                ui->btn_Edit->setEnabled(isCatOrSubcat && selection.count()==1);
                ui->btn_Delete->setEnabled(isCatOrSubcat && selection.count()>0);
                bool hasCat=false, hasSubcat=false, hasOp=false;
                foreach (const QModelIndex& item, selection) {
                    if (mdlHier->isCategory(item))
                        hasCat = true;
                    if (mdlHier->isSubcategory(item))
                        hasSubcat = true;
                    if (mdlHier->isOperation(item))
                        hasOp = true;
                }
                if (oppositeSelection.count()==1) {
                    const QModelIndex& oppItem = oppositeSelection.first();
                    // Move and merge access
                    ui->btn_Move->setEnabled(
                        // 1. Move subcategories to other category
                        (mdlHier->isCategory(oppItem) && !hasCat && hasSubcat && !hasOp)
                        // 2. Move operations to other subcategory
                        || (mdlHier->isSubcategory(oppItem) && !hasCat && !hasSubcat && hasOp)
                    );
                    ui->btn_Merge->setEnabled(
                        // 1. Merge categories
                        (mdlHier->isCategory(oppItem) && hasCat && !hasSubcat && !hasOp)
                        // 2. Merge subcategories
                        || (mdlHier->isSubcategory(oppItem) && !hasCat && hasSubcat && !hasOp)
                    );
                }
            }
            else {
                // TODO transfers, etc.
            }
        }
    }
}

void CategoryOrganizerWindow::on_cbShowOperations_toggled(bool checked)
{
    mdlExpCatLeft->setOperationShow(checked);
    mdlExpCatRight->setOperationShow(checked);
    mdlIncCatLeft->setOperationShow(checked);
    mdlIncCatRight->setOperationShow(checked);
}

void CategoryOrganizerWindow::SetTreeSelectionHandler(QTreeView* tree)
{
    connect(tree->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged()));
}
