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

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QSqlRecord>

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
         | Expenses);
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
    for (QDomElement e=elRoot.firstChildElement(); !e.isNull(); e=e.nextSiblingElement())
    {
        if (e.nodeName()=="aliases") {
            if (!importAliases(e, db))
                return false;
        }
        // TODO
        else if (e.nodeName()!="metadata")
            _errors << S_ERR_UNK_ELEM.arg(e.nodeName());
    }
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
    // TODO call testFlags for other subtypes, call exportExpenses <exp>, etc

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

bool XmlHwFile::exportAliases(HwDatabase &db, QDomElement &elRoot)
{
    QDomElement elList = addElem(elRoot, "aliases");
    DB_CHK(exportElemsFromQuery(db, elList, "foraccounts", "ali", Q_SEL_ALIAS_ACC,
        QStringList() << "pattern" << "to_descr" << "ref"));
    DB_CHK(exportElemsFromQuery(db, elList, "forcurrency", "ali", Q_SEL_ALIAS_CUR,
        QStringList() << "pattern" << "to_descr" << "ref"));
    DB_CHK(exportElemsFromQuery(db, elList, "forunit", "ali", Q_SEL_ALIAS_UN,
        QStringList() << "pattern" << "to_descr" << "ref"));
    // TODO other alias destinations
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
    // Income categories
    QDomElement elICGroup = addElem(elRoot, "incomecategories");
    bool res = exportDbRecordsGroup(db, Q_SEL_IN_CAT, elICGroup, "cat", &children);
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
    // TODO check attention
    return true;
}

bool XmlHwFile::exportImportReferences(HwDatabase &db, QDomElement &elRoot)
{
    return true; // TODO
}

bool XmlHwFile::exportDbRecordsGroup(HwDatabase &db, const QString &qs, QDomElement &elGroup,
    const QString &reqElemName, ChildRecMap* children)
{
    QSqlQuery q;
    DB_CHK(db.prepQuery(q, qs));
    DB_CHK(db.execQuery(q))
    if (q.first()) {
        // Collect field names => attr names
        QStringList fieldNames;
        for (int i=1; i<q.record().count(); i++)
            fieldNames << q.record().fieldName(i);
        // Process records
        while (q.isValid()) {
            QDomElement elRec = addElem(elGroup, reqElemName);
            for (int i=1; i<=fieldNames.count(); i++) {
                if (!q.isNull(i))
                    elRec.setAttribute(fieldNames[i-1], q.value(i).toString());
            }
            if (children) {
                int id = q.value(0).toInt();
                (*children)[id] = elRec;
            }
            q.next();
        }
        _processedRecordsCount += elGroup.childNodes().count();
    }
    return true;
}
