/* Home wallet
 *
 * Module: Generic abstract class for XML-base file formats export/import
 *
 * Copyright 2023 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef XMLFILE_H
#define XMLFILE_H

#include <QDateTime>
#include <QDomDocument>
#include "fileformat.h"

#define S_ELEM_MISSING \
    QObject::tr("XML element %1 is missing")
#define S_ERR_UNK_ELEM \
    QObject::tr("Unknown XML element: %1")
#define S_ERR_READ_CONTENT \
    QObject::tr("Can't read content from file %1\n%2\nline %3, col %4\n")

typedef QMap<int,QDomElement> ChildRecMap;

class XmlFile: public FileFormat, public QDomDocument
{
public:
    XmlFile(); // for read-only types
    XmlFile(const QString &docTypeName);
    virtual void clear();
    virtual QStringList supportedExtensions();
protected:
    // Write
    QDomElement beginCreateXml(const QString& rootElementName);
    bool endCreateXml(const QString& path);
    QDomElement addElem(QDomElement& elParent, const QString& name);
    bool exportDbRecordsGroup(HwDatabase& db, const QString& qs, QDomElement& elGroup,
        const QString& reqElemName, ChildRecMap* children = 0);
    bool exportDbRecordsGroupWithParent(HwDatabase& db, const QString& qs, QDomElement& elParent,
        const QString& groupElemName,  const QString& reqElemName, ChildRecMap* children = 0);
    // Read
    bool importDbRecordsGroup(HwDatabase& db,
        const QDomElement& elGroup, const QString& elemName,
        const QString& tableName, const QStringList &fieldNames,
        const QString &fieldTypes, const QString &fieldOptionality,
        const QStringList& attrNames,
        const HwDatabase::SubDictColl &refAttrs = HwDatabase::SubDictColl(),
        const QVariantList& extraValues = QVariantList(),
        ChildRecMap* children = 0);
    bool readFromFile(const QString &path);
    bool readDoubleVal(const QDomElement& el, const QString& attrName, double& res, const QString& errorMessageTemplate);
    bool readDateVal(const QDomElement& el, const QString& attrName, QDateTime& res, const QString& format, const QString& errorMessageTemplate);
    bool readPercentVal(const QDomElement& el, const QString& attrName, double &res, const QString& errorMessageTemplate, QString& tail);
};

#endif // XMLFILE_H
