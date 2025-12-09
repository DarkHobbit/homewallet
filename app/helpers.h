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

#include <QComboBox>
#include <QListWidget>
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
// Set color/font for each table view
void updateTableConfig(QTableView* table);
void updateOneView(QTableView* view, bool isDatabaseView);

#endif // HELPERS_H
