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

#include <iostream>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>

#include "commonexpimpdef.h"
#include "xmlhwfile.h"

XmlHwFile::XmlHwFile()
    :XmlFile("homewallet")
{}

bool XmlHwFile::detect(const QString &path)
{
    // Read XML
    if (!readFromFile(path))
        return false;
    QDomElement elRoot = documentElement();
    return elRoot.nodeName()=="homewallet";
}

QIODevice::OpenMode XmlHwFile::supportedModes()
{
    return QIODevice::ReadOnly | QIODevice::WriteOnly;
}

QStringList XmlHwFile::supportedFilters()
{
    return QStringList() << QObject::tr("HomeWallet XML (*.xml *.XML)");
}

FileFormat::SubTypeFlags XmlHwFile::supportedExportSubTypes()
{
    return (FileFormat::SubTypeFlags)
        (AccountsInBrief | Aliases | Categories
         | Expenses | Incomes | Transfer
         | CurrencyConversion | Debtors | Creditors
        );
}

QString XmlHwFile::formatAbbr()
{
    return "XMHW";
}

bool XmlHwFile::isDialogRequired()
{
    return false;
}

bool XmlHwFile::importRecords(const QString &path, HwDatabase &db)
{
    // Read XML
    if (!readFromFile(path))
        return false;
    QDomElement elRoot = documentElement();
    if (elRoot.nodeName()!="homewallet")
        return false;
    // Order matter! Accounts, categories (at first, units), inc/exp/trans, aliases
    _processedRecordsCount = 0;
    // Accounts
    QDomElement e = elRoot.firstChildElement("accounts");
    if (!e.isNull()) {
        if (!importAccounts(e, db))
            return false;
    }
    // Categories
    if (!importCategories(elRoot, db))
        return false;
    // Import refs
    e = elRoot.firstChildElement("importfiles");
    if (!e.isNull()) {
        if (!importImportReferences(e, db))
            return false;
    }
    // Expenses
    e = elRoot.firstChildElement("expenses");
    if (!e.isNull()) {
        if (!importExpenses(e, db))
            return false;
    }
    // Incomes
    e = elRoot.firstChildElement("incomes");
    if (!e.isNull()) {
        if (!importIncomes(e, db))
            return false;
    }

    // TODO operations
    e = elRoot.firstChildElement("aliases");
    if (!e.isNull()) {
        if (!importAliases(e, db))
            return false;
    }
    // Check for unknown elements (e.g. from newer HW version)
    for (QDomElement e=elRoot.firstChildElement(); !e.isNull(); e=e.nextSiblingElement())
    {
        QString nn = e.nodeName();
        if (nn!="metadata" && nn!="accounts" && nn!="units"
                && nn!="expensecategories"  && nn!="incomecategories"
                && nn!="transfertypes"  && nn!="correspondents"
                && nn!="incomes"  && nn!="expenses"
                // TODO
                && nn!="importfiles" && nn!="aliases")
            _errors << S_ERR_UNK_ELEM.arg(e.nodeName());
    }
    std::cout << "Rec " << _processedRecordsCount << " processed" << std::endl;
    return true;
}

bool XmlHwFile::exportRecords(const QString &path, HwDatabase &db, SubTypeFlags subTypes)
{
    QDomElement elRoot = beginCreateXml("homewallet");
    QDomElement elMeta = addElem(elRoot, "metadata");
    elMeta.setAttribute("created", QDateTime::currentDateTime().toString(Qt::ISODate));
//#if QT_VERSION >= 0x050000
    elMeta.setAttribute("user", QDir::home().dirName());
//#endif
    elMeta.setAttribute("appversion", qApp->applicationVersion());

    if (subTypes.testFlag(FileFormat::AccountsInBrief))
        UP_CHK(exportAccounts(db, elRoot));
    if (subTypes.testFlag(FileFormat::Aliases))
        UP_CHK(exportAliases(db, elRoot));
    if (subTypes.testFlag(FileFormat::Categories))
        UP_CHK(exportCategories(db, elRoot));
    if (subTypes.testFlag(FileFormat::Expenses))
        UP_CHK(exportExpenses(db, elRoot));
    if (subTypes.testFlag(FileFormat::Incomes))
        UP_CHK(exportIncomes(db, elRoot));
    if (subTypes.testFlag(FileFormat::Transfer))
        UP_CHK(exportTransfer(db, elRoot));
    if (subTypes.testFlag(FileFormat::CurrencyConversion))
        UP_CHK(exportCurrencyConversion(db, elRoot));
    if (subTypes.testFlag(FileFormat::Debtors))
        UP_CHK(exportCredits(db, elRoot, "lendto", "ln", true));
    if (subTypes.testFlag(FileFormat::Creditors))
        UP_CHK(exportCredits(db, elRoot, "borrowfrom", "br", false));
    // TODO call testFlags for other subtypes

    // Export references to import file refs (not all export types)
    // In Qt 6.2+, we can use testFlags()... maybe later...
    if (subTypes.testFlag(FileFormat::Expenses)
     || subTypes.testFlag(FileFormat::Incomes)
     || subTypes.testFlag(FileFormat::Transfer)
     || subTypes.testFlag(FileFormat::CurrencyConversion)
     || subTypes.testFlag(FileFormat::Debtors)
     || subTypes.testFlag(FileFormat::Creditors))
        UP_CHK(exportImportReferences(db, elRoot));

    return endCreateXml(path);
}

