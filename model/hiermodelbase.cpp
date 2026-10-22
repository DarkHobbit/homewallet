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

#include <QLocale>
#include <QSqlQuery>

HierModelBase::HierModelBase(HwDatabase* db, QObject *parent)
    : QAbstractItemModel(parent)
    , m_db(db)
    , m_showOperations(false)
{
    Q_ASSERT(m_db != nullptr);
}

void HierModelBase::setOperationShow(bool show)
{
    if (m_showOperations == show)
        return;

    m_showOperations = show;
    refresh();
}

QString HierModelBase::formatAmount(int amountInLowUnits) const
{
    double amountInMainUnits = amountInLowUnits / 100.0;
    return QLocale().toString(amountInMainUnits, 'f', 2);
}
bool HierModelBase::loadAliases(const QString& entityType,
                                QMap<int, QVector<QPair<int, QString>>>& aliases)
{
    aliases.clear();

    QSqlQuery query(m_db->sqlDbRef());
    QString sql = QString("SELECT id, %1, pattern FROM hw_alias WHERE %1 IS NOT NULL")
                  .arg(entityType);

    if (!m_db->prepQuery(query, sql)) {
        m_lastError = m_db->lastError();
        return false;
    }
    if (!m_db->execQuery(query)) {
        m_lastError = m_db->lastError();
        return false;
    }

    while (query.next()) {
        int aliasId = query.value(0).toInt();
        int entityId = query.value(1).toInt();
        QString pattern = query.value(2).toString();
        aliases[entityId].append(qMakePair(aliasId, pattern));
    }
    return true;
}
