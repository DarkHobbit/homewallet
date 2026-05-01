/* Home Wallet
 *
 * Module: Report set
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef REPORTS_H
#define REPORTS_H

#include <QMap>
#include <QObject>
#include <QTextFormat>
#include <QTextTable>

#include "hwdatabase.h"
#include "reportsdata.h"

class Reports : public QObject
{
    Q_OBJECT
public:
    explicit Reports(QObject *parent = nullptr);
    bool findDuplicates(const QString& path,
        const QDate& dFrom, const QDate& dTo,
        int amountDelta, const ReportsData::Duplicates& duplicates);
    QString makeDocumentFormatFilters();
    void show(const QString &path);
protected:
    typedef QVector<int> IVec;
    QTextCharFormat frmHeader, frmSubHeader, frmCellHdr, frmCell;
    QTextBlockFormat fmtHeader;
    QMap<QString, QString> formatExtensions;
    bool usePercentColumnsWith;
    void adjustToFormat(const QString& path);
    void setTableColSizes(QTextTable* t, const IVec& sizes);
    void addCellText(QTextTable* t, int row, int col, const QString& text,
        const QTextCharFormat &charFormat = QTextCharFormat(), const QTextBlockFormat &blockFormat = QTextBlockFormat());
    void addCellHtml(QTextTable* t, int row, int col, const QString& text,
        const QTextBlockFormat &blockFormat = QTextBlockFormat());
    QString fromLowUnit(int lowUnitSum);

    void findOneDupKind(QTextCursor& c, const QString& header, const ReportsData::DupVector& dupVec);
signals:
};

#endif // REPORTS_H
