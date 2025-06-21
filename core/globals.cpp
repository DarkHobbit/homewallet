/* Home wallet
 *
 * Module: Global definitions
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QtGlobal>
#include <QObject>
#include "globals.h"

GlobalConfig gd;

// Enumerations
EnumSetting enFilterDatesOnStartup {
    "Filter", "DatesOnStartup",
    "ShowAllRecords;RestorePrevRange;ShowLastNMonths",
    2
};