bool XmlHwFile::importAccounts(const QDomElement &e, HwDatabase &db)
{
    ChildRecMap accInits;
    bool res = importDbRecordsGroup(db, e, "ac", "hw_account",
        QStringList() << "name" << "descr" << "foundation",
        "SSD", "MOO",
        QStringList() << "n" << "d" << "fd",
        HwDatabase::TableRefColl(),
        QVariantList(), &accInits);
    if (!res)
        return false;
    HwDatabase::TableRefColl tRefs;
    DB_CHK(db.collectDict(tRefs["cur"], "hw_currency", "abbr"));
    for (int idAcc: accInits.keys()) {
        const QDomElement& eIn = accInits[idAcc];
        res = importDbRecordsGroup(db, eIn, "init", "hw_acc_init",
            QStringList() << "init_sum" << "id_cur" << "id_ac",
            "IRI", "OOM",
            QStringList() << "init_sum" << "cur", tRefs,
            QVariantList() << QVariant(idAcc));
        if (!res)
            return false;
    }
    return true;
}

bool XmlHwFile::importCategories(const QDomElement &elRoot, HwDatabase &db)
{
    QDomElement e = elRoot.firstChildElement("units");
    if (!e.isNull()) {
        if (!importDbRecordsGroup(db, e, "un", "hw_unit",
                QStringList() << "name" << "short_name" << "descr",
                "SSS", "MMO",
                QStringList() << "n" << "sn" << "d"))
            return false;
    }
    e = elRoot.firstChildElement("expensecategories");
    if (!e.isNull()) {
        if (!importCategoryTree(e, db, true))
            return false;
    }
    e = elRoot.firstChildElement("incomecategories");
    if (!e.isNull()) {
        if (!importCategoryTree(e, db, false))
            return false;
    }
    e = elRoot.firstChildElement("transfertypes");
    if (!e.isNull()) {
        if (!importDbRecordsGroup(db, e, "tt", "hw_transfer_type",
                QStringList() << "name" << "descr", "SS", "MO",
                QStringList() << "n" << "d"))
            return false;
    }
    e = elRoot.firstChildElement("correspondents");
    if (!e.isNull()) {
        if (!importDbRecordsGroup(db, e, "cor", "hw_correspondent",
                QStringList() << "name" << "descr", "SS", "MO",
                QStringList() << "n" << "d"))
            return false;
    }
    return true;
}

