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
    return Aliases;
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



        // TODO
        }
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

    if (subTypes.testFlag(FileFormat::Aliases))
        UP_CHK(exportAliases(db, elRoot));
    // TODO call testFlags for other subtypes, call exportExpenses <exp>, etc
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
