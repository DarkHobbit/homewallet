/* Home wallet
 *
 * Module: Creator of file export/import format classes
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
//#include <QFileInfo>
#include <QObject>
#include <QStringList>

#include "formatfactory.h"
#include "globals.h"
#include "txtcompactfile.h"
#include "xmlhbfile.h"
#include "xmlhwfile.h"
#include "xlsxrepaymentfile.h"

FormatFactory::FormatFactory()
    :error("")
{
    formats
        << new XmlHbFile()
        << new XmlHwFile()
        << new TxtCompactFile()
        << new XlsxRepaymentFile();
    // ...here add new formats
    for (FileFormat* ff: formats)
        for (const QString& filter: ff->supportedFilters())
            formatsByFilter[filter] = ff;
}

FormatFactory::~FormatFactory()
{
    for (FileFormat* ff: formats)
        delete ff;
}

QStringList FormatFactory::supportedFilters(QIODevice::OpenModeFlag mode, bool isReportFormat)
{
    QStringList allTypes;
    // Known formats
    QString allSupported;
    for (FileFormat* ff: formats)
        if (ff->supportedModes().testFlag(mode)) {
            allSupported += "*." + ff->supportedExtensions().join(" *.");
            allTypes << ff->supportedFilters();

        }
    if (mode==QIODevice::ReadOnly) {
        allTypes.insert(0, S_ALL_SUPPORTED.arg(allSupported));
        allTypes << S_ALL_FILES;
    }
    return allTypes;
}

FileFormat *FormatFactory::getObject(const QString &url, QIODevice::OpenModeFlag mode)
{
    if (url.isEmpty()) {
        error = QObject::tr("Empty file name");
        return 0;
    }
    QStringList fmtErrors;
    for (FileFormat* ff: formats)
        if (ff->supportedModes().testFlag(mode)) {
            ff->clear();
            if (ff->detect(url))
                return ff; // success
            if (matchExtension(url, ff))
                fmtErrors << ff->supportedFilters().join("; ") + ": " + ff->fatalError();
        }
    // Sad but true
    if (fmtErrors.isEmpty())
        error = QObject::tr("Unknown file format:\n%1").arg(url);
    else
        error = S_READ_ERR.arg(url) + "\n\n"
                + QObject::tr("Candidates error messages:") + "\n"
                + fmtErrors.join("\n");
    return 0;
}

FileFormat *FormatFactory::formatByFilter(const QString &filter)
{
    if (formatsByFilter.keys().contains(filter)) {
        formatsByFilter[filter]->clear();
        return formatsByFilter[filter];
    }
    else
        return 0;
}

bool FormatFactory::matchExtension(const QString &path, FileFormat *ff)
{
    for (const QString& candidate: ff->supportedExtensions())
        if (path.endsWith(candidate))
            return true;
    return false;
}