bool XmlHwFile::importCategoryTree(const QDomElement &e, HwDatabase &db, bool forExpenses)
{
    ChildRecMap eCats;
    QString primaryTable = forExpenses ? "hw_ex_cat" : "hw_in_cat";
    QString secondaryTable = forExpenses ? "hw_ex_subcat" : "hw_in_subcat";
    QString fldParent = forExpenses ? "id_ecat" : "id_icat";
    bool res = importDbRecordsGroup(db, e, "cat", primaryTable,
        QStringList() << "name" << "descr", "SS", "MO",
        QStringList() << "n" << "d",
        HwDatabase::TableRefColl(), QVariantList(), &eCats);
    if (!res)
        return false;
    HwDatabase::TableRefColl tRefs;
    DB_CHK(db.collectDict(tRefs["und"], "hw_unit", "short_name"));
    for (int idCat: eCats.keys()) {
        ChildRecMap eSubCats;
        bool res = importDbRecordsGroup(db, eCats[idCat], "cat", secondaryTable,
            QStringList() << "name" << "descr" << "id_un_default" << fldParent,
            "SSRI", "MOOM",
            QStringList() << "n" << "d" << "und", tRefs,
            QVariantList() << QVariant(idCat), &eSubCats);
        if (!res)
            return false;
        for (int idSubCat: eSubCats.keys()) {
            if (!eSubCats[idSubCat].childNodes().isEmpty()) {
                _fatalError = QObject::tr("This version of HomeWallet not support third category level: %1::%2")
                    .arg(eCats[idCat].attribute("n"))
                    .arg(eSubCats[idSubCat].attribute("n"));
                return false;
            }
        }
    }
    return true;
}

bool XmlHwFile::importImportReferences(const QDomElement &e, HwDatabase &db)
{
    return importDbRecordsGroup(db, e, "imf", "hw_imp_file",
               QStringList() << "imp_date" << "filename" << "filetype" << "descr",
               "DSSS", "MMMO",
               QStringList() << "dt" << "fn" << "ft" << "d",
               HwDatabase::TableRefColl(), QVariantList(), 0,
               QStringList() << "filename");
}

bool XmlHwFile::importExpenses(const QDomElement &e, HwDatabase &db)
{
    HwDatabase::TableRefColl tRefs;
    DB_CHK(db.collectDict(tRefs["ac"], "hw_account"));
    DB_CHK(db.collectDict(tRefs["cu"], "hw_currency", "abbr"));
    DB_CHK(db.collectTwoLevelCat(tRefs["ca"], "hw_ex_cat", "hw_ex_subcat", "id_ecat"));
    DB_CHK(db.collectDict(tRefs["u"], "hw_unit", "short_name")); // optional
    DB_CHK(db.collectDict(tRefs["imp"], "hw_imp_file","filename")); // optional
    DB_CHK(db.collectDict(tRefs["vfy"], "hw_imp_file","filename")); // optional
    return importDbRecordsGroup(db, e, "exp", "hw_ex_op",
        QStringList() << "op_date" << "quantity" << "amount"
                      << "id_ac" << "id_cur" << "id_esubcat" << "id_un"
                      << "discount" << "attention" << "descr"
                      << "id_imp" << "uid_imp" << "id_imp_verify" << "uid_imp_verify",
        "DFIRRRRIISZSZS", "MOMMMMOOOOOOOO",
        QStringList() << "dt" << "q" << "a"
                      << "ac" << "cu" << "ca" << "u"
                      << "di" << "at" << "d" << "imp" << "vfy",
        tRefs);
}

bool XmlHwFile::importIncomes(const QDomElement &e, HwDatabase &db)
{
    HwDatabase::TableRefColl tRefs;
    DB_CHK(db.collectDict(tRefs["ac"], "hw_account"));
    DB_CHK(db.collectDict(tRefs["cu"], "hw_currency", "abbr"));
    DB_CHK(db.collectTwoLevelCat(tRefs["ca"], "hw_in_cat", "hw_in_subcat", "id_icat"));
    DB_CHK(db.collectDict(tRefs["u"], "hw_unit", "short_name")); // optional
    DB_CHK(db.collectDict(tRefs["imp"], "hw_imp_file","filename")); // optional
    DB_CHK(db.collectDict(tRefs["vfy"], "hw_imp_file","filename")); // optional
    return importDbRecordsGroup(db, e, "inc", "hw_in_op",
        QStringList() << "op_date" << "quantity" << "amount"
                      << "id_ac" << "id_cur" << "id_isubcat" << "id_un"
                      << "attention" << "descr"
                      << "id_imp" << "uid_imp" << "id_imp_verify" << "uid_imp_verify",
        "DFIRRRRISZSZS", "MOMMMMOOOOOOO",
        QStringList() << "dt" << "q" << "a"
                      << "ac" << "cu" << "ca" << "u"
                      << "at" << "d" << "imp" << "vfy",
        tRefs);
}

