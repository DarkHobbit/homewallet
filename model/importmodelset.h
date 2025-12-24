/* Home Wallet
 *
 * Module: Set of models for interactive import
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef IMPORTMODELSET_H
#define IMPORTMODELSET_H

#include <QObject>
#include <QSortFilterProxyModel>

#include "importcandidatesmodel.h"

class ImportModelSet : public QObject
{
    Q_OBJECT
public:
    explicit ImportModelSet(ImpCandidates* cands, QObject *parent = 0);
    QString stat();
    bool canImport();
    ImportCandidatesModel
        *mdlIncome, *mdlExpense, *mdlTransfer,
        *mdlDebAndCred, *mdlUnknown;
    QSortFilterProxyModel
        *proxyIncome, *proxyExpense, *proxyTransfer,
        *proxyDebAndCred, *proxyUnknown;
    CandRefs refsIncome, refsExpense, refsTransfer,
        refsDebAndCred, refsUnknown;
    int incReadyCount, expReadyCount, trfReadyCount, dncReadyCount;
    int incPossDupCount, expPossDupCount, trfPossDupCount, dncPossDupCount;

private:
    QSortFilterProxyModel* makeProxy(ImportCandidatesModel* source);
    void calcCounts(CandRefs& refs, int& readyCount, int& possiblyDupsCount);
    void calcAllCounts();

signals:

};

#endif // IMPORTMODELSET_H
