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

#include <QDateTime>
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
    // TODO user, program version

    // TODO call testFlags for each subtypes
    // TODO on each iteration, call exportAliases, exportExpenses, etc
    // TODO <aliases> <expenses> etc
    // <ali> <exp> etc
    return endCreateXml(path);
}
