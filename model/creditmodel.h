/* Home Wallet
 *
 * Module: Credit (lend and borrow) query SQL model
 *
 * Copyright 2025 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef CREDITMODEL_H
#define CREDITMODEL_H

#include "filteredquerymodel.h"

class CreditModel : public FilteredQueryModel
{
    Q_OBJECT
public:
    explicit CreditModel(QObject *parent, bool _isLend);
    virtual void setDefaultVisibleColumns();
    virtual void update();
    virtual QString localizedName();
private:
    bool isLend;
};

#endif // CREDITMODEL_H
