/* Home wallet
 *
 * Module: Abstract class for file export/import format
 *
 * Copyright 2016 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef FILEFORMAT_H
#define FILEFORMAT_H

#include <QFile>
#include "hwdatabase.h"
#include <QStringList>

class FileFormat
{
public:
    FileFormat();
    virtual ~FileFormat();
    virtual void clear();
    virtual bool detect(const QString &path)=0;
    virtual QIODevice::OpenMode supportedModes()=0;
    virtual QStringList supportedExtensions()=0;
    virtual QStringList supportedFilters()=0;
    virtual QString formatAbbr()=0;
    virtual bool isDialogRequired()=0;
    virtual bool importRecords(const QString &path, HwDatabase& db)=0;
    virtual bool postImport(HwDatabase&);
    void setIdImp(int idImp);
    QStringList errors();
    QString fatalError();
    int importedRecordsCount();
    int totalRecordsCount();
    static void lossData(QStringList& errors, const QString& recName,
        const QString& fieldName, bool condition);
    static void lossData(QStringList& errors, const QString& recName,
        const QString& fieldName, const QString& field);
protected:
    int _idImp;
    QFile file;
    QStringList _errors;
    QString _fatalError;
    int _importedRecordsCount, _totalRecordsCount;
    bool openFile(QString path, QIODevice::OpenMode mode);
    void closeFile();
};

#endif // FILEFORMAT_H
