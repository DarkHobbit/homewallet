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

#include <iostream>
bool TxtCompactFile::importRecords(const QString &path, HwDatabase &db)
{
    if (!openFile(path, QIODevice::ReadOnly))
        return false;
    candidates.clear();
    QTextStream ss(&file);
    QString s;
    QDate lastDate;
    QRegExp reFullDate(":\\d\\d\\d\\d\\d\\d:");
    QRegExp reOnlyDay(":(\\d\\d?):");
    // Тип, СумЦел, СумДроб, Источ, Кат+Подкат,  КолЦел, КолДроб, Хвост
    // QRegExp reIncExp("(\\+?)(\\d+)(?:,|\\.?)(\\d*)(@\\S+)?(?:\\s+)(\\D+)(?:\\s+)(\\d+)(?:,|.?)(\\d*)(.*)?");
    // Тип, СумЦел, СумДроб, Валюта, Источ, Кат+Подкат,  КолЦел, КолДроб, Хвост
    QRegExp reIncExp("(\\+?)(\\d+)(?:,|\\.?)(\\d*)(\\:\\S+)?(@\\S+)?(?:\\s+)(\\D+)(?:\\s+)(\\d+)(?:,|.?)(\\d*)(.*)?");
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
            lastDate = QDate(lastDate.year(), lastDate.month(), reOnlyDay.cap(1).toInt());
            if (!lastDate.isValid()) {
                _fatalError = QObject::tr("Invalid day for this month: %1, last date was %2")
                    .arg(reOnlyDay.cap(1)).arg(lastDate.toString());
                closeFile();
                return false;
            }
            continue;
        }
        else

        // Before money lines, at least one FULL date must be
        if (lastDate.isNull()) {
            _fatalError = S_ERR_FIRST_DATE.arg(s);
            closeFile();
            return false;
        }
        ImpRecCandidate c;
        c.source = s;
        QString sNum;
        bool ok;
        // Expense or income without currency
        if (reIncExp.exactMatch(s)) {
            std::cout << reIncExp.cap(1).toUtf8().data() << "||"
                      << reIncExp.cap(2).toUtf8().data() << "||"
                      << reIncExp.cap(3).toUtf8().data() << "||"
                      << reIncExp.cap(4).toUtf8().data() << "||"
                      << reIncExp.cap(5).toUtf8().data() << "||"
                      << reIncExp.cap(6).toUtf8().data() << "||"
                      << reIncExp.cap(7).toUtf8().data() << "||"
                      << reIncExp.cap(8).toUtf8().data()
                      << std::endl;
            c.type = (reIncExp.cap(3)=="+")
                ? ImpRecCandidate::Income : ImpRecCandidate::Expense;
            sNum = reIncExp.cap(3); // process 35 and 35,3 and 35,45
            switch (sNum.length()) {  // Align to integer in low units (cent, kopeck, pfennig etc.)
            case 0:
                sNum = "00";
                break;
            case 1:
                sNum += "0";
                break;
            case 2:
                break;
            default:
                sNum = sNum.left(2);
                _errors << QObject::tr("Too long money sum fractional part: %1,%2")
                    .arg(reIncExp.cap(2)).arg(reIncExp.cap(3));
                break;
            }
            sNum = reIncExp.cap(2)+sNum;
            c.amount = sNum.toInt(&ok);
            if (!ok)
                c.state = ImpRecCandidate::ParseError;
            else {

                // TODO
                c.state = ImpRecCandidate::ReadyToImport; //==>
            }
        }
        // Expense or income with currency
        // Transfer
        // TODO Other item types
        // Unrecognized line
        else
            c.state = ImpRecCandidate::ParseError;
        candidates << c;
    } while (!ss.atEnd());
    // Debug
    for (const ImpRecCandidate& c: candidates)
        std::cout << "[" << c.source.toUtf8().data() << "] st "
                  << c.state << " sum " << c.amount << std::endl;
    // Done
    closeFile();
    return true;
}
