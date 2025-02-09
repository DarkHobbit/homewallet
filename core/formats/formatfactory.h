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
#ifndef FORMATFACTORY_H
#define FORMATFACTORY_H

#include <QIODevice>
#include <QStringList>

#include "fileformat.h"

class FormatFactory
{
public:
    FormatFactory();
    ~FormatFactory();
    QStringList supportedFilters(QIODevice::OpenModeFlag mode, bool isReportFormat);
    FileFormat* createObject(const QString& url, QIODevice::OpenModeFlag mode);
    QString error;
private:
    QList<FileFormat*> formats;
};

#endif // FORMATFACTORY_H
