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

#include "importcandidatesmodel.h"

ImportCandidatesModel::ImportCandidatesModel(ImpRecCandidate::Type _candType,
                                               CandRefs& _candRefs,
                                               QObject *parent)
    : QAbstractTableModel(parent),
      candType(_candType),
      candRefs(_candRefs)
{    
}

Qt::ItemFlags ImportCandidatesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    // TODO m.b. exclude receipt begin & receipt end
    return f;
}

int ImportCandidatesModel::rowCount(const QModelIndex &parent) const
{
    //
}

const int candModelColumnCount = 13;
// Import/Export:
// 0 type 1 date, 2 source 3 amount, 4 curr, 5 acc, 6 alias,
// 7 cat, 8 subcat, 9 qty, 10 unit, 11 descr, 12 button
// Transfer:
// 0 type 1 date, 2 source 3 amount, 4 curr, 5 accfrom, 6 accto, 7 type, 8 descr, 9 button
int ImportCandidatesModel::columnCount(const QModelIndex &parent) const
{
    //
}

QVariant ImportCandidatesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    //
}

QVariant ImportCandidatesModel::data(const QModelIndex &index, int role) const
{
    //
}
