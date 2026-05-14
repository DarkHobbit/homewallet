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

#ifndef HIERFILTERPROXYMODEL_H
#define HIERFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class HierFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit HierFilterProxyModel(QObject *parent = nullptr);
    
    void setFilterWildcard(const QString &pattern);
    QString filterPattern() const { return m_filterPattern; }
    
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    
private:
    QString m_filterPattern;
    bool matchesPattern(const QString &text) const;
};

#endif // HIERFILTERPROXYMODEL_H