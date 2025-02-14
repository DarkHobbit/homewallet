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

#ifndef TXTCOMPACTFILE_H
#define TXTCOMPACTFILE_H

#include "interactiveformat.h"

class TxtCompactFile : public InteractiveFormat
{
public:
    TxtCompactFile();
    virtual QStringList supportedExtensions();
    virtual QStringList supportedFilters();
    virtual QIODevice::OpenMode supportedModes();
    virtual bool detect(const QString &path);
    virtual QString formatAbbr();
    virtual bool importRecords(const QString &path, HwDatabase& db);
private:
    int captureMoneySum(const QString& highPart, const QString& lowPart, bool& ok);
    double captureDouble(const QString& highPart, const QString& lowPart, bool& ok);
};

#endif // TXTCOMPACTFILE_H
