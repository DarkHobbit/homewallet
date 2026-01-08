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
#define S_ERR_INT_IMP QObject::tr("Can't convert string to integer: %1")
#define S_ERR_FLOAT_IMP QObject::tr("Can't convert string to float: %1")
#define S_ERR_QTY_IMP QObject::tr("Can't convert quantity to number: %1")
#define S_ERR_AMO_IMP QObject::tr("Can't convert amount to number: %1")
#define S_ERR_PERC_IMP QObject::tr("Can't convert percents to number: %1")

#define S_ERR_CUR_NOT_FOUND QObject::tr("Currency not found: %1")
#define S_ERR_UNIT_NOT_FOUND QObject::tr("Unit not found: %1")
#define S_ERR_ACC_NOT_FOUND QObject::tr("Account not found: %1")
#define S_ERR_ATTR_NOT_FOUND QObject::tr("Attribute %1 not found at line %2")

#define S_ERR_NO_SUBCAT QObject::tr("At first, define subcategory for this record")
#define S_ERR_UNIT_ISNT_EMPTY QObject::tr("Unit isn't empty")
#define S_ERR_NOT_INC_NOT_EXP QObject::tr("It isn't neither income, nor expense")
#define S_ERR_NO_UNIT QObject::tr("No units found in database")

#define S_CONFIRM_DEF_UNIT_EXIST \
    QObject::tr("Subcategory %1 already have default unit: %2. Are you really want replace it?")

// Common import queries
#define SQL_GET_DEF_EXP_UNIT \
    "select un.short_name, un.id" \
    " from hw_ex_subcat sc, hw_unit un" \
    " where sc.id_un_default=un.id and sc.id=:id"

#define SQL_UNIT_COUNTS \
    "select short_name as unname," \
    " (select count(id) from hw_ex_op where id_esubcat=:id_sc and id_un=hw_unit.id) as cnt " \
    " from hw_unit" \
    " where cnt>0" \
    " order by cnt desc"

#define SQL_ALL_UNITS \
    "select short_name, id from hw_unit order by short_name"

#define SQL_SET_DEF_EXP_UNIT \
    "update hw_ex_subcat set id_un_default=:id_un where id=:id_sc"

// At least for Home Bookkeeping XML file
QString prepareDoubleImport(const QString& s);

#endif // COMMONEXPIMPDEF_H
