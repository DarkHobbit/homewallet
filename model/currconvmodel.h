/* Home Wallet
 *
 * Module: Currency conversion (exchange) query SQL model
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
#ifndef CURRCONVMODEL_H
#define CURRCONVMODEL_H

#include "filteredquerymodel.h"

class CurrConvModel : public FilteredQueryModel
{
public:
    explicit CurrConvModel(QObject *parent);
    void update();
};

#endif // CURRCONVMODEL_H
