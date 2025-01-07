/* Home Wallet
 *
 * Module: Widget helpers
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include "globals.h"
#include "helpers.h"

void fillComboByDict(QComboBox *combo, GenericDatabase::DictColl coll, bool addAllItem)
{
    combo->clear();
    if (addAllItem)
        combo->addItem(S_ALL_CAT, -2);
    for (const QString& key: coll.keys())
        combo->addItem(key, coll[key]);
}

int getComboCurrentId(QComboBox *combo)
{
    return combo->itemData(combo->currentIndex()).toInt();
}