bool XmlHwFile::importAliases(const QDomElement &e, HwDatabase &db)
{
    HwDatabase::DictColl alColl, srcColl;
    for (QDomElement elAliGr=e.firstChildElement(); !elAliGr.isNull(); elAliGr=elAliGr.nextSiblingElement())
    {
        alColl.clear();
        srcColl.clear();
        QString noName = elAliGr.nodeName();
        if (noName=="foraccounts") {
            DB_CHK(db.collectDict(alColl, "hw_alias", "pattern", "id", "where id_ac is not null"));
            DB_CHK(db.collectDict(srcColl, "hw_account"));
            if (!importAliasesGroup(db, elAliGr, HwDatabase::Account, S_ERR_ACC_NOT_FOUND, alColl, srcColl))
                return false;
        }
        else if (noName=="forcurrency") {
            DB_CHK(db.collectDict(alColl, "hw_alias", "pattern", "id", "where id_cur is not null"));
            DB_CHK(db.collectDict(srcColl, "hw_currency", "short_name"));
            if (!importAliasesGroup(db, elAliGr, HwDatabase::Currency, S_ERR_CUR_NOT_FOUND, alColl, srcColl))
                return false;
        }
        else if (noName=="forunit") {
            DB_CHK(db.collectDict(alColl, "hw_alias", "pattern", "id", "where id_un is not null"));
            DB_CHK(db.collectDict(srcColl, "hw_unit"));
            if (!importAliasesGroup(db, elAliGr, HwDatabase::Unit, S_ERR_UNIT_NOT_FOUND, alColl, srcColl))
                return false;
        }


        // TODO
        else
            _errors << S_ERR_UNK_ELEM.arg(elAliGr.nodeName());
    }
    return true;
}

#define Q_SEL_ACCOUNT \
    "select id, name as n, descr as d, foundation as fd" \
    " from hw_account" \
    " order by name;"

#define Q_SEL_ACC_INIT \
    "select ain.id, ain.init_sum, cur.abbr as cur" \
    " from hw_acc_init ain, hw_currency cur" \
    " where ain.id_cur=cur.id" \
    " and ain.id_ac=%1" \
    " order by cur.seq_order;"

bool XmlHwFile::exportAccounts(HwDatabase &db, QDomElement &elRoot)
{
    ChildRecMap elAccs;
    QDomElement elGroup = addElem(elRoot, "accounts");
    bool res = exportDbRecordsGroup(db, Q_SEL_ACCOUNT, elGroup, "ac", &elAccs);
    if (res) {
        foreach (int idAcc, elAccs.keys())
            if (!exportDbRecordsGroup(db, QString(Q_SEL_ACC_INIT).arg(idAcc), elAccs[idAcc], "init"))
                return false;
    }
    return res;
}

#define Q_SEL_ALIAS_ACC \
    "select al.pattern, al.to_descr, reft.name as ref" \
    " from hw_alias al, hw_account reft" \
    " where al.id_ac=reft.id" \
    " order by ref;"

#define Q_SEL_ALIAS_CUR \
    "select al.pattern, al.to_descr, reft.short_name as ref" \
    " from hw_alias al, hw_currency reft" \
    " where al.id_cur=reft.id" \
    " order by ref" \

#define Q_SEL_ALIAS_UN \
"select al.pattern, al.to_descr, reft.name as ref" \
    " from hw_alias al, hw_unit reft" \
    " where al.id_un=reft.id" \
    " order by ref" \

#define Q_SEL_ALIAS_ICAT \
    "select al.pattern, al.to_descr, c.name as ref" \
        " from hw_alias al, hw_in_cat c" \
        " where al.id_icat=c.id" \
        " order by ref" \

#define Q_SEL_ALIAS_ISUBCAT \
    "select al.pattern, al.to_descr," \
        "  c.name||'::'||sc.name as ref" \
        " from hw_alias al, hw_in_cat c, hw_in_subcat sc" \
        " where al.id_isubcat=sc.id and sc.id_icat=c.id" \
        " order by ref" \

#define Q_SEL_ALIAS_ECAT \
    "select al.pattern, al.to_descr, c.name as ref" \
        " from hw_alias al, hw_ex_cat c" \
        " where al.id_ecat=c.id" \
        " order by ref" \

#define Q_SEL_ALIAS_ESUBCAT \
    "select al.pattern, al.to_descr," \
        "  c.name||'::'||sc.name as ref" \
        " from hw_alias al, hw_ex_cat c, hw_ex_subcat sc" \
        " where al.id_esubcat=sc.id and sc.id_ecat=c.id" \
        " order by ref" \


