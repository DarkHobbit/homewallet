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

#ifndef XLSXREPAYMENTFILE_H
#define XLSXREPAYMENTFILE_H

#include "fileformat.h"
#include "hbhelper.h"

#define S_ERR_NO_CREDITS \
  QObject::tr("No credit or debt record in database to bind this file")
class XlsxRepaymentFile : public FileFormat
{
public:
    XlsxRepaymentFile();
    virtual void clear();
    virtual bool detect(const QString &path);
    virtual QIODevice::OpenMode supportedModes();
    virtual QStringList supportedExtensions();
    virtual QStringList supportedFilters();
    virtual SubTypeFlags supportedExportSubTypes();
    virtual QString formatAbbr();
    virtual bool isDialogRequired();
    virtual bool importRecords(const QString &path, HwDatabase& db);
    virtual bool exportRecords(const QString &path, HwDatabase& db, FileFormat::SubTypeFlags subTypes);
    void setCredId(int _idCrd);
    QDateTime getEstimatedOpDate(); // from input file
    QString getEstimatedCorrName(); // -"-
private:
    HbHelper hb;
    int idCrd;
    QDateTime estimatedOpDate; // from input file
    QString estimatedCorrName; // -"-
};

#endif // XLSXREPAYMENTFILE_H
