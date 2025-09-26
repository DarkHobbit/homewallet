/* Home Wallet
 *
 * Module: Candidates model for interactive import formats
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef IMPORTCANDIDATESMODEL_H
#define IMPORTCANDIDATESMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "formats/interactiveformat.h"

typedef QList<ImpRecCandidate*> CandRefs;

class ImportCandidatesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ImportCandidatesModel(ImpRecCandidate::Type _candType,
                                    CandRefs& _candRefs,
                                    QObject *parent = 0);
    // Base model implementation methods
    Qt::ItemFlags flags(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
    ImpRecCandidate* cand(int index);
    void update();

private:
    ImpRecCandidate::Type candType;
    CandRefs& candRefs;
    QString fromLowUnit(int amount) const;
};

#endif // IMPORTCANDIDATESMODEL_H
