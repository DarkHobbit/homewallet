/* Home Wallet
 *
 * Module: Semi-Base class for SQL model with categories and subcategories in filter
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef CABAQUMODEL_H
#define CABAQUMODEL_H

#include "filteredquerymodel.h"

class CategoriesBasedQueryModel : public FilteredQueryModel
{
    Q_OBJECT
public:
    CategoriesBasedQueryModel(QObject *parent);
    void setFilterCategories(int _idCat, int _idSubcat=-1);
protected:
    int idCat, idSubcat; // -1 without subcat, -2 all cat/subcat
    virtual void makeFilter();
};

#endif // CABAQUMODEL_H
