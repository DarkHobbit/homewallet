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

#include <QDesktopServices>
#include <QMessageBox>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QUrl>

#include "globals.h"
#include "reports.h"

Reports::Reports(QObject *parent)
    : QObject{parent}
{
    frmHeader.setFontWeight(QFont::Bold);
    frmHeader.setFontFamily("Times");
    frmHeader.setFontPointSize(16);
    frmSubHeader.setFontWeight(QFont::Bold);
    frmSubHeader.setFontFamily("Times");
    frmSubHeader.setFontPointSize(14);
    frmCellHdr.setFontWeight(QFont::Bold);
    frmCellHdr.setFontPointSize(10);
    frmCell.setFontPointSize(10);

    fmtHeader.setAlignment(Qt::AlignHCenter);

    // Unfortunately, document format names in QTextDocumentWriter is not regular
    // Neither extension, nor MIME types (hardcode needed)
    formatExtensions["plaintext"] = "txt";
    formatExtensions["HTML"] = "html";
    formatExtensions["markdown"] = "md";
    formatExtensions["ODF"] = "odt";
}

void Reports::setTableColSizes(QTextTable* t, const IVec& sizes)
{
    QTextTableFormat tf;
    QVector<QTextLength> constraints;
    // QTextLength::PercentageLength don't work for ODF format
    for (int sz: sizes)
        constraints << QTextLength(QTextLength::FixedLength, sz);
    tf.setColumnWidthConstraints(constraints);
    t->setFormat(tf);
}

void Reports::addCellText(QTextTable* t, int row, int col, const QString& text,
    const QTextCharFormat &charFormat, const QTextBlockFormat &blockFormat)
{
    QTextCursor hc = t->cellAt(row, col).firstCursorPosition();
    hc.setBlockFormat(blockFormat);
    hc.insertText(text, charFormat);
}

void Reports::addCellHtml(QTextTable *t, int row, int col, const QString &text,
    const QTextBlockFormat &blockFormat)
{
    QTextCursor hc = t->cellAt(row, col).firstCursorPosition();
    hc.setBlockFormat(blockFormat);
    hc.insertHtml(text);
}

QString Reports::makeDocumentFormatFilters()
{
    QStringList res, exts;
    for (const QByteArray& f: QTextDocumentWriter::supportedDocumentFormats()) {
        QString sf(f);
        if (formatExtensions.keys().contains(sf)) {
            res += S_ONE_FILETYPE.arg(sf).arg(formatExtensions[sf]);
            exts += formatExtensions[sf];
        }
        else { // Unknown (for HomeWallet) file types
            res += S_ONE_FILETYPE.arg(sf).arg(sf);
            exts += sf;
        }
    }
    res << S_ALL_SUPPORTED.arg("*." + exts.join(" *."));
    res << S_ALL_FILES;
    return res.join(";;");
}

void Reports::findOneDupKind(QTextCursor &c, const QString &header, const ReportsData::DupVector &dupVec)
{
    c.setBlockFormat(fmtHeader);
    if (!dupVec.isEmpty()) {
        c.insertText(header, frmSubHeader);

        int cols = 5;
        if (!dupVec.hasSubCat && !dupVec.showSrc) cols = 4;
        int colDesc = cols-1;
        if (dupVec.showSrc) colDesc--;
        QTextTable* t = c.insertTable(dupVec.totalCount()+1, cols);
        if (dupVec.showSrc) {
            setTableColSizes(t, IVec() << 80 << 70 << 170 << 165 << 165);
        }
        else {
            if (dupVec.hasSubCat)
                setTableColSizes(t, IVec() << 80 << 70 << 140 << 140 << 220);
            else
                setTableColSizes(t, IVec() << 80 << 70 << 170 << 330);
        }
        addCellText(t, 0, 0, S_COL_DATE, frmCellHdr, fmtHeader);
        addCellText(t, 0, 1, S_COL_SUM, frmCellHdr, fmtHeader);
        addCellText(t, 0, 2, S_COL_CATEGORY, frmCellHdr, fmtHeader);
        if (dupVec.hasSubCat && !dupVec.showSrc)
            addCellText(t, 0, 3, S_COL_SUBCATEGORY, frmCellHdr, fmtHeader);
        addCellText(t, 0, colDesc, S_COL_DESCRIPTION, frmCellHdr, fmtHeader);
        if (dupVec.showSrc)
            addCellText(t, 0, colDesc+1, S_COL_SOURCE, frmCellHdr, fmtHeader);

        int row = 1;
        for (const ReportsData::DupSet& set: dupVec)
        {
            t->mergeCells(row, 0, set.dups.count(), 1);
            t->mergeCells(row, 2, set.dups.count(), 1);
            if (dupVec.hasSubCat && !dupVec.showSrc)
                t->mergeCells(row, 3, set.dups.count(), 1);
            addCellText(t, row, 0, set.d.toString(gd.dateFormat), frmCell);
            if (dupVec.showSrc) {
                QString cat = set.catName;
                if (!set.subcatName.isEmpty())
                    cat += " /<br>" + set.subcatName;
                addCellHtml(t, row, 2, cat);
            }
            else {
                addCellText(t, row, 2, set.catName, frmCell);
                if (dupVec.hasSubCat)
                    addCellText(t, row, 3, set.subcatName, frmCell);
            }
            for (const ReportsData::DupInfo& dup : set.dups)
            {
                addCellText(t, row, 1, QString::number((float)dup.sum/100), frmCell);
                addCellText(t, row, colDesc, dup.descr, frmCell);
                if (dupVec.showSrc)
                    addCellText(t, row, colDesc+1, dup.src, frmCell);
                row++;
            }
        }
    }
    else
        c.insertText(header + " - " + S_DUP_NOT_FOUND, frmSubHeader);
    c.movePosition(QTextCursor::End);
    c.insertBlock();
}

bool Reports::findDuplicates(const QString &path,
    const QDate &dFrom, const QDate &dTo,
    int amountDelta, const ReportsData::Duplicates &duplicates)
{
    QTextDocument doc;
    QTextCursor c(&doc);
    c.setBlockFormat(fmtHeader);

    c.insertText(tr("Report on duplicates"), frmHeader);
    c.insertBlock();
    c.insertText(tr("From %1 to %2; amount delta %3")
        .arg(dFrom.toString(gd.dateFormat))
        .arg(dTo.toString(gd.dateFormat))
        .arg((float)amountDelta/100), frmCell);
    c.insertBlock();
    c.insertBlock();

    findOneDupKind(c, S_DK_EXPENSES, duplicates.expenses);
    findOneDupKind(c, S_DK_INCOMES, duplicates.incomes);
    findOneDupKind(c, S_DK_TRANSFER, duplicates.transfer);

    c.insertBlock();
    c.insertText(duplicates.stat(), frmSubHeader);
    c.insertBlock();

    QTextDocumentWriter w(path);
    if (!w.write(&doc)) {
        QMessageBox::critical(0, S_ERROR, S_WRITE_ERR.arg(path));
        return false;
    }
    return true;
}

void Reports::show(const QString &path)
{
    QDesktopServices::openUrl(QUrl(QString("file://")+path)); // TODO for Windows file:///
}
