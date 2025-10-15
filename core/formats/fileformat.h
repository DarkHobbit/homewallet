/* Home wallet
 *
 * Module: Abstract class for file export/import format
 *
 * Copyright 2016 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef FILEFORMAT_H
#define FILEFORMAT_H

#include <QFile>
#include <QFlags>
#include "hwdatabase.h"
#include <QStringList>

#define DB_CHK(action) \
{ \
        if (!(action)) \
    { \
            _fatalError = db.lastError(); \
            return false; \
    } \
}

class FileFormat
{
public:
    enum SubType { // Subtype or export & import file
        Unknown         = 0x0000,
        AccountsInBrief = 0x0001,
        CurrencyRate    = 0x0002,
        Incomes         = 0x0004,
        Expenses        = 0x0008,
        Transfer        = 0x0010,
        CurrencyConversion = 0x0020,
        Debtors     = 0x0040, // TODO see, has database fio dictionary?
        Creditors   = 0x0080, // TODO -"-
        IncomePlan  = 0x0100,
        ExpensePlan = 0x0200,
        // HomeWallet specific
        Aliases     = 0x0400,
        Categories  = 0x0800, // including currencies and units
        // (Home Bookkeeping XML) specific
        AccountsInDetail  = 0x2000, // not supported, because not contains key account data
        IncomesOrExpenses, // ambiguous, must be replaced on Incomes or Expenses by caller
        DebtorsOrCreditors, // ambiguous, must be replaced on Debtors or Creditors by caller
        IncomeOrExpensePlan // ambiguous, must be replaced on PlanIncomes or PlanExpenses by caller
    };
    typedef QFlags<SubType> SubTypeFlags;
    static const int subTypeFlagsCount = 13;

    FileFormat();
    virtual ~FileFormat();
    virtual void clear();
    virtual bool detect(const QString &path)=0;
    virtual QIODevice::OpenMode supportedModes()=0;
    virtual QStringList supportedExtensions()=0;
    virtual QStringList supportedFilters()=0;
    virtual SubTypeFlags supportedExportSubTypes()=0;
    virtual QString formatAbbr()=0;
    virtual bool isDialogRequired()=0;
    virtual bool importRecords(const QString &path, HwDatabase& db)=0;
    virtual bool postImport(HwDatabase&);
    void setIdImp(int idImp);
    virtual bool exportRecords(const QString &path, HwDatabase& db, FileFormat::SubTypeFlags subTypes)=0;
    QStringList errors();
    QString fatalError();
    int importedRecordsCount();
    int totalRecordsCount();
    static void lossData(QStringList& errors, const QString& recName,
        const QString& fieldName, bool condition);
    static void lossData(QStringList& errors, const QString& recName,
        const QString& fieldName, const QString& field);
protected:
    int _idImp;
    QFile file;
    QStringList _errors;
    QString _fatalError;
    int _importedRecordsCount, _totalRecordsCount;
    bool openFile(QString path, QIODevice::OpenMode mode);
    void closeFile();
};

extern char subTypeFileNames[][FileFormat::subTypeFlagsCount];

#endif // FILEFORMAT_H
