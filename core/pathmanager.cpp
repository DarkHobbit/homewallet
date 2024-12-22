/* Home Wallet
 *
 * Module: Program data path manager
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QCoreApplication>
#include <QDir>
#include "pathmanager.h"

PathManager::PathManager()
{
}

void PathManager::prepare()
{
    QString appDir = qApp->applicationDirPath();
#ifdef WIN32
    _transPath = appDir;
    _dbCreateScriptPath = appDir+"\\dbinit";
#elif defined(Q_OS_MAC)
    _transPath = appDir+"/../Translations";
    _dbCreateScriptPath = appDir+"/../DataBaseInit";
#else // Linux, BSD...
    if (appDir.contains("/usr/bin")) {
        // Standard case
        _transPath = "/usr/share/homewallet/translations";
        _dbCreateScriptPath = "/usr/share/homewallet/dbinit";
    }
    else {
        // Developer case
        if (QDir(appDir+"/../translations").exists()) {// in-source build
            _transPath = appDir+"/../translations";
            _dbCreateScriptPath = appDir+"/../core/dbinit";
        }
        else {
            _transPath = appDir+"/../../homewallet/translations"; // shadow build
            _dbCreateScriptPath = appDir+"/../../homewallet/core/dbinit";
        }
    }
#endif
}

QString PathManager::transPath()
{
    return _transPath;
}

QString PathManager::dbCreateScriptPath()
{
    return _dbCreateScriptPath;
}

PathManager pathManager;
