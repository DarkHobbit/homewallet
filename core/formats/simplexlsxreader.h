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