bool XmlHwFile::exportAliases(HwDatabase &db, QDomElement &elRoot)
{
    QDomElement elList = addElem(elRoot, "aliases");

    DB_CHK(exportDbRecordsGroupWithParent(db, Q_SEL_ALIAS_ACC, elList, "foraccounts", "ali"));
    DB_CHK(exportDbRecordsGroupWithParent(db, Q_SEL_ALIAS_CUR, elList, "forcurrency", "ali"));
    DB_CHK(exportDbRecordsGroupWithParent(db, Q_SEL_ALIAS_UN, elList, "forunit", "ali"));
    DB_CHK(exportDbRecordsGroupWithParent(db, Q_SEL_ALIAS_ICAT, elList, "forincomecategories", "ali"));
    DB_CHK(exportDbRecordsGroupWithParent(db, Q_SEL_ALIAS_ISUBCAT, elList, "forincomesubcategories", "ali"));
    DB_CHK(exportDbRecordsGroupWithParent(db, Q_SEL_ALIAS_ECAT, elList, "forexpensecategories", "ali"));
    DB_CHK(exportDbRecordsGroupWithParent(db, Q_SEL_ALIAS_ESUBCAT, elList, "forexpensesubcategories", "ali"));
    // TODO transfer type
    return true;
}

bool XmlHwFile::importAliasesGroup(HwDatabase &db, const QDomElement& elAliGr, HwDatabase::AliasType alType,
    const QString &errorMessageIfRefMissing, HwDatabase::DictColl& alColl, HwDatabase::DictColl& srcColl)
{
    for (QDomElement elA=elAliGr.firstChildElement(); !elA.isNull(); elA=elA.nextSiblingElement())
    {
        QString alName = elA.attribute("pattern");
        QString alRef = elA.attribute("ref");
        if (alColl.keys().contains(alName))
            continue;
        if (!srcColl.contains(alRef)) {
            _fatalError = errorMessageIfRefMissing.arg(alRef);
            return false;
        }
        db.addAlias(alName, elA.attribute("to_descr"),
            alType, srcColl[alRef]);
    }
    _processedRecordsCount += elAliGr.childNodes().count();
    return true;
}

#define Q_SEL_UNIT \
"select id, name as n, short_name as sn, descr as d" \
    " from hw_unit order by name;"

#define Q_SEL_IN_CAT \
"select id, name as n, descr as d" \
    " from hw_in_cat order by name;"

#define Q_SEL_IN_SUBCAT \
"select id, name as n, descr as d" \
    " from hw_in_subcat where id_icat=%1 order by name;"

#define Q_SEL_EX_CAT \
    "select id, name as n, descr as d" \
    " from hw_ex_cat order by name;"

#define Q_SEL_EX_SUBCAT \
"select sc.id, sc.name as n, sc.descr as d, un.short_name as und" \
    " from hw_ex_subcat sc, hw_unit un" \
    " where sc.id_un_default=un.id and sc.id_ecat=%1" \
" union " \
" select sc.id, sc.name as n, sc.descr as d, null as und" \
    " from hw_ex_subcat sc" \
    " where sc.id_un_default is null and sc.id_ecat=%2" \
" order by sc.name;"

#define Q_SEL_TR_TYPE \
"select id, name as n, descr as d" \
    " from hw_transfer_type order by name;"

#define Q_SEL_CORRESPONDENT \
"select id, name as n, descr as d" \
    " from hw_correspondent order by name;"

