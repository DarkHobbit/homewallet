/* Home Wallet
 *
 * Module: Configuration manager
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QtGlobal>
#include <QSettings>
#include <QStringList>

#include "corehelpers.h"
#ifdef WITH_MESSAGES
#include "decodedmessagelist.h"
#endif

class ConfigManager
{
public:
    ConfigManager();
    ~ConfigManager();
    void prepare();
    // Common configuration, managed by config dialog
    void setDefaults(const QString& tableFont, const QString& gridColor1, const QString& gridColor2);
    void readConfig();
    void writeConfig();
    // Separate settings, managed by main window and other dialogs
    QString readLanguage();
    void writeLanguage(const QString& language);
    QString lastImportedFile();
    void setLastImportedFile(const QString& path);
    QString localDatabaseDir();
    void setLocalDatabaseDir(const QString& dir);
    static QString defaultDocDir();
    void updateFormats();
private:
    QSettings* settings;
};

extern ConfigManager configManager;

#endif // CONFIGMANAGER_H
