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

#define S_ERR_UNK_ELEM QObject::tr("Unknown XML element: %1")

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
    bool exportElemsFromQuery(HwDatabase &db, QDomElement& elParent,
        const QString& groupElemName, const QString& recElemName, const QString& query,
        const QStringList& fieldNames);
    // Read
    bool readFromFile(const QString &path);
    bool readDoubleVal(const QDomElement& el, const QString& attrName, double& res, const QString& errorMessageTemplate);
    bool readDateVal(const QDomElement& el, const QString& attrName, QDateTime& res, const QString& format, const QString& errorMessageTemplate);
};

#endif // XMLFILE_H