bool XmlHwFile::exportCategories(HwDatabase &db, QDomElement &elRoot)
{
    ChildRecMap children;
    // Units
    QDomElement elUnGroup = addElem(elRoot, "units");
    bool res = exportDbRecordsGroup(db, Q_SEL_UNIT, elUnGroup, "un");
    if (!res)
        return false;
    // Income categories
    QDomElement elICGroup = addElem(elRoot, "incomecategories");
    res = exportDbRecordsGroup(db, Q_SEL_IN_CAT, elICGroup, "cat", &children);
    if (!res)
        return false;
    foreach (int idInCat, children.keys())
        if (!exportDbRecordsGroup(db, QString(Q_SEL_IN_SUBCAT).arg(idInCat), children[idInCat], "cat"))
            return false;
    // Expense categories
    children.clear();
    QDomElement elECGroup = addElem(elRoot, "expensecategories");
    res = exportDbRecordsGroup(db, Q_SEL_EX_CAT, elECGroup, "cat", &children);
    if (!res)
        return false;
    foreach (int idExCat, children.keys())
        if (!exportDbRecordsGroup(db, QString(Q_SEL_EX_SUBCAT).arg(idExCat).arg(idExCat), children[idExCat], "cat"))
            return false;
    // Transfer types
    QDomElement elTTGroup = addElem(elRoot, "transfertypes");
    res = exportDbRecordsGroup(db, Q_SEL_TR_TYPE, elTTGroup, "tt");
    if (!res)
        return false;
    // Correspondents (debtors/creditors)
    QDomElement elCorGroup = addElem(elRoot, "correspondents");
    res = exportDbRecordsGroup(db, Q_SEL_CORRESPONDENT, elCorGroup, "cor");
    return res;
}

#define Q_SEL_EX_OP \
    "select ex.id, ex.op_date as dt, ex.quantity as q, un.short_name as u," \
    " ex.amount as a, cur.abbr as cu, acc.name as ac," \
    " cat.name||'::'||scat.name as ca," \
    " ex.discount as di," \
    " case ex.attention when 1 then 'yes' else null end as at," \
    " ex.descr as d," \
    " fim.filename||'::'||uid_imp as imp," \
    " fvf.filename||'::'||uid_imp_verify as vfy" \
    " from" \
    " hw_ex_cat cat, hw_ex_subcat scat," \
    " hw_ex_op ex" \
    " left join hw_unit un on un.id=ex.id_un" \
    " left join hw_currency cur on cur.id=ex.id_cur" \
    " left join hw_account acc on acc.id=ex.id_ac" \
    " left join hw_imp_file fim on fim.id=ex.id_imp" \
    " left join hw_imp_file fvf on fvf.id=ex.id_imp_verify" \
    " where id_rc is null" \
    " and ex.id_esubcat=scat.id" \
    " and scat.id_ecat=cat.id" \
    " order by dt, ca;"

bool XmlHwFile::exportExpenses(HwDatabase &db, QDomElement &elRoot)
{
    QDomElement elExpGroup = addElem(elRoot, "expenses");
    bool res = exportDbRecordsGroup(db, Q_SEL_EX_OP, elExpGroup, "exp");
    if (!res)
        return false;
    // TODO receipts here!
    return true;
}

#define Q_SEL_IN_OP \
"select inc.id, inc.op_date as dt, inc.quantity as q, un.short_name as u," \
    " inc.amount as a, cur.abbr as cu, acc.name as ac," \
    " cat.name||'::'||scat.name as ca," \
    " case inc.attention when 1 then 'yes' else null end as at," \
    " inc.descr as d," \
    " fim.filename||'::'||uid_imp as imp," \
    " fvf.filename||'::'||uid_imp_verify as vfy" \
    " from" \
    " hw_in_cat cat, hw_in_subcat scat," \
    " hw_in_op inc" \
    " left join hw_unit un on un.id=inc.id_un" \
    " left join hw_currency cur on cur.id=inc.id_cur" \
    " left join hw_account acc on acc.id=inc.id_ac" \
    " left join hw_imp_file fim on fim.id=inc.id_imp" \
    " left join hw_imp_file fvf on fvf.id=inc.id_imp_verify" \
    " where inc.id_isubcat=scat.id" \
    " and scat.id_icat=cat.id" \
    " order by dt, ca;"

bool XmlHwFile::exportIncomes(HwDatabase &db, QDomElement &elRoot)
{
    QDomElement elIncGroup = addElem(elRoot, "incomes");
    return exportDbRecordsGroup(db, Q_SEL_IN_OP, elIncGroup, "inc");
}

#define Q_SEL_TR \
    "select tr.id, tr.op_date as dt, cur.abbr as cur," \
    " acci.name as ai, acco.name as ao, tt.name as t," \
    " tr.descr as d," \
    " fim.filename||'::'||uid_imp as imp," \
    " fvf.filename||'::'||uid_imp_verify as vfy" \
    " from" \
    " hw_transfer tr" \
    " left join hw_currency cur on cur.id=tr.id_cur" \
    " left join hw_account acci on acci.id=tr.id_ac_in" \
    " left join hw_account acco on acco.id=tr.id_ac_out" \
    " left join hw_imp_file fim on fim.id=tr.id_imp" \
    " left join hw_imp_file fvf on fvf.id=tr.id_imp_verify" \
    " left join hw_transfer_type tt on tt.id=tr.id_tt" \
    " order by dt, t;"

