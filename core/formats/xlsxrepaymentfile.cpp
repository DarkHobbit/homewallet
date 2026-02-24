/* Home Wallet
 *
 * Module: Home Bookkeeping (keepsoft.ru) XLSX repayment file
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include "commonexpimpdef.h"
#include "xlsxrepaymentfile.h"
#include "simplexlsxreader.h"

XlsxRepaymentFile::XlsxRepaymentFile()
    :hb(&_fatalError), idCrd(-1),
      estimatedOpDate(QDateTime()), estimatedCorrName("")
{}

void XlsxRepaymentFile::clear()
{
    FileFormat::clear();
    idCrd = -1;
    estimatedOpDate = QDateTime();
    estimatedCorrName = "";
}

bool XlsxRepaymentFile::detect(const QString &path)
{
    SimpleXlsxReader reader(path);
    CUST_CHK(reader.read(), reader.lastError())
    // Check for sheet sizes
    CUST_CHK((reader.rowCount(0)>=11 && reader.columnCount(0)==15),
        QObject::tr("Home Bookkeeping XLSX repayment sheet size must be at least 15x11"));
    // Check for keepsoft.ru url at G column
    QString urlKs = reader.cellValue(reader.rowCount(0)-1, 6);
    CUST_CHK(urlKs.contains("keepsoft.ru"),
        _fatalError = QObject::tr("URL keepsoft.ru not found"));
    // Get date and correspondent
    QString credSummary = reader.cellValue(2, 0);
    int lfIndex = credSummary.indexOf("\n");
    if (lfIndex>-1)
        credSummary = credSummary.left(lfIndex);
    // Datemarker: date. CorrNameMarker: CorrName
    // Depends of l12n. Example for ru_RU: Дата: dd.mm.yyyy. Имя кредитора: Банк Финсервис
    QRegExp reDateAndCorr("[^:]+:\\s(\\S+)\\.\\s[^:]+:\\s(.*)");
    if (reDateAndCorr.exactMatch(credSummary)) {
        estimatedOpDate = QDateTime::fromString(reDateAndCorr.cap(1), "dd.MM.yyyy");
        estimatedCorrName = reDateAndCorr.cap(2);
    }
    return true;
}

QIODevice::OpenMode XlsxRepaymentFile::supportedModes()
{
    return QIODevice::ReadOnly;
}

QStringList XlsxRepaymentFile::supportedExtensions()
{
    return (QStringList() << "xlsx" << "XLSX");
}

QStringList XlsxRepaymentFile::supportedFilters()
{
    return QStringList() << QObject::tr("Home Bookkeeping Repayment XLSX (*.xlsx *.XLSX)");
}

FileFormat::SubTypeFlags XlsxRepaymentFile::supportedExportSubTypes()
{
    return FileFormat::SubTypeFlags();
}

QString XlsxRepaymentFile::formatAbbr()
{
    return "XXHBKRP";
}

bool XlsxRepaymentFile::isDialogRequired()
{
    return false;
}

void XlsxRepaymentFile::setCredId(int _idCrd)
{
    idCrd = _idCrd;
}

QDateTime XlsxRepaymentFile::getEstimatedOpDate()
{
    return estimatedOpDate;
}

QString XlsxRepaymentFile::getEstimatedCorrName()
{
    return estimatedCorrName;
}

bool XlsxRepaymentFile::importRecords(const QString &path, HwDatabase &db)
{
    SimpleXlsxReader reader(path);
    CUST_CHK(reader.read(), reader.lastError())
    for (int row=5; row<reader.rowCount(0)-6; row++) {
        // Date
        QString sDt = reader.cellValue(row, 0);
        QDateTime dt = QDateTime::fromString(sDt, "dd.MM.yyyy");
        CUST_CHK(dt.isValid(),S_ERR_DATE_IMP.arg(sDt));
        // Account
        QString accName = reader.cellValue(row, 2);
        int idAcc = db.accountId(accName);
        CUST_CHK(idAcc>-1, S_ERR_ACC_NOT_FOUND.arg(accName));
        // Amount
        QString sSum = prepareDoubleImport(reader.cellValue(row, 7));
        QString moneyChar;
        int sum = hb.importOneMoneyAttr(sSum, moneyChar);
        UP_CHK(_fatalError.isEmpty());
        // Currency
        int idCur = hb.importCurrencyByChar(moneyChar, db);
        UP_CHK(idCur>-1);
        // Description
        QString desc = reader.cellValue(row, 11);
        // Insert!
        DB_CHK(db.addRepayment(idCrd, dt, sum, idAcc, idCur, desc,
             _idImp, QString::number(row)));
        _processedRecordsCount++;
    }
    return true;
}

bool XlsxRepaymentFile::exportRecords(const QString&, HwDatabase&, SubTypeFlags)
{
    return false;
}
