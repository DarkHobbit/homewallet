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

#include "commonexpimpdef.h"
#include "xmlfile.h"

XmlFile::XmlFile()
    :FileFormat(), QDomDocument()
{}

QStringList XmlFile::supportedExtensions()
{
    return (QStringList() << "xml" << "XML");
}

bool XmlFile::readFromFile(const QString &path)
{
    if (!openFile(path, QIODevice::ReadOnly))
        return false;
    QString err_msg;
    int err_line, err_col;
    if (!setContent(&file, &err_msg, &err_line, &err_col)) {
        _fatalError = QObject::tr("Can't read content from file %1\n%2\nline %3, col %4\n")
                          .arg(path).arg(err_msg).arg(err_line).arg(err_col);
        closeFile();
        return false;
    }
    closeFile();
    return true;
}

bool XmlFile::readDoubleVal(const QDomElement &el, const QString &attrName, double &res, const QString &errorMessageTemplate)
{
    QString s = prepareDoubleImport(el.attribute(attrName));
    bool ok;
    res = s.toDouble(&ok);
    if (!ok)
        _fatalError = errorMessageTemplate.arg(s);
    return ok;
}

bool XmlFile::readDateVal(const QDomElement &el, const QString &attrName, QDateTime &res, const QString& format, const QString &errorMessageTemplate)
{
    QString s = el.attribute(attrName);
    res = QDateTime::fromString(s, format);
    if (!res.isValid())
        _fatalError = errorMessageTemplate.arg(s);
    return res.isValid();
}

