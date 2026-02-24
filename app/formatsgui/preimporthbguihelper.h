/* Home Wallet
 *
 * Module: Pre-import GUI helper for Home Bookkeeping (keepsoft.ru) file formats
 *
 * Copyright 2026 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
#ifndef PREIMPORTHBGUIHELPER_H
#define PREIMPORTHBGUIHELPER_H

#include "formats/fileformat.h"

class PreImportHbGuiHelper
{
public:
    PreImportHbGuiHelper();
    static bool check(const QString& path, FileFormat* impFile, HwDatabase& db);
};

#endif // PREIMPORTHBGUIHELPER_H