bool XmlHwFile::exportTransfer(HwDatabase &db, QDomElement &elRoot)
{
    QDomElement elTrGroup = addElem(elRoot, "transfer");
    return exportDbRecordsGroup(db, Q_SEL_TR, elTrGroup, "tr");
}

#define Q_SEL_CUR_EXCH \
    "select ce.id, ce.op_date as dt, acc.name as ac," \
    " curi.abbr as ci, curo.abbr as co, amount_in as ami, amount_out as amo," \
    " ce.descr as d," \
    " fim.filename||'::'||uid_imp as imp," \
    " fvf.filename||'::'||uid_imp_verify as vfy" \
    " from" \
    " hw_curr_exch ce" \
    " left join hw_account acc on acc.id=ce.id_ac" \
    " left join hw_currency curi on curi.id=ce.id_cur_in" \
    " left join hw_currency curo on curo.id=ce.id_cur_out" \
    " left join hw_imp_file fim on fim.id=ce.id_imp" \
    " left join hw_imp_file fvf on fvf.id=ce.id_imp_verify" \
    " order by dt, co;"

bool XmlHwFile::exportCurrencyConversion(HwDatabase &db, QDomElement &elRoot)
{
    QDomElement elCeGroup = addElem(elRoot, "currencyexchange");
    return exportDbRecordsGroup(db, Q_SEL_CUR_EXCH, elCeGroup, "ce");
}

#define Q_SEL_CRED \
    "select cr.id, cr.op_date as dt, cr.close_date as dtc, remind_date as dtr," \
    " cs.name as crs, cr.amount as a, cr.down_pay as dp, cr.money_back as mb, cr.money_remaining_debt as mrd," \
    " cur.abbr as cu, acc.name as ac," \
    " cr.rate as r, cr.is_rate_onetime as ot, period as p," \
    " case period_unit" \
    "  when 0 then 'eternal'" \
    "  when 1 then 'month'" \
    "  when 2 then 'year'" \
    "  else 'unknown'" \
    " end as pu," \
    " case is_closed when 1 then 'yes' else 'no' end as cls," \
    " cr.descr as d," \
    " fim.filename||'::'||uid_imp as imp," \
    " fvf.filename||'::'||uid_imp_verify as vfy" \
    " from" \
    " hw_credit cr" \
    " left join hw_currency cur on cur.id=cr.id_cur" \
    " left join hw_account acc on acc.id=cr.id_ac" \
    " left join hw_correspondent cs on cs.id=cr.id_crs" \
    " left join hw_imp_file fim on fim.id=cr.id_imp" \
    " left join hw_imp_file fvf on fvf.id=cr.id_imp_verify" \
    " where is_lend=%1" \
    " order by dt;"

bool XmlHwFile::exportCredits(HwDatabase &db, QDomElement &elRoot, const QString& groupName, const QString& elName, bool isLend)
{
    ChildRecMap elCreds;
    QDomElement elGroup = addElem(elRoot, groupName);
    bool res = exportDbRecordsGroup(db, QString(Q_SEL_CRED).arg(isLend ? "true" : "false"),
               elGroup, elName, &elCreds);
    if (res) {
        // TODO возврат, уточнить, возврат это RETURN или что-то ещё
        /*
        foreach (int idCred, elCreds.keys())
            if (!exportDbRecordsGroup(db, QString(Q_SEL_CRED_RET).arg(idCred), elCreds[idCred], "???"))
                return false;
                */
    }
    return res;
}

#define Q_SEL_IMP_FILES \
    "select id, imp_date as dt, filename as fn, filetype as ft, descr as d" \
    " from hw_imp_file order by dt, fn;"

bool XmlHwFile::exportImportReferences(HwDatabase &db, QDomElement &elRoot)
{
    return exportDbRecordsGroupWithParent(db, Q_SEL_IMP_FILES, elRoot, "importfiles", "imf");
}
