/* Home wallet
 *
 * Module: HomeWallet XML file export/import
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef XMLHWFILE_H
#define XMLHWFILE_H

#include "xmlfile.h"

class XmlHwFile : public XmlFile
{
public:
    XmlHwFile();
    virtual bool detect(const QString &path);
    virtual QIODevice::OpenMode supportedModes();
    virtual QStringList supportedFilters();
    virtual SubTypeFlags supportedExportSubTypes();
    virtual QString formatAbbr();
    virtual bool isDialogRequired();
    virtual bool importRecords(const QString &path, HwDatabase& db);
};

#endif // XMLHWFILE_H
