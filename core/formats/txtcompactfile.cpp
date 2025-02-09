/* Home wallet
 *
 * Module: Compact Text File (see ???) export/import
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QRegExp>
#include <QTextStream>
#include "txtcompactfile.h"

#define S_ERR_FIRST_DATE \
QObject::tr("First non-empty string in file must be full (yyMMdd) date.\nFound: %1")


TxtCompactFile::TxtCompactFile() {}

QStringList TxtCompactFile::supportedExtensions()
{
    return (QStringList() << "txt" << "TXT");
}

QStringList TxtCompactFile::supportedFilters()
{
    return (QStringList() << QObject::tr("Compact text file (*.txt *.TXT)"));
}

QIODevice::OpenMode TxtCompactFile::supportedModes()
{
    return QIODevice::ReadOnly;
}

bool TxtCompactFile::detect(const QString &path)
{
    if (!openFile(path, QIODevice::ReadOnly))
        return false;    
    QTextStream ss(&file);
    QString s;
    do {
        s = ss.readLine().trimmed();
    } while (!ss.atEnd() && s.isEmpty());
    bool res = s.startsWith(":") && s.endsWith(":");
    closeFile();
    return res;
}

QString TxtCompactFile::formatAbbr()
{
    return "TXTCF";
}

bool TxtCompactFile::importRecords(const QString &path, HwDatabase &db)
{
    if (!openFile(path, QIODevice::ReadOnly))
        return false;
    candidates.clear();
    QTextStream ss(&file);
    QString s;
    QDate lastDate;
    QRegExp reFullDate(":\d\d\d\d\d\d:");
    QRegExp reOnlyDay(":(\d\d):");
    // Тип, СумЦел, СумДроб, Источ, Кат+Подкат,  КолЦел, КолДроб, Хвост
    QRegExp reIncExp("(\\+?)(\\d+)(?:,|.?)(\\d*)(@\\S+)?(?:\\s+)(\\D+)(?:\\s+)(\\d+)(?:,|.?)(\\d*)(.*)?");
    // TODO try add currency in common regexp
    do {
        s = ss.readLine().trimmed();
        if (s.isEmpty() || s.startsWith("#"))
            continue;
        // Date for some next lines
        if (reFullDate.exactMatch(s)) {
            lastDate = QDate::fromString(s, ":yyMMdd:");
            if (!lastDate.isValid()) {
                _fatalError = QObject::tr("Invalid date: %1").arg(s);
                closeFile();
                return false;
            }
            continue;
        }
        else if (reOnlyDay.exactMatch(s)) {
            if (lastDate.isNull()) {
                _fatalError = S_ERR_FIRST_DATE.arg(s);
                closeFile();
                return false;
            }
            lastDate = QDate(lastDate.year(), lastDate.month(), reOnlyDay.cap().toInt());
            if (!lastDate.isValid()) {
                _fatalError = QObject::tr("Invalid day for this month: %1, last date was %2")
                    .arg(s).arg(lastDate.toString());
                closeFile();
                return false;
            }
            continue;
        }
        // Before money lines, at least one FULL date must be
        if (lastDate.isNull()) {
            _fatalError = S_ERR_FIRST_DATE.arg(s);
            closeFile();
            return false;
        }
        // Expense or income without currency
        // Expense or income with currency
        // Transfer
        // TODO Other item types

        //
    } while (!ss.atEnd());

    //
    closeFile();
    return true;
}
