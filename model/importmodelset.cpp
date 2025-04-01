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

#include "importmodelset.h"

ImportModelSet::ImportModelSet(ImpCandidates *cands, QObject *parent)
    :QObject(parent)
{
    // Categorize all candidates
    int i=0;
    for (ImpRecCandidate& cand: *cands) {
        switch (cand.type) {
        case ImpRecCandidate::Income:
            refsIncome << &cand;
            break;
        case ImpRecCandidate::Expense:
        case ImpRecCandidate::ReceiptStart:
        case ImpRecCandidate::ReceiptEnd:
            refsExpense << &cand;
            break;
        case ImpRecCandidate::Transfer:
            refsTransfer << &cand;
            break;
        case ImpRecCandidate::Debtor:
        case ImpRecCandidate::Creditor:
            refsDebAndCred << &cand;
            break;
        default:
            refsUnknown << &cand;
        }
        i++;
    }
    // Create models
    mdlIncome = new ImportCandidatesModel(ImpRecCandidate::Income, refsIncome, this);
    mdlExpense = new ImportCandidatesModel(ImpRecCandidate::Expense, refsExpense, this);
    mdlTransfer = new ImportCandidatesModel(ImpRecCandidate::Transfer, refsTransfer, this);
    mdlDebAndCred = new ImportCandidatesModel(ImpRecCandidate::Debtor, refsDebAndCred, this);
    mdlUnknown = new ImportCandidatesModel(ImpRecCandidate::Unknown, refsUnknown, this);
}

QString ImportModelSet::stat()
{
    calcAllCounts();
    return tr("Candidates (all/ready): expenses %1/%2, incomes %3/%4, transfer %5/%6, debit-credit %7/%8, unknown %9\n")
            .arg(refsExpense.count()).arg(expReadyCount)
            .arg(refsIncome.count()).arg(incReadyCount)
            .arg(refsTransfer.count()).arg(trfReadyCount)
            .arg(refsDebAndCred.count()).arg(dncReadyCount)
            .arg(refsUnknown.count())
          + tr("Possibly duplicates: expenses %1, incomes %2, transfer %3, debit-credit %4")
            .arg(expPossDupCount).arg(incPossDupCount)
            .arg(trfPossDupCount).arg(dncPossDupCount);
}

bool ImportModelSet::canImport()
{
    calcAllCounts();
    return
        (refsExpense.count()==expReadyCount) &&
        (refsIncome.count()==incReadyCount) &&
        (refsTransfer.count()==trfReadyCount) &&
        (refsDebAndCred.count()==dncReadyCount) &&
        (refsUnknown.count()==0);
}

void ImportModelSet::calcCounts(CandRefs &refs, int &readyCount, int &possiblyDupsCount)
{
    readyCount = 0;
    possiblyDupsCount = 0;
    for (ImpRecCandidate* cand: refs) {
        if (cand->state==ImpRecCandidate::ReadyToImport || cand->state==ImpRecCandidate::PossiblyDup)
            readyCount++;
        if (cand->state==ImpRecCandidate::PossiblyDup)
            possiblyDupsCount++;
    }
}

void ImportModelSet::calcAllCounts()
{
    calcCounts(refsIncome, incReadyCount, incPossDupCount);
    calcCounts(refsExpense, expReadyCount, expPossDupCount);
    calcCounts(refsTransfer, trfReadyCount, trfPossDupCount);
    calcCounts(refsDebAndCred, dncReadyCount, dncPossDupCount);
}
