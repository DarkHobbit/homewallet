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

#include <QSqlQuery>
#include "categoriesbasedquerymodel.h"

CategoriesBasedQueryModel::CategoriesBasedQueryModel(QObject *parent)
    : FilteredQueryModel(parent), idCat(-2), idSubcat(-2)
{
}

void CategoriesBasedQueryModel::setFilterCategories(int _idCat, int _idSubcat)
{
    idCat = _idCat;
    idSubcat = _idSubcat;
}

void CategoriesBasedQueryModel::makeFilter()
{
    FilteredQueryModel::makeFilter();
    // -1 without subcategories, -2 all categories or subcategories
    // c must be alias for categories table and sc must be alias for subcategories table
    // See SQL text in update() in subclasses
    if (idCat==-2)
        return;
    else if (idSubcat==-2)
        filters << QString("c.id=%1").arg(idCat);
    else
        filters << QString("sc.id=%1").arg(idSubcat);
}
