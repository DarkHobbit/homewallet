/* Home Wallet
 *
 * Module: Base class for hierarchical "dictionary+operations" models
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 * This class is written using DeepSeek chat
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include "hiermodelbase.h"
#include "hwdatabase.h"

HierModelBase::HierModelBase(HwDatabase* db, QObject *parent)
    : QAbstractItemModel(parent)
    , m_db(db)
    , m_showOperations(false)
    , m_lastError()
{
    Q_ASSERT(m_db != 0);
}

void HierModelBase::setOperationShow(bool show)
{
    if (m_showOperations == show)
        return;
    m_showOperations = show;
    refresh();
}