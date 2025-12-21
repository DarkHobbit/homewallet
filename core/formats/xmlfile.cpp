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

#include <QSqlQuery>
#include <QSqlRecord>
#include <QTextStream>

#include "commonexpimpdef.h"
#include "xmlfile.h"

XmlFile::XmlFile()
    :FileFormat(), QDomDocument()
{}

XmlFile::XmlFile(const QString &docTypeName)
    :FileFormat(), QDomDocument(docTypeName)
{}

void XmlFile::clear()
{
    FileFormat::clear();
    QDomDocument::clear();
}

QStringList XmlFile::supportedExtensions()
{
    return (QStringList() << "xml" << "XML");
}

QDomElement XmlFile::beginCreateXml(const QString &rootElementName)
{
    QDomNode xmlNode =
        createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
    insertBefore(xmlNode, firstChild());
    QDomElement elRoot = createElement(rootElementName);
    appendChild(elRoot);
    return elRoot;
}

bool XmlFile::endCreateXml(const QString &path)
{
    QFile file(path);
    if (file.exists())
        file.remove();
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream out(&file);
        save(out, 4);
        file.close();
        QDomDocument::clear();
        return true;
    }
    else {
        _fatalError = file.errorString();
        return false;
    }
}

QDomElement XmlFile::addElem(QDomElement &elParent, const QString &name)
{
    QDomElement elem = createElement(name);
    elParent.appendChild(elem);
    return elem;
}

bool XmlFile::exportElemsFromQuery(HwDatabase &db, QDomElement &elParent,
    const QString &groupElemName, const QString &recElemName, const QString &query,
    const QStringList& fieldNames)
{
    QSqlQuery sqlSel(db.sqlDbRef());
    if (!db.prepQuery(sqlSel, query))
        return false;
    if (!db.execQuery(sqlSel))
        return false;
    if (db.queryRecCount(sqlSel)==0)
        return true;
    QDomElement elGroup = addElem(elParent, groupElemName);
    sqlSel.first();
    while (sqlSel.isValid()) {
        QDomElement elRec = addElem(elGroup, recElemName);
        for (int i=0; i<fieldNames.count(); i++)
            elRec.setAttribute(fieldNames[i], sqlSel.value(i).toString());
        _processedRecordsCount++;
        sqlSel.next();
    }
    return true;
}

bool XmlFile::exportDbRecordsGroup(HwDatabase &db, const QString &qs, QDomElement &elGroup,
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

bool XmlFile::readFromFile(const QString &path)
{
    if (!openFile(path, QIODevice::ReadOnly))
        return false;
    QString err_msg;
    int err_line, err_col;
    if (!setContent(&file, &err_msg, &err_line, &err_col)) {
        _fatalError = S_ERR_READ_CONTENT.arg(path).arg(err_msg).arg(err_line).arg(err_col);
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

bool XmlFile::readPercentVal(const QDomElement &el, const QString &attrName, double &res, const QString &errorMessageTemplate, QString& tail)
{
    bool ok;
    QString sPercRate = el.attribute(attrName);
    int percPos = sPercRate.indexOf("%");
    if (percPos==-1) {
        _fatalError = errorMessageTemplate.arg(sPercRate);
        return false;
    }
    res = prepareDoubleImport(sPercRate.left(percPos)).toDouble(&ok);
    if (!ok) {
        _fatalError = errorMessageTemplate.arg(sPercRate);
        return false;
    }
    tail = sPercRate.mid(percPos+1).trimmed();
    return ok;
}

