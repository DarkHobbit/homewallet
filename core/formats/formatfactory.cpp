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

#include "formatfactory.h"
#include "globals.h"
#include "txtcompactfile.h"
#include "xmlhbfile.h"

FormatFactory::FormatFactory()
    :error("")
{
    formats
        << new XmlHbFile()
        << new TxtCompactFile();
    // ...here add new formats
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
    allTypes << S_ALL_SUPPORTED.arg(allSupported);
    allTypes << S_ALL_FILES;
    return allTypes;
}

FileFormat *FormatFactory::createObject(const QString &url, QIODevice::OpenModeFlag mode)
{
    if (url.isEmpty()) {
        error = QObject::tr("Empty file name");
        return 0;
    }    
    for (FileFormat* ff: formats)
        if (ff->supportedModes().testFlag(mode)) {
            ff->clear();
            if (ff->detect(url))
                return ff;
        }
    // Sad but true
    error = QObject::tr("Unknown file format:\n%1").arg(url);
    return 0;
}
