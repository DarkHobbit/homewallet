/* Home Wallet
 *
 * Module: Money application database wrapper
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QDir>
#include <QSqlError>
#include <QVariant>
//#include <iostream>

#include "hwdatabase.h"
#include "pathmanager.h"

HwDatabase::HwDatabase()
    :GenericDatabase()
{
}

HwDatabase::~HwDatabase()
{
}

QString HwDatabase::fileName()
{
    return "homewallet.sqlite";
}

void HwDatabase::getCounts(int &totalInCount, int &totalExpCount)
{
    QSqlQuery q(sqlDb);
    q.prepare("select count(id) from hw_in_op");
    q.exec();
    q.first();
    totalInCount = q.value(0).toInt();
    q.prepare("select count(id) from hw_ex_op");
    q.exec();
    q.first();
    totalExpCount = q.value(0).toInt();
}

HwDatabase::DBFileState HwDatabase::test(const QString &dir)
{
    if (!open(dir))
        return OpenError;
    if (!checkTablePresence("hw_currency")) {
        close();
        return Alien;
    }
    // TODO if obsolete, must post reason in _lastError
    close();
    return Actual;
}

bool HwDatabase::upgrade(const QString &dir)
{
    // TODO
    return true;
}

bool HwDatabase::create(const QString &dir)
{
    // Open empty database
    if (!open(dir))
        return false;
    // Load SQL files
    if (!loadSqlFile(pathManager.dbCreateScriptPath()+QDir::separator()+"dbinit.sql"))
        return false;
    if (!loadSqlFile(pathManager.dbCreateScriptPath()+QDir::separator()+"loadcurrency.sql"))
        return false;
    // Close before reopen
    close();
    return true;
}

bool HwDatabase::isEmpty()
{
    if (!isTableEmpty("hw_account"))
        return false;
    if (!isTableEmpty("hw_in_cat"))
        return false;
    if (!isTableEmpty("hw_ex_cat"))
        return false;
    if (!isTableEmpty("hw_transfer_type"))
        return false;
    // TODO other categories / currencies / non-account data
    return true;
}

int HwDatabase::addImportFile(const QString &fileName, const QString &fileType)
{
    QSqlQuery sqlIns(sqlDb);
    sqlIns.prepare(
        "insert into hw_imp_file (imp_date, filename, filetype) values (:imp_date, :filename, :filetype)");
    sqlIns.bindValue(":imp_date", QDateTime::currentDateTime());
    sqlIns.bindValue(":filename", fileName);
    sqlIns.bindValue(":filetype", fileType);
    if (!execQuery(sqlIns)) {
        return -1;
    }
    return findImportFile(fileName);
}

int HwDatabase::findImportFile(const QString &fileName)
{
    QSqlQuery sqlSel(sqlDb);
    sqlSel.prepare("select id from hw_imp_file where filename=:filename");
    sqlSel.bindValue(":filename", fileName);
    return dictId(sqlSel);
}

bool HwDatabase::addAccount(const QString &name, const QString &descr,
    const QDateTime& foundation, const MultiCurrById& startBalance)
{
    QSqlQuery sqlIns(sqlDb);
    sqlIns.prepare(
      "insert into hw_account (name, descr, foundation) values (:name, :descr, :foundation)");
    sqlIns.bindValue(":name", name);
    sqlIns.bindValue(":descr", descr);
    sqlIns.bindValue(":foundation", dateOrNull(foundation));    
    if (!execQuery(sqlIns))
        return -1;
    int idAcc = accountId(name);
    // Insert start balance
    for (int idCur: startBalance.keys()) {
        sqlIns.prepare(
            "insert into hw_acc_init(id_ac, id_cur, init_sum) values (:id_ac, :id_cur, :init_sum)");
        sqlIns.bindValue(":id_ac", idAcc);
        sqlIns.bindValue(":id_cur", idCur);
        sqlIns.bindValue(":init_sum", startBalance[idCur]);
        if (!execQuery(sqlIns))
            return -1;
    }
    return idAcc;
}

int HwDatabase::accountId(const QString &name)
{
    QSqlQuery sqlSel(sqlDb);
    if (isICUSupported) {
        sqlSel.prepare("select id from hw_account where upper(name)=:name");
        sqlSel.bindValue(":name", name.toUpper());
    }
    else {
        sqlSel.prepare("select id from hw_account where name=:name");
        sqlSel.bindValue(":name", name);
    }
    return dictId(sqlSel);
}

int HwDatabase::addUnit(const QString &name, const QString &shortName, const QString &descr)
{
    QSqlQuery sqlIns(sqlDb);
    sqlIns.prepare(
        QString("insert into hw_unit (name, short_name, descr) values (:name, :short_name, :descr)"));
    sqlIns.bindValue(":name", name);
    sqlIns.bindValue(":short_name", shortName);
    sqlIns.bindValue(":descr", descr);
    if (!execQuery(sqlIns))
        return -1;
    return unitId(name);
}

int HwDatabase::unitId(const QString &name)
{
    QSqlQuery sqlSel(sqlDb);
    if (isICUSupported) {
        sqlSel.prepare("select id from hw_unit where upper(short_name)=:name");
        sqlSel.bindValue(":name", name.toUpper());
    }
    else {
        sqlSel.prepare("select id from hw_unit where short_name=:name");
        sqlSel.bindValue(":name", name);
    }
    int res = dictId(sqlSel);
    if (res==-1) {
        sqlSel.prepare("select id from hw_unit where name=:name");
        sqlSel.bindValue(":name", name);
        res = dictId(sqlSel);

    }
    return res;
}

int HwDatabase::currencyIdByAbbr(const QString &abbr)
{
    QSqlQuery sqlSel(sqlDb);
    sqlSel.prepare("select id from hw_currency where abbr=:abbr");
    sqlSel.bindValue(":abbr", abbr);
    return dictId(sqlSel);
}

int HwDatabase::addIncomeCategory(const QString &name, const QString &descr)
{
    QSqlQuery sqlIns(sqlDb);
    sqlIns.prepare(QString("insert into hw_in_cat (name, descr) values (:name, :descr)"));
    sqlIns.bindValue(":name", name);
    sqlIns.bindValue(":descr", descr);
    if (!execQuery(sqlIns))
        return -1;
    int idCat = incomeCategoryId(name);
    // Insert empty subcategory
    sqlIns.prepare(QString("insert into hw_in_subcat (id_icat, name, descr) values (:id_icat, :name, :descr)"));
    sqlIns.bindValue(":name", "--");
    sqlIns.bindValue(":descr", QObject::tr("No subcategory"));
    sqlIns.bindValue(":id_icat", idCat);
    return execQuery(sqlIns) ? idCat : -1;
}

int HwDatabase::incomeCategoryId(const QString &name)
{
    QSqlQuery sqlSel(sqlDb);
    if (isICUSupported) {
        sqlSel.prepare("select id from hw_in_cat where upper(name)=:name");
        sqlSel.bindValue(":name", name.toUpper());
    }
    else {
        sqlSel.prepare("select id from hw_in_cat where name=:name");
        sqlSel.bindValue(":name", name);
    }
    return dictId(sqlSel);
}

int HwDatabase::addIncomeSubCategory(int idParentCat, const QString &name, const QString &descr)
{
    QSqlQuery sqlIns(sqlDb);
    sqlIns.prepare(QString("insert into hw_in_subcat (id_icat, name, descr) values (:id_icat, :name, :descr)"));
    sqlIns.bindValue(":name", name);
    sqlIns.bindValue(":descr", descr);
    sqlIns.bindValue(":id_icat", idParentCat);
    if (!execQuery(sqlIns))
        return -1;
    else
        return incomeSubCategoryId(idParentCat, name);
}

int HwDatabase::incomeSubCategoryId(int idParentCat, const QString &name)
{
    QSqlQuery sqlSel(sqlDb);
    if (isICUSupported) {
        sqlSel.prepare("select id from hw_in_subcat where upper(name)=:name and id_icat=:id_icat");
        sqlSel.bindValue(":name", name.toUpper());
    }
    else {
        sqlSel.prepare("select id from hw_in_subcat where name=:name and id_icat=:id_icat");
        sqlSel.bindValue(":name", name);
    }
    sqlSel.bindValue(":id_icat", idParentCat);
    return dictId(sqlSel);
}

int HwDatabase::addExpenseCategory(const QString &name, const QString &descr)
{
    QSqlQuery sqlIns(sqlDb);
    sqlIns.prepare(QString("insert into hw_ex_cat (name, descr) values (:name, :descr)"));
    sqlIns.bindValue(":name", name);
    sqlIns.bindValue(":descr", descr);
    if (!execQuery(sqlIns))
        return -1;
    int idCat = expenseCategoryId(name);
    // Insert empty subcategory
    sqlIns.prepare(QString("insert into hw_ex_subcat (id_ecat, name, descr) values (:id_ecat, :name, :descr)"));
    sqlIns.bindValue(":name", "--");
    sqlIns.bindValue(":descr", QObject::tr("No subcategory"));
    sqlIns.bindValue(":id_ecat", idCat);
    return execQuery(sqlIns) ? idCat : -1;
}

int HwDatabase::expenseCategoryId(const QString &name)
{
    QSqlQuery sqlSel(sqlDb);
    if (isICUSupported) {
        sqlSel.prepare("select id from hw_ex_cat where upper(name)=:name");
        sqlSel.bindValue(":name", name.toUpper());
    }
    else {
        sqlSel.prepare("select id from hw_ex_cat where name=:name");
        sqlSel.bindValue(":name", name);
    }
    return dictId(sqlSel);
}

int HwDatabase::addExpenseSubCategory(int idParentCat, const QString &name, const QString &descr)
{
    QSqlQuery sqlIns(sqlDb);
    sqlIns.prepare(QString("insert into hw_ex_subcat (id_ecat, name, descr) values (:id_ecat, :name, :descr)"));
    sqlIns.bindValue(":name", name);
    sqlIns.bindValue(":descr", descr);
    sqlIns.bindValue(":id_ecat", idParentCat);
    if (!execQuery(sqlIns))
        return -1;
    else
        return expenseSubCategoryId(idParentCat, name);
}

int HwDatabase::expenseSubCategoryId(int idParentCat, const QString &name)
{
    QSqlQuery sqlSel(sqlDb);
    if (isICUSupported) {
        sqlSel.prepare("select id from hw_ex_subcat where upper(name)=:name and id_ecat=:id_ecat");
        sqlSel.bindValue(":name", name.toUpper());
    }
    else {
        sqlSel.prepare("select id from hw_ex_subcat where name=:name and id_ecat=:id_ecat");
        sqlSel.bindValue(":name", name);
    }
    sqlSel.bindValue(":id_ecat", idParentCat);
    return dictId(sqlSel);
}

int HwDatabase::addTransferType(const QString &name, const QString &descr)
{
    QSqlQuery sqlIns(sqlDb);
    sqlIns.prepare(QString("insert into hw_transfer_type (name, descr) values (:name, :descr)"));
    sqlIns.bindValue(":name", name);
    sqlIns.bindValue(":descr", descr);
    if (!execQuery(sqlIns))
        return -1;
    else
        return transferTypeId(name);
}

int HwDatabase::transferTypeId(const QString &name)
{
    QSqlQuery sqlSel(sqlDb);
    if (isICUSupported) {
        sqlSel.prepare("select id from hw_transfer_type where upper(name)=:name");
        sqlSel.bindValue(":name", name.toUpper());
    }
    else {
        sqlSel.prepare("select id from hw_transfer_type where name=:name");
        sqlSel.bindValue(":name", name);
    }
    return dictId(sqlSel);
}

bool HwDatabase::addIncomeOp(const QDateTime &opDT, double quantity, int amount, int idAcc, int idCur, int idSubCat, int idUnit,
     bool attention, const QString &descr,
     int idImp, const QString& uid)
{
    QSqlQuery sqlIns(sqlDb);
    sqlIns.prepare(
        "insert into hw_in_op(op_date, quantity, amount, id_ac, id_cur, id_isubcat, id_un, attention, descr, id_imp, uid_imp) " \
        "values (:op_date, :quantity, :amount, :id_ac, :id_cur, :id_isubcat, :id_un, :attention, :descr, :id_imp, :uid_imp)");
    sqlIns.bindValue(":op_date", opDT);
    // TODO Проверить ornull с нулевыми значениями!!!
    sqlIns.bindValue(":quantity", quantity);
    sqlIns.bindValue(":amount", amount);
    sqlIns.bindValue(":id_ac", idAcc);
    sqlIns.bindValue(":id_cur", idCur);
    sqlIns.bindValue(":id_isubcat", idSubCat);
    sqlIns.bindValue(":id_un", idOrNull(idUnit));
    sqlIns.bindValue(":attention", attention); // проверить отдельно
    sqlIns.bindValue(":descr", strOrNull(descr));
    sqlIns.bindValue(":id_imp", idOrNull(idImp));
    sqlIns.bindValue(":uid_imp", strOrNull(uid));
    return execQuery(sqlIns);
}

bool HwDatabase::addExpenseOp(const QDateTime &opDT, double quantity, int amount,
    int idAcc, int idCur, int idSubCat, int idUnit,
    int idReceipt, int discount, bool attention, const QString &descr,
    int idImp, const QString& uid)
{
    QSqlQuery sqlIns(sqlDb);
    sqlIns.prepare(
        "insert into hw_ex_op(op_date, quantity, amount, id_ac, id_cur, id_esubcat, id_un, id_rc, discount, attention, descr, id_imp, uid_imp) " \
        "values (:op_date, :quantity, :amount, :id_ac, :id_cur, :id_esubcat, :id_un, :id_rc, :discount, :attention, :descr, :id_imp, :uid_imp)");
    sqlIns.bindValue(":op_date", opDT);
    // TODO Проверить ornull с нулевыми значениями!!!
    sqlIns.bindValue(":quantity", quantity);
    sqlIns.bindValue(":amount", amount);
    sqlIns.bindValue(":id_ac", idAcc);
    sqlIns.bindValue(":id_cur", idCur);
    sqlIns.bindValue(":id_esubcat", idSubCat);
    sqlIns.bindValue(":id_un", idOrNull(idUnit));
    sqlIns.bindValue(":id_rc", idOrNull(idReceipt));
    sqlIns.bindValue(":discount", intOrNull(discount, discount!=0));
    sqlIns.bindValue(":attention", attention); // проверить отдельно
    sqlIns.bindValue(":descr", strOrNull(descr));
    sqlIns.bindValue(":id_imp", idOrNull(idImp));
    sqlIns.bindValue(":uid_imp", strOrNull(uid));
    return execQuery(sqlIns);
}

bool HwDatabase::addTransfer(const QDateTime &opDT, int amount, int idCur, int idAccFrom, int idAccTo,
    int idTransferType, const QString &descr, int idImp, const QString &uid)
{
    QSqlQuery sqlIns(sqlDb);
    sqlIns.prepare(
        "insert into hw_transfer(op_date, amount, id_cur, id_ac_out, id_ac_in, id_tt, descr, id_imp, uid_imp) " \
        "values (:op_date, :amount, :id_cur, :id_ac_out, :id_ac_in, :id_tt, :descr, :id_imp, :uid_imp)");
    sqlIns.bindValue(":op_date", opDT);
    sqlIns.bindValue(":amount", amount);
    sqlIns.bindValue(":id_cur", idCur);
    sqlIns.bindValue(":id_ac_out", idAccFrom);
    sqlIns.bindValue(":id_ac_in", idAccTo);
    sqlIns.bindValue(":id_tt", idTransferType);
    sqlIns.bindValue(":descr", strOrNull(descr));
    sqlIns.bindValue(":id_imp", idOrNull(idImp));
    sqlIns.bindValue(":uid_imp", strOrNull(uid));
    return execQuery(sqlIns);
}

