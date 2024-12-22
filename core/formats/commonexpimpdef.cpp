/* Home wallet
 *
 * Module: Common export/import definitions
 *
 * Copyright 2023 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include "commonexpimpdef.h"

QString prepareDoubleImport(const QString &s)
{
    QString res = s;
    res.replace(" ", "");
    res.replace(0xa0, ""); // NBSP, really present in HB XML number fields!
    res.replace(",", ".");
    return res;
}
