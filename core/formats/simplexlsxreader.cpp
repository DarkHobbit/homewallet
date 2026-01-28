#include <iostream>
#include "simplexlsxreader.h"
#include "quazipfile.h"

SimpleXlsxReader::SimpleXlsxReader(const QString& _path)
    :path(_path), error(""), maxColCount(0)
{
}

bool SimpleXlsxReader::read()
{
    // Read XLSX via QuaZip
    QuaZip arc(path);
    if (!arc.open(QuaZip::mdUnzip)) {
const QString S_READ_ERR = QString("file open error %1"); //===>
        error = S_READ_ERR.arg(path);
        return false;
    }

    // Load xl/sharedStrings.xml as DOM
    QDomDocument docShared;
    QDomElement elSharedRoot;
    if (!loadInnerXml(arc, "xl/sharedStrings.xml", "sst", docShared, elSharedRoot)) {
        arc.close();
        return false;
    }
    for(QDomElement elSi=elSharedRoot.firstChildElement("si"); !elSi.isNull(); elSi=elSi.nextSiblingElement("si")) {
        QDomElement elT = elSi.firstChildElement("t");
        if (elT.isNull()) {
            error = QObject::tr("si without t in sharedStrings.xml");
            arc.close();
            return false;
        }
        sharedStrings << elT.text();
    }

    // Load xl/worksheets/sheet1.xml as DOM
    QDomDocument docSheet;
    QDomElement elSheetRoot;
    if (!loadInnerXml(arc, "xl/worksheets/sheet1.xml", "worksheet", docSheet, elSheetRoot)) {
        arc.close();
        return false;
    }
    arc.close();

    // Read values
    values.clear();
    maxColCount = 0;
    QDomElement elSd = elSheetRoot.firstChildElement("sheetData");
    if (elSd.isNull()) {
        error = QObject::tr("Element %1 is missing").arg("sheetData"); // S_ELEM_MISSING
        return false;
    }
    // Walk rows
    int i=1;
    for(QDomElement elR=elSd.firstChildElement("row"); !elR.isNull(); elR=elR.nextSiblingElement("row")) {
        if (elR.attribute("r").toInt()!=i) {
            error = QObject::tr("This version of SimpleXlsxReader doesn't support inconsquent rows, r=%1").arg(elR.attribute("r"));
            return false;
        }
        QVariantList strValues;
        // Walk columns
        char j=0;
        for(QDomElement elC=elR.firstChildElement("c"); !elC.isNull(); elC=elC.nextSiblingElement("c")) {
            QString cellR = elC.attribute("r");
            if (cellR.length()<2) {
                error = QObject::tr("Attribute r not found or too small at row %1, col %2: r=%3").arg(i).arg(j+1).arg(cellR);
                return false;
            }
            char cellR0 = cellR[0].toLatin1();
            if (cellR0<'A' || cellR0>'Z') {
                error = QObject::tr("Attribute r first character must be a capital letter at row %1, col %2: r=%3").arg(i).arg(j+1).arg(cellR);
            }
            if (!cellR[1].isDigit()) {
                error = QObject::tr("This version of SimpleXlsxReader doesn't support more than 26 columns at row %1, col %2:  r=%").arg(i).arg(j+1).arg(cellR);
                return false;
            }
            if (cellR0<'A'+j) {
                error = QObject::tr("This version of SimpleXlsxReader doesn't support inconsquent columns at row %1, col %2:  r=%").arg(i).arg(j+1).arg(cellR);
                return false;
            }
            if (cellR0>'A'+j) { // empty cells are present
                for (char k=cellR0; k<'A'+j; k++)
                    strValues << QVariant();
            }
            QDomElement elV = elC.firstChildElement("v");
            if (elV.isNull()) // empty cell
                strValues << QVariant();
            else {
                QString cType = elC.attribute("t");
                if (cType=="s") {
                    bool ok;
                    int shIndex=elV.text().toInt(&ok);
                    if (!ok) {
                        error = QObject::tr("Not nubmer shared index: %1 at row %2, col %3").arg(elV.text()).arg(i).arg(j+1);
                        return false;
                    }
                    if (shIndex>=sharedStrings.count()) {
                        error = QObject::tr("Shared index too big: %1>=%2 at row %3, col %4")
                            .arg(shIndex).arg(sharedStrings.count()).arg(i).arg(j+1);
                        return false;
                    }
                    strValues << QVariant(sharedStrings[shIndex]);
                }
                else if (cType=="n") {
                    strValues << QVariant(elV.text()); // TODO see spec; if "n" means number, not native - save as int or double
                }
                else if (cType.isEmpty()) {
                    strValues << QVariant(elV.text());
                }
                else {
                    error = QObject::tr("Unknown cell type: %1 at row %2, col %3").arg(cType).arg(i).arg(j+1);
                    return false;
                }
            }
            j++;
        }
        values << strValues;
        if (strValues.count()>maxColCount)
            maxColCount = strValues.count();
        i++;
    }
    std::cout << "sharedStrings count: " << sharedStrings.count()
              << " rows: " << values.count() << " cols: " << maxColCount << std::endl;
    return true;
}

int SimpleXlsxReader::rowCount(int /*pageNum*/)
{
    return values.count();
}

int SimpleXlsxReader::columnCount(int /*pageNum*/)
{
    return maxColCount;
}

QString SimpleXlsxReader::cellValue(int row, int column)
{
    if (row>=values.count() || column>=maxColCount)
        return "INV";
    const QVariantList& rowV = values[row];
    if (column>=rowV.count())
        return "INV";
    else
        return rowV[column].toString();
}

QString SimpleXlsxReader::lastError()
{
    return error;
}

bool SimpleXlsxReader::loadInnerXml(QuaZip& arc, const QString& path, const QString& rootName, QDomDocument &doc, QDomElement& elRoot)
{
    if (!arc.setCurrentFile(path)) {
const QString S_ERR_SET_ARCH_ITEM = QString("err set it %1"); //===>
        error = S_ERR_SET_ARCH_ITEM.arg(path);
        return false;
    }
    QuaZipFile zf(&arc);
    if (!zf.open(QIODevice::ReadOnly)) {
const QString S_ERR_OPEN_ARCH_ITEM = QString("err op it %1"); //===>
        error = S_ERR_OPEN_ARCH_ITEM.arg(path);
        return false;
    }
    QString err_msg;
    int err_line, err_col;
    if (!doc.setContent(&zf, &err_msg, &err_line, &err_col)) {
const QString S_ERR_READ_CONTENT = QString("err rd content %1 m %2 l %3 c %4"); //===>
        error = S_ERR_READ_CONTENT.arg(path).arg(err_msg).arg(err_line).arg(err_col);
        return false;
    }
    zf.close();
    elRoot = doc.documentElement();
    if (elRoot.nodeName()!=rootName) {
        error = QObject::tr("Root node of %1 must be %2").arg(path).arg(rootName);
        return false;
    }
    return true;
}
