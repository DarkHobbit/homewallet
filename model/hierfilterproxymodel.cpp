/* Home Wallet
 *
 * Module: Hierarchical proxy model (filter only, not sort)
 * This class is written using DeepSeek chat
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

// TODO filter works on first column only, DeepSeek know solution

#include "hierfilterproxymodel.h"
#include <QDebug>

HierFilterProxyModel::HierFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterKeyColumn(-1);
}

void HierFilterProxyModel::setFilterWildcard(const QString &pattern)
{
    m_filterPattern = pattern;
    qDebug() << "HierFilterProxyModel::setFilterWildcard:" << pattern;
    invalidateFilter();
}

bool HierFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // Если нет фильтра, показываем всё
    if (m_filterPattern.isEmpty()) {
        return true;
    }
    
    // Получаем индекс элемента
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    
    // Получаем данные для отображения (имя категории/подкатегории)
    QString displayData = sourceModel()->data(index, filterRole()).toString();
    
    // Проверяем текущий элемент
    if (matchesPattern(displayData)) {
        return true;
    }
    
    // Рекурсивно проверяем потомков
    int childCount = sourceModel()->rowCount(index);
    for (int i = 0; i < childCount; ++i) {
        if (filterAcceptsRow(i, index)) {
            return true;
        }
    }
    
    return false;
}

bool HierFilterProxyModel::matchesPattern(const QString &text) const
{
    return text.contains(m_filterPattern, filterCaseSensitivity());
}