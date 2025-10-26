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

#include "xmlhwfile.h"

XmlHwFile::XmlHwFile()
    :XmlFile("homewallet")
{}

bool XmlHwFile::detect(const QString &path)
{
    // TODO
    return false; //===>
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
    QDomDocument doc;
    // TODO
    return false; //===>
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
