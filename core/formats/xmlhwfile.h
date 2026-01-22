/* Home wallet
 *
 * Module: HomeWallet XML file export/import
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef XMLHWFILE_H
#define XMLHWFILE_H

#include <QMap>
#include "xmlfile.h"

class XmlHwFile : public XmlFile
{
public:
    XmlHwFile();
    virtual bool detect(const QString &path);
    virtual QIODevice::OpenMode supportedModes();
    virtual QStringList supportedFilters();
    virtual SubTypeFlags supportedExportSubTypes();
    virtual QString formatAbbr();
    virtual bool isDialogRequired();
    virtual bool importRecords(const QString &path, HwDatabase& db);
    virtual bool exportRecords(const QString &path, HwDatabase& db, FileFormat::SubTypeFlags subTypes);
private:
    bool importAccounts(const QDomElement& e, HwDatabase& db);
    bool importCategories(const QDomElement& elRoot, HwDatabase& db);
    bool importCategoryTree(const QDomElement& e, HwDatabase& db, bool forExpenses);
    bool importImportReferences(const QDomElement& e, HwDatabase& db);
    bool importExpenses(const QDomElement& e, HwDatabase& db);
    bool importIncomes(const QDomElement& e, HwDatabase& db);
    bool importTransfer(const QDomElement& e, HwDatabase& db);
    bool importCurrencyConversion(const QDomElement& e, HwDatabase& db);
    bool importAliases(const QDomElement& e, HwDatabase& db);

    bool exportAccounts(HwDatabase& db, QDomElement& elRoot);
    bool exportAliases(HwDatabase& db, QDomElement& elRoot);

    bool importAliasesGroup(HwDatabase &db, const QDomElement& elAliGr, HwDatabase::AliasType alType,
         const QString& errorMessageIfRefMissing, HwDatabase::DictColl& alColl, HwDatabase::DictColl& srcColl);

    bool exportCategories(HwDatabase& db, QDomElement& elRoot);
    bool exportExpenses(HwDatabase& db, QDomElement& elRoot);
    bool exportIncomes(HwDatabase& db, QDomElement& elRoot);
    bool exportTransfer(HwDatabase& db, QDomElement& elRoot);
    bool exportCurrencyConversion(HwDatabase& db, QDomElement& elRoot);
    bool exportCredits(HwDatabase& db, QDomElement& elRoot, const QString& groupName, const QString& elName, bool isLend);

    bool exportImportReferences(HwDatabase& db, QDomElement& elRoot);
};

#endif // XMLHWFILE_H
