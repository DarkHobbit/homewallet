/* Home Wallet
 *
 * Module: Widget helpers
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QFont>
#include <QHeaderView>
#include <QMessageBox>

#include "globals.h"
#include "helpers.h"

void fillComboByDict(QComboBox *combo, GenericDatabase::DictColl coll, bool addAllItem)
{
    combo->clear();
    if (addAllItem)
        combo->addItem(S_ALL_CAT, -2);
    for (const QString& key: coll.keys())
        combo->addItem(key, coll[key]);
}

int getComboCurrentId(QComboBox *combo)
{
    return combo->itemData(combo->currentIndex()).toInt();
}

// Set color/font for each table view
void updateTableConfig(QTableView *table)
{
    table->setSortingEnabled(gd.enableSorting);
    table->setShowGrid(gd.showTableGrid);
    table->verticalHeader()->setVisible(gd.showLineNumbers);
    table->setAlternatingRowColors(gd.useTableAlternateColors);
    if (!gd.useSystemFontsAndColors) {
        QFont f;
        bool fontSuccess = f.fromString(gd.tableFont);
        if (fontSuccess)
            table->setFont(f);
        table->setStyleSheet(QString("QTableView { alternate-background-color: %1; background: %2 }")
                                 .arg(gd.gridColor2).arg(gd.gridColor1));
    }
    if (gd.resizeTableRowsToContents)
        table->resizeRowsToContents();
}


void updateOneView(QTableView *view, bool isDatabaseView)
{
    if (isDatabaseView)
        view->setColumnHidden(0, true);
    if (gd.resizeTableRowsToContents)
        view->resizeRowsToContents();
    const int rH = 20;
#if QT_VERSION >= 0x050200
    view->verticalHeader()->setMaximumSectionSize(rH);
#endif
    for (int i=0; i<view->model()->rowCount(); i++) // M.b. slow on some computers!
        view->setRowHeight(i, rH);
}

QStringList getListItems(QListWidget *list)
{
    QStringList res;
    for (int i=0; i<list->count(); i++)
        res << list->item(i)->text();
    return res;
}

void setSimilarComboText(QComboBox *combo, const QString &pattern)
{
    if (combo->count()<2)
        return;
    for (int i=0; i<combo->count()-1; i++) {
        if (combo->itemText(i)<=pattern && combo->itemText(i+1)>pattern) {
            combo->setCurrentIndex(i);
            return;
        }
    }
    combo->setCurrentIndex(combo->count()-1);
}

bool SelecTables::checkSelection(bool errorIfNoSelected, bool onlyOneRowAllowed)
{
    QModelIndexList proxySelection = activeView->selectionModel()->selectedRows();
    if (proxySelection.count()==0) {
        if (errorIfNoSelected)
            QMessageBox::critical(0, S_ERROR, S_REC_NOT_SEL);
        return false;
    }
    if (onlyOneRowAllowed && (proxySelection.count()>1)) {
        QMessageBox::critical(0, S_ERROR, S_ONLY_ONE_REC);
        return false;
    }
    // Map indices from proxy to source
    QSortFilterProxyModel* selectedProxy = dynamic_cast<QSortFilterProxyModel*>(activeView->model());
    selection.clear();
    foreach(QModelIndex index, proxySelection)
        selection << selectedProxy->mapToSource(index);
    return true;
}

void SelecTables::prepareModel(QSqlQueryModel *source, QSortFilterProxyModel *proxy, QTableView *view, const QString &nameForDebug, bool customSorting)
{
    source->setObjectName(QString("mdl")+nameForDebug);
    proxy->setSourceModel(source);
    proxy->setFilterKeyColumn(-1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive); // Driver == driver
    if (customSorting)
        proxy->setSortRole(SortStringRole);
    proxy->setObjectName(QString("proxy")+nameForDebug);
    view->setModel(proxy);
//    view->horizontalHeader()->setResizeContentsPrecision(64);
    view->horizontalHeader()->setStretchLastSection(true);
    view->setObjectName(QString("tv")+nameForDebug);
}
