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

#ifndef COMMONEXPIMPDEF_H
#define COMMONEXPIMPDEF_H

#include <QObject>
#include <QString>

// Common import errors
#define S_ERR_DATE_IMP QObject::tr("Can't convert date string to date: %1")
#define S_ERR_QTY_IMP QObject::tr("Can't convert quantity to number: %1")
#define S_ERR_AMO_IMP QObject::tr("Can't convert amount to number: %1")
#define S_ERR_CUR_NOT_FOUND QObject::tr("Currency not found: %1")
#define S_ERR_ACC_NOT_FOUND QObject::tr("Account not found: %1")

// At least for Home Bookkeeping XML file
QString prepareDoubleImport(const QString& s);

#endif // COMMONEXPIMPDEF_H
