/* Home wallet
 *
 * Module: Global definitions
 *
 * Copyright 2016 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QDate>
#include <QHash>
#include <QObject>
#include <QStringList>
#include <qnamespace.h>

// Common strings for translation unify
// Message boxes
#define S_ERROR QObject::tr("Error")
#define S_WARNING QObject::tr("Warning")
#define S_CONFIRM QObject::tr("Confirmation")
#define S_INFORM QObject::tr("Information")
// File ops
#define S_ALL_SUPPORTED QObject::tr("All supported files (%1)")
#define S_ALL_FILES QObject::tr("All files (*.*)")
// Common errors, warnings and questions
#define S_READ_ERR QObject::tr("Can't read file\n%1")
#define S_WRITE_ERR QObject::tr("Can't write file\n%1")
#define S_EMPTY_FILE QObject::tr("No records found in file\n%1")
#define S_MKDIR_ERR QObject::tr("Can't create directory\n%1")
#define S_SEEK_ERR QObject::tr("Can't seek to %1 in file\n%2")
#define S_REC_NOT_SEL QObject::tr("Record not selected")
#define S_ONLY_ONE_REC QObject::tr("Group operation not implemented, select one record")
#define S_READ_ONLY_FORMAT QObject::tr("This format is read only")
#define S_ERR_UNSUPPORTED_TAG \
    QObject::tr("Warning: record %1 has %2, not supported in this format.\nData will be lost")
#define S_ERR_EXTRA_TAG \
    QObject::tr("%1 %2 will be lost at record %3")
// Status messages
#define SS_MODE QObject::tr("Mode: ")
#define SS_SORT_ON QObject::tr("sorted")
#define SS_SORT_OFF QObject::tr("not sorted")
// Model item roles
#define SortStringRole Qt::UserRole
// Pseudo-name for all categories/subcategoried
#define S_ALL_CAT QObject::tr("<All>")

struct Account {
    QDate openDate;
    QString name;
};

extern
struct GlobalConfig {
    // View
    bool showTableGrid;
    bool showLineNumbers;
    bool resizeTableRowsToContents;
    bool useTableAlternateColors;
    bool useSystemFontsAndColors;
    QString tableFont, gridColor1, gridColor2;
    // Locale
    QString dateFormat, timeFormat;
    bool useSystemDateTimeFormat;
    // TODO
    // Session-specific data from command line
    bool fullScreenMode; // Maximize main window at startup
    bool debugDataMode;  // Create debug data at startup
} gd;

#endif // GLOBALS_H
