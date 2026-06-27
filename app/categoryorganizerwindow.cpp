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

#define S_ERR_SEL_PARENT QObject::tr("Select parent category")

#include <QKeyEvent>
#include <QMessageBox>

#include "categoryorganizerwindow.h"
#include "globals.h"
#include "hierfilterproxymodel.h"
#include "simpledictdialog.h"
#include "subcategorydialog.h"
#include "ui_categoryorganizerwindow.h"

CategoryOrganizerWindow::CategoryOrganizerWindow(HwDatabase* db,  QWidget *parent)
    : QWidget(parent), SelecTablesPair()
    , ui(new Ui::CategoryOrganizerWindow), _db(db)
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
    mdlTransTypeLeft = new TransferTypeHierModel(db, this);
    proxyTransTypeLeft = new HierFilterProxyModel(this);
    mdlTransTypeRight = new TransferTypeHierModel(db, this);
    proxyTransTypeRight = new HierFilterProxyModel(this);

    prepareModel(mdlExpCatLeft, proxyExpCatLeft, ui->tvExpCatLeft, "ExpCatLeft");
    prepareModel(mdlExpCatRight, proxyExpCatRight, ui->tvExpCatRight, "ExpCatRight");
    prepareModel(mdlIncCatLeft, proxyIncCatLeft, ui->tvIncCatLeft, "IncCatLeft");
    prepareModel(mdlIncCatRight, proxyIncCatRight, ui->tvIncCatRight, "IncCatRight");
    prepareModel(mdlTransTypeLeft, proxyTransTypeLeft, ui->tvTransTypeLeft, "TransferTypesLeft");
    prepareModel(mdlTransTypeRight, proxyTransTypeRight, ui->tvTransTypeRight, "TransferTypesRight");

    activeView = ui->tvExpCatLeft;
    // Filter
    ui->leQuickFilter->installEventFilter(this);
    addAction(ui->actFilter);
    // Add menu
    menuAdd = new QMenu(this);
    menuAdd->addAction(ui->actAddCat);
    connect(ui->actAddCat, SIGNAL(triggered()), this, SLOT(addCategory()));
    menuAdd->addAction(ui->actAddSubcat);
    connect(ui->actAddSubcat, SIGNAL(triggered()), this, SLOT(addSubcategory()));
    ui->btn_Add->setMenu(menuAdd);
    connect(ui->btn_Add, SIGNAL(clicked(bool)), this, SLOT(addCategory()));
    // Button access control
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(selectionChanged()));
    selectionChanged();
}

CategoryOrganizerWindow::~CategoryOrganizerWindow()
{
    delete ui;
}

