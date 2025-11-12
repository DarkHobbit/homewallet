/* Home wallet
 *
 * Module: Home Bookkeeping (keepsoft.ru) XML file export/import
 *
 * Copyright 2023 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef XMLHBFILE_H
#define XMLHBFILE_H

#include <QRegExp>
#include "xmlfile.h"

#define S_HB_SELECT_AMBIG_TITLE QObject::tr("Ambiguous file type")
#define S_HB_SELECT_AMBIG_ASK \
  QObject::tr("File contents can be %1.\nCategory samples:\n%2\nSpecify appropriate content type")
#define S_HB_SELECT_AMBIG_IE  QObject::tr("incomes or expenses")
#define S_HB_SELECT_AMBIG_DC  QObject::tr("debtors or creditors")
#define S_HB_SELECT_AMBIG_PIE  QObject::tr("income or expense plan")

#define S_ERR_ACC_DET_NOT_SUPPORTED \
  QObject::tr("Accounts in-detail file not supported because not contains key account data/" \
  "\nUse Accounts in-brief file, if you have it")
#define S_ERR_ACC_EMPTY_SUM \
  QObject::tr("Income or expense amount can't be equal to 0. Date: %1")
#define S_ERR_MULTI_ACC_NOT_SUPPORTED \
  QObject::tr("This version of HomeWallet not support multi-currency incomes and expenses, item from %1, currencies: %2")
#define S_ERR_UNK_LANG_FLAG \
  QObject::tr("Unknown %1: %2. Probably you are using Home Bookkeeping with new UI language.\n" \
    "Contact Homewallet author, tell %3 and UI language,\n" \
    "or manually add it to %4.")

#define S_TREAT_AS_INCOMES QObject::tr("Treat as incomes")
#define S_TREAT_AS_EXPENSES QObject::tr("Treat as expenses")
#define S_TREAT_AS_DEBTORS QObject::tr("Treat as debtors")
#define S_TREAT_AS_CREDITORS QObject::tr("Treat as creditors")
#define S_TREAT_AS_INC_PLAN QObject::tr("Treat as income planning")
#define S_TREAT_AS_EXP_PLAN QObject::tr("Treat as expense planning")

class XmlHbFile: public XmlFile
{
public:
    XmlHbFile();
    virtual QIODevice::OpenMode supportedModes();
    virtual QStringList supportedFilters();
    virtual SubTypeFlags supportedExportSubTypes();
    virtual void clear();
    virtual bool detect(const QString &path);
    virtual QString formatAbbr();
    virtual bool isDialogRequired();
    virtual bool importRecords(const QString &path, HwDatabase& db);
    virtual bool exportRecords(const QString &path, HwDatabase& db, FileFormat::SubTypeFlags subTypes);
    SubType fileSubType();
    void setFileSubType(SubType subType);
    QString categorySamples();
    inline bool isAmbiguous()
    {
        return
            _fileSubType==IncomesOrExpenses ||
            _fileSubType==DebtorsOrCreditors ||
            _fileSubType==IncomeOrExpensePlan;

    }
private:
    SubType _fileSubType;
    QString _categorySamples;
    const QRegExp hbMoneySum;
    QStringList
        debtClosedValues, debtNotClosedValues,
        yearPercentValues, yearPeriodValues, monthPeriodValues;
    void collectCatSamples(short maxRecordCount, const QString &fieldName);
    int importAccount(const QString& attr, const QDomElement& elRow, HwDatabase::DictColl& accs, HwDatabase& db);
    bool importMoneyGroup(HwDatabase::MultiCurrByChar& values, const QString& attrPrefix, const QDomElement& elRow,
        bool skipNulls, bool failIfAllNulls=true);
    int importOneMoneyAttr(const QString& val, QString& sCur);
    int importCurrencyByChar(const QString& moneyChar, HwDatabase& db);
    bool prepareImportValues(const QString& hbkImpValPath);
};

#endif // XMLHBFILE_H
