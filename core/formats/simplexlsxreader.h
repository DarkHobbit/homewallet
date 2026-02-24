/* Home wallet
 *
 * Module: Simple XLSX format reader using quazip and QtXml
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef SIMPLEXLSXREADER_H
#define SIMPLEXLSXREADER_H

#include <QDomDocument>
#include <QStringList>
#include <QVariantList>
#include "quazip.h"

class SimpleXlsxReader
{
public:
    SimpleXlsxReader(const QString& _path);
    bool read();
    int rowCount(int pageNum);
    int columnCount(int pageNum);
    QString cellValue(int row, int column);
    QString lastError();
private:
    QString path, error;
    int maxColCount;
    QStringList sharedStrings;
    QList<QVariantList> values;
    bool loadInnerXml(QuaZip& arc, const QString& path, const QString& rootName, QDomDocument &doc, QDomElement& elRoot);
};

#endif // SIMPLEXLSXREADER_H