void CategoryOrganizerWindow::checkActiveTree()
{
    curW = ui->tabWidget->currentWidget();
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
    else if (curW==ui->tabTransType) {
        activeModel = mdlTransTypeLeft;
        if (activeView!=ui->tvTransTypeRight) {
            activeView = ui->tvTransTypeLeft;
            oppositeView = ui->tvTransTypeRight;
            activeModel = mdlTransTypeLeft;
            oppositeModel = mdlTransTypeRight;
        }
        else {
            oppositeView = ui->tvTransTypeLeft;
            activeModel = mdlTransTypeRight;
            oppositeModel = mdlTransTypeLeft;
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
    ui->tvTransTypeLeft->setColumnWidth(0, w);
    ui->tvTransTypeRight->setColumnWidth(0, w);
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

void CategoryOrganizerWindow::on_btn_Move_clicked()
{
    checkActiveTree();
    if (selection.isEmpty() || oppositeSelection.isEmpty())
        return;
    QModelIndex indDest = oppositeSelection.first();
    QString destName = oppositeModel->getName(indDest);
    if (QMessageBox::question(0, S_CONFIRM,
        S_MOVE_CONFIRM.arg(destName),
        QMessageBox::Yes, QMessageBox::No)==QMessageBox::Yes)
    {
        if (!activeModel->moveSelectedNodes(oppositeModel, selection, indDest))
            QMessageBox::critical(0, S_ERROR, activeModel->lastError());
        activeModel->refresh();
        oppositeModel->refresh();
    }
}

void CategoryOrganizerWindow::on_btn_Merge_clicked()
{
    checkActiveTree();
    if (selection.isEmpty() || oppositeSelection.isEmpty())
        return;
    QModelIndex indSrc = selection.first();
    QModelIndex indDest = oppositeSelection.first();
    QString srcName = activeModel->getName(indSrc);
    QString destName = oppositeModel->getName(indDest);
    if (QMessageBox::question(0, S_CONFIRM,
        S_MERGE_CONFIRM.arg(srcName).arg(destName).arg(destName),
        QMessageBox::Yes, QMessageBox::No)==QMessageBox::Yes)
    {
        if (!activeModel->mergeSelectedNodes(oppositeModel, indSrc, indDest))
            QMessageBox::critical(0, S_ERROR, activeModel->lastError());
        activeModel->refresh();
        oppositeModel->refresh();
    }
}

void CategoryOrganizerWindow::addCategory()
{
    QString tableName = "", entityName = "";
    if (curW==ui->tabExpenseCats || curW==ui->tabIncomeCats) {
        bool isExpense = curW==ui->tabExpenseCats;
        tableName = isExpense ? "hw_ex_cat" : "hw_in_cat";
        entityName = isExpense ? tr("expense category") : tr("income category");
    }
    else if (curW==ui->tabTransType) {
        tableName = "hw_transfer_type";
        entityName = tr("transfer type");
    }
    // TODO transfer types, etc.
    SimpleDictDialog* d = new SimpleDictDialog(tableName, entityName, false, _db, 0);
    d->addRecord("");
    if (d->result()==QDialog::Accepted) {
        // TODO
        activeModel->refresh();
        oppositeModel->refresh();
    }
    delete d;
}

void CategoryOrganizerWindow::addSubcategory()
{
    checkActiveTree();
    if (selection.count()!=1) {
        QMessageBox::critical(0, S_ERROR, S_ERR_SEL_PARENT);
        return;
    }
    const QModelIndex& parentItem = selection.first();
    if (!activeModel->isCategory(parentItem)) {
        QMessageBox::critical(0, S_ERROR, S_ERR_SEL_PARENT);
        return;
    }
    bool isExpense = curW==ui->tabExpenseCats;
    SubCategoryDialog* d = new SubCategoryDialog(isExpense, false, _db, 0);
    d->addSubCategory("", activeModel->getId(parentItem));
    if (d->result()==QDialog::Accepted) {
        activeModel->refresh();
        oppositeModel->refresh();
    }
    // TODO find parent in refreshed tree and expand
    delete d;
}

void CategoryOrganizerWindow::on_btn_Edit_clicked()
{
    checkActiveTree();
    if (!checkSelection()) return;
    // TODO
}

void CategoryOrganizerWindow::on_btn_Delete_clicked()
{
    checkActiveTree();
    if (!checkSelection()) return;
    if (QMessageBox::question(0, S_CONFIRM, S_REMOVE_CONFIRM,
                              QMessageBox::Yes, QMessageBox::No)==QMessageBox::Yes)
    {
        if (!activeModel->removeAnyRows(selection))
            QMessageBox::critical(0, S_ERROR, activeModel->lastError());
        activeModel->refresh();
        oppositeModel->refresh();
    }
}

void CategoryOrganizerWindow::on_btn_Refresh_clicked()
{
    checkActiveTree();
    activeModel->refresh();
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
        if (curW==ui->tabExpenseCats || curW==ui->tabIncomeCats)
            ui->btn_Add->setMenu(menuAdd);
        else
            ui->btn_Add->setMenu(0);
        // Current and opposite selection
        checkSelection(false, false);
        // Access
        if (!selection.isEmpty()) {
            bool isCatOrSubcat = activeModel->isCategory(selection.first())
                    || activeModel->isSubcategory(selection.first());
            ui->btn_Edit->setEnabled(isCatOrSubcat && selection.count()==1);
            ui->btn_Delete->setEnabled(isCatOrSubcat && selection.count()>0);
            bool hasCat=false, hasSubcat=false, hasOp=false;
            foreach (const QModelIndex& item, selection) {
                if (activeModel->isCategory(item))
                    hasCat = true;
                if (activeModel->isSubcategory(item))
                    hasSubcat = true;
                if (activeModel->isOperation(item))
                    hasOp = true;
            }
            if (oppositeSelection.count()==1) {
                const QModelIndex& oppItem = oppositeSelection.first();
                // Move and merge access
                ui->btn_Move->setEnabled(
                    // 1. Move subcategories to other category
                    (oppositeModel->isCategory(oppItem) && !hasCat && hasSubcat && !hasOp)
                    // 2. Move operations to other subcategory
                    || (oppositeModel->isSubcategory(oppItem) && !hasCat && !hasSubcat && hasOp)
                );
                if (selection.count()==1)
                    ui->btn_Merge->setEnabled(
                        // 1. Merge categories
                        (oppositeModel->isCategory(oppItem) && hasCat && !hasSubcat && !hasOp)
                         // 2. Merge subcategories
                        || (oppositeModel->isSubcategory(oppItem) && !hasCat && hasSubcat && !hasOp)
                    );
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
    mdlTransTypeLeft->setOperationShow(checked);
    mdlTransTypeRight->setOperationShow(checked);
}

void CategoryOrganizerWindow::showUserInfo(const QString &message)
{
    QMessageBox::information(0, S_INFORM, message);
}

void CategoryOrganizerWindow::prepareModel(HierModelBase *source,
    QSortFilterProxyModel *proxy, QTreeView *tree, const QString &nameForDebug)
{
    prepareBaseModel(source, proxy, tree, nameForDebug, false);
    // Provide track active tree
    connect(tree, SIGNAL(activated(QModelIndex)), this, SLOT(treeEntered(QModelIndex)));
    // Button access control
    connect(tree->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged()));
    // Model info
    connect(source, SIGNAL(infoForUser(QString)), this, SLOT(showUserInfo(QString)));
}
