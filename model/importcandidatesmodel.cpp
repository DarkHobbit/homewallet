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

#include <QBrush>
#include "globals.h"
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

int ImportCandidatesModel::rowCount(const QModelIndex&) const
{
    return candRefs.count();
}

// Income/Expense:
// 0 date, 1 source 2 amount, 3 curr, 4 acc, 5 alias,
// 6 cat, 7 subcat,8 qty, 9 unit, 10 descr, 11 button
// Transfer:
// 0 date, 1 source 2 amount, 3 curr, 4 accfrom, 5 accto, 6 type, 7 descr, 8 button
int ImportCandidatesModel::columnCount(const QModelIndex&) const
{
    switch(candType) {
    case ImpRecCandidate::Expense:
    case ImpRecCandidate::Income:
        return 12;
    case ImpRecCandidate::Transfer:
        return 9;
        // TODO
    default: return 1;
    }
}

QVariant ImportCandidatesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (orientation==Qt::Horizontal)) {
        switch(candType) {
        case ImpRecCandidate::Expense:
        case ImpRecCandidate::Income:
            switch (section) {
            case 0:  return S_COL_DATE;
            case 1:  return S_COL_SOURCE;
            case 2:  return S_COL_SUM;
            case 3:  return S_COL_CURRENCY;
            case 4:  return S_COL_ACCOUNT;
            case 5:  return S_COL_ALIAS;
            case 6:  return S_COL_CATEGORY;
            case 7:  return S_COL_SUBCATEGORY;
            case 8:  return S_COL_QUANTITY;
            case 9:  return S_COL_UNIT;
            case 10: return S_COL_DESCRIPTION;
            //case 11: return S_COL_DATE; button?
            default: return QAbstractItemModel::headerData(section, orientation, role);
            }
        case ImpRecCandidate::Transfer:
            // TODO
        // TODO
        default: return 1;
        }
    }
    else
        return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant ImportCandidatesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= candRefs.count())
        return QVariant();
    ImpRecCandidate* c = candRefs[index.row()];
    if (role==Qt::DisplayRole) {
        switch(candType) {
        case ImpRecCandidate::Expense:
        case ImpRecCandidate::Income:
        switch(index.column()) {
            case 0:  return gd.useSystemDateTimeFormat ? c->opDT.toString() : c->opDT.toString(gd.dateFormat);
            case 1:  return c->source;
            case 2:  return fromLowUnit(c->amount);
            case 3:  return c->currName;
            case 4:  return c->accName;
            case 5:  return c->alias;
            case 6:  return c->catName;
            case 7:  return c->subcatName;
            case 8:  return c->quantity;
            case 9:  return c->unitName;
            case 10: return c->descr;
        default: return QVariant();
        }
        case ImpRecCandidate::Transfer:
            // TODO
        // TODO
        default: return "--";
        }
    }
    else if (role==Qt::TextAlignmentRole) {
        return index.column()==2 ? Qt::AlignRight : QVariant(); // TODO and quantity
    }
    else if (role==Qt::BackgroundRole) {
        switch (c->state) {
        case ImpRecCandidate::ReadyToImport:
            return QBrush(Qt::green);
        case ImpRecCandidate::PossiblyDup:
            return QBrush(Qt::yellow);
        default:
            return QBrush(Qt::red);
        }
    }
//    else if (role==SortStringRole) {
//    }
    return QVariant();
}

QString ImportCandidatesModel::fromLowUnit(int amount) const
{
    return QString::number((double)amount/100, 'f', 2);
}
