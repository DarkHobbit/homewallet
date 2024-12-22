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

#ifndef PATHMANAGER_H
#define PATHMANAGER_H

#include <QString>

class PathManager
{
public:
    PathManager();
    void prepare();
    QString transPath();
    QString dbCreateScriptPath();
private:
    QString _transPath;
    QString _dbCreateScriptPath;
};

extern PathManager pathManager;

#endif // PATHMANAGER_H
