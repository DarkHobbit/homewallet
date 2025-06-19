/* Home Wallet
 *
 * Module: Expenses query SQL model
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef EXPENSEMODEL_H
#define EXPENSEMODEL_H

#include "categoriesbasedquerymodel.h"

class ExpenseModel : public CategoriesBasedQueryModel
{
    Q_OBJECT
public:
    ExpenseModel(QObject *parent);
    virtual void setDefaultVisibleColumns();
    virtual void update();
    virtual QString localizedName();
};

#endif // EXPENSEMODEL_H
