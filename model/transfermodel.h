/* Home Wallet
 *
 * Module: Incter-account transfer query SQL model
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
#ifndef TRANSFERMODEL_H
#define TRANSFERMODEL_H

#include "categoriesbasedquerymodel.h"

class TransferModel : public CategoriesBasedQueryModel
{
public:
    TransferModel(QObject *parent);
    void update();
};

#endif // TRANSFERMODEL_H
