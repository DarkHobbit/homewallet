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

#ifndef HELPERS_H
#define HELPERS_H

#include <QAbstractItemModel>
#include <QComboBox>
#include <QListWidget>
#include <QSortFilterProxyModel>
#include <QSqlQueryModel>
#include <QStringList>
#include <QTableView>

#include "genericdatabase.h"

#define GUI_DB_CHK(action, q) \
{ \
        if (!(action)) \
    { \
            QMessageBox::critical(0, S_ERROR, q.lastError().text()); \
            return; \
    } \
}


QStringList getListItems(QListWidget* list);
void fillComboByDict(QComboBox* combo, GenericDatabase::DictColl coll, bool addAllItem);
int getComboCurrentId(QComboBox* combo);
// setSimilarComboText() assumes that combo box items sorted alphabetically
void setSimilarComboText(QComboBox* combo, const QString& pattern);
// Set color/font for each table view
void updateTableConfig(QTableView* table);
void updateOneView(QTableView* view, bool isDatabaseView);

// Second base class for widgets with QTableView-s and proxy models
// Can be used with QMainWindow, QWidget, etc.
class SelecTables {
protected:
    // Potentially unsafe pointers (covered by activeTab() in all changed)
    QTableView* activeView;
    // End of potentially unsafe pointers
    QModelIndexList selection;
    void prepareModel(QSqlQueryModel* source, QSortFilterProxyModel *proxy, QTableView* view, const QString& nameForDebug);
    bool checkSelection(bool errorIfNoSelected = true, bool onlyOneRowAllowed = false);
};

#endif // HELPERS_H
