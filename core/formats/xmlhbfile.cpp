/* Home wallet
 *
 * Module: Home Bookkeeping (keepsoft.ru) XML file export/import
 *
 * Copyright 2023 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <iostream>

#include "commonexpimpdef.h"
#include "globals.h"
#include "xmlhbfile.h"

XmlHbFile::XmlHbFile()
    : XmlFile(),
    _fileSubType(Unknown), _categorySamples(""),
    hbMoneySum("^([0-9\\s]*[.,]?[0-9\\s]+)(\\S+)$") // 11 900,00Є
{
}

QIODevice::OpenMode XmlHbFile::supportedModes()
{
    return QIODevice::ReadOnly;
}

QStringList XmlHbFile::supportedFilters()
{
    return QStringList() << QObject::tr("Home Bookkeeping XML (*.xml *.XML)");
}

FileFormat::SubTypeFlags XmlHbFile::supportedExportSubTypes()
{
    return Unknown;
}

void XmlHbFile::clear()
{
    QDomDocument::clear();
    _fileSubType = Unknown;
    _categorySamples = "";
}

bool XmlHbFile::detect(const QString &path)
{
    // Read XML
    if (!readFromFile(path))
        return false;
    // Metadata
    QDomNodeList fields = elementsByTagName("FIELD");
    if (fields.isEmpty()) {
        _fatalError = QObject::tr("Metadata not found. Probably it isn't Home Bookkeeping file");
        closeFile();
    }
    QStringList fieldNames;
    for (int i=0; i<fields.count(); i++)
        fieldNames << fields.at(i).toElement().attribute("attrname");
    if (fieldNames.isEmpty()) {
        _fatalError = QObject::tr("Metadata is empty. Probably it isn't Home Bookkeeping file");
        closeFile();
    }
    // 1. Accounts
    if (fieldNames.contains("Expense1")) {
        if (fieldNames.contains("StartBalans1") && fieldNames.contains("Note"))
            _fileSubType = AccountsInBrief;
        else
            _fileSubType = AccountsInDetail; // not
    }
    // 2. Currency rate
    else if ((fieldNames.contains("Rate2") || fieldNames.contains("Rate1"))
    && !fieldNames.contains("Money1"))
        _fileSubType = CurrencyRate;
    // 3. Expenses or Income, Debtors or Creditors, Plan Incomes or Expenses
    else if (fieldNames.contains("Money1")) {
        if (fieldNames.contains("Rate2") || fieldNames.contains("Rate1")) {
            _fileSubType = IncomesOrExpenses; // ambiguous, need to specify
            collectCatSamples(3, "Category");
        }
        else if (fieldNames.contains("FIO")) {
            _fileSubType = DebtorsOrCreditors; // ambiguous, need to specify
            collectCatSamples(5, "FIO");
        }
        else {
            _fileSubType = IncomeOrExpensePlan; // ambiguous, need to specify
            collectCatSamples(3, "Category");
        }
    }
    // 4. Transfer
    else if (fieldNames.contains("AccountIn") && fieldNames.contains("MoneyStr"))
        _fileSubType = Transfer;
    // 5. Currency conversion (must be a last check)
    else {
        for(const QString& fldName: fieldNames) {
            if (fldName.startsWith("MoneyIn") || fldName.startsWith("MoneyOut")) {
                _fileSubType = CurrencyConversion;
                break;
            }
        }
    }
    return _fileSubType != Unknown;
}

QString XmlHbFile::formatAbbr()
{
    return "XMHBK";
}

bool XmlHbFile::isDialogRequired()
{
    return false;
}

bool XmlHbFile::importRecords(const QString &path, HwDatabase &db)
{
    QDomDocument::clear();
    if (!hbMoneySum.isValid()) {
        _fatalError = hbMoneySum.errorString();
        return false;
    }
    // Was detected?
    if (_fileSubType==Unknown) {
        _fatalError = QObject::tr("Unknown Home Bookkeeping file subtype.\nContact author.");
        return false;
    }
    if (isAmbiguous()) {
        // Internal error, will not appear if caller class is correct
        _fatalError = QObject::tr("Ambiguous Home Bookkeeping file subtype wasn't resolved.\nContact author.");
        return false;
    }
    // Read XML
    if (!readFromFile(path))
        return false;
    // Prepare dictionaries
    HwDatabase::DictColl accs; // Account list will need always :)
    DB_CHK(db.collectDict(accs, "hw_account"));
    HwDatabase::DictColl currs, units; // Currencies, units - almost always
    if (_fileSubType!=AccountsInBrief) {
        DB_CHK(db.collectDict(currs, "hw_currency", "abbr"))
        DB_CHK(db.collectDict(units, "hw_unit", "short_name"))
    }
    HwDatabase::DictColl cats;
    HwDatabase::SubDictColl subcats;
    int idTransferTypeOther = -1;
    if (_fileSubType==Incomes || _fileSubType==IncomePlan) {
        DB_CHK(db.collectDict(cats, "hw_in_cat"))
        DB_CHK(db.collectSubDict(cats, subcats, "hw_in_subcat", "name", "id", "id_icat"))
    }
    else if (_fileSubType==Expenses || _fileSubType==ExpensePlan) {
        DB_CHK(db.collectDict(cats, "hw_ex_cat"))
        DB_CHK(db.collectSubDict(cats, subcats, "hw_ex_subcat", "name", "id", "id_ecat"))
    }
    else if (_fileSubType==Transfer) {
        DB_CHK(db.collectDict(cats, "hw_transfer_type"))
        if (cats.keys().contains(S_CAT_OTHER))
            idTransferTypeOther = cats[S_CAT_OTHER];
        else {
            idTransferTypeOther = db.addTransferType(S_CAT_OTHER, "");
            cats[S_CAT_OTHER] =  idTransferTypeOther;
        }
    }

    /*
    std::cout << "ac_count " << accs.keys().count() <<std::endl; //===>
    std::cout << "cur_count " << currs.keys().count() <<std::endl; //===>
    std::cout << "cat_count " << cats.keys().count() <<std::endl; //===>
    std::cout << "subcat_count " << subcats.keys().count() <<std::endl; //===>
    for (const QString& catname : cats.keys())
        std::cout << subcats[catname].count() << " " <<std::endl; //===>
    std::cout << std::endl; //===>
*/
    // Import records
    QDomNodeList records = elementsByTagName("RECORD");
    _totalRecordsCount = records.count();
    _importedRecordsCount = 0;
    int skippedRecordCount = 0;
    for (int i=0; i<records.count() /*&& i<20*/; i++) {
        if (i%500==0)
            std::cout << "Rec " << i << " from " << records.count() << std::endl;
        QDomElement elRow = records.at(i).firstChildElement("ROW");
        QString sLine = QString::number(elRow.lineNumber());
        switch (_fileSubType) {
        case AccountsInBrief: {
            QString accName = elRow.attribute("Account");
            if (accs.keys().contains(accName))
                skippedRecordCount++;
            else {
                HwDatabase::MultiCurrByChar money;
                if (!importNotNullMoney(money, "StartBalans", elRow, false))
                    return false;
                HwDatabase::MultiCurrById moneyIds;
                for (const QString& moneyChar: money.keys()) {
                    int idCur = importCurrencyByChar(moneyChar, db);
                    if (idCur==-1)
                        return false;
                    moneyIds[idCur] = money[moneyChar];
                }
                if (db.addAccount(accName, elRow.attribute("Note"), QDateTime(), moneyIds)==-1) {
                    _fatalError = db.lastError();
                    return false;
                }
            }
            break;
        }
        case CurrencyRate:

            // TODO
            break;
        case Incomes:
        case Expenses: {
            bool isExp = _fileSubType==Expenses;
            QDateTime dt;
            if (!readDateVal(elRow, "MyDate", dt, "yyyyMMdd", S_ERR_DATE_IMP))
                return false;
            double quantity;
            if (!readDoubleVal(elRow, "Quantity", quantity, S_ERR_QTY_IMP))
                return false;
            // Account
            int idAcc = importAccount("Account", elRow, accs, db);
            // Money
            HwDatabase::MultiCurrByChar money;
            if (!importNotNullMoney(money, "Money", elRow, true))
                return false;
            if (money.count()==0) {
                _fatalError = QObject::tr(
                    "Income or expense amount can't be equal to 0. Date: %1")
                    .arg(dt.toString());
                return false;
            }
            else if (money.count()>1) {
                _fatalError = QObject::tr(
                    "This version of HomeWallet not support multi-currency incomes and expenses, item from %1, currencies: %2")
                    .arg(dt.toString()).arg(money.count());
                return false;
            }
            QString moneyChar = money.keys().first();
            int idCur = importCurrencyByChar(moneyChar, db);
            if (idCur==-1)
                return false;
            // Categories an subcategories
            int idCat, idSubCat;
            QString catName = elRow.attribute("Category");
            QString subCatName = elRow.attribute("Subcategory");
            if (subCatName.isEmpty())
                subCatName = "--";
            if (cats.keys().contains(catName))
                idCat = cats[catName];
            else {
                if (isExp) {
                    idCat = db.addExpenseCategory(catName, "");
                    subcats[catName]["--"] = db.expenseSubCategoryId(idCat, "--");
                }
                else {
                    idCat =  db.addIncomeCategory(catName, "");
                    subcats[catName]["--"] = db.incomeSubCategoryId(idCat, "--");
                }
                cats[catName] =  idCat;
            }
            DB_CHK(idCat!=-1);
            if (subcats[catName].keys().contains(subCatName))
                idSubCat = subcats[catName][subCatName];
            else {
                if (isExp)
                    idSubCat = db.addExpenseSubCategory(idCat, subCatName, "");
                else
                    idSubCat =  db.addIncomeSubCategory(idCat, subCatName, "");
                subcats[catName][subCatName] = idSubCat;
            }
            DB_CHK(idSubCat!=-1);
            // Units
            QString unitName = elRow.attribute("Unit");
            int idUnit;
            if (units.keys().contains(unitName))
                idUnit = units[unitName];
            else {
                idUnit = db.addUnit(unitName, unitName, "");
                units[unitName] = idUnit;
            }
// TODO how check ex/in for skip as dup?
            // Insert!
            bool ok;
            if (isExp)
                ok = db.addExpenseOp(dt, quantity, money.values().first(),
                    idAcc, idCur, idSubCat, idUnit, -1, 0, false, elRow.attribute("Note"),
                    _idImp, sLine);
            else
                ok = db.addIncomeOp(dt, quantity, money.values().first(), idAcc, idCur, idSubCat, idUnit, false, elRow.attribute("Note"),
                    _idImp, sLine);
            DB_CHK(ok);
            break;
        }
        case Transfer: {
            QDateTime dt;
            if (!readDateVal(elRow, "MyDate", dt, "yyyyMMdd", S_ERR_DATE_IMP))
                return false;
            // Accounts
            int idAccFrom = importAccount("AccountOut", elRow, accs, db);
            DB_CHK(idAccFrom!=-1);
            int idAccTo = importAccount("AccountIn", elRow, accs, db);
            DB_CHK(idAccTo!=-1);
            // Money
            QString moneyChar;
            QString val = elRow.attribute("MoneyStr", moneyChar);
            if (val.isEmpty()) {
                _fatalError = S_ERR_ATTR_NOT_FOUND.arg("MoneyStr").arg(elRow.lineNumber());
                return false;
            }
            int sum = importOneMoneyAttr(val, moneyChar);
            if (!_fatalError.isEmpty())
                return false;
            int idCur = importCurrencyByChar(moneyChar, db);
            if (idCur==-1)
                return false;
            // Insert!
            bool ok = db.addTransfer(dt, sum, idCur, idAccFrom, idAccTo,
                idTransferTypeOther, elRow.attribute("Note"), _idImp, sLine);
            DB_CHK(ok);
            break;
        }
        case CurrencyConversion: {
            QDateTime dt;
            if (!readDateVal(elRow, "MyDate", dt, "yyyyMMdd", S_ERR_DATE_IMP))
                return false;
            // Account
            int idAcc = importAccount("Account", elRow, accs, db);
            // Money
            HwDatabase::MultiCurrByChar moneyFrom, moneyTo;
            if (!importNotNullMoney(moneyFrom, "MoneyOut", elRow, true))
                return false;
            if (!importNotNullMoney(moneyTo, "MoneyIn", elRow, true))
                return false;
            if (moneyFrom.count()!=1 || moneyTo.count()!=1) {
                _fatalError = QObject::tr(
                    "For currency conversion strongly 1 MoneyOut* and 1 MoneyIn* attributes needed at line %1")
                    .arg(elRow.lineNumber());
                return false;
            }
            int idCurFrom = importCurrencyByChar(moneyFrom.keys().first(), db);
            if (idCurFrom==-1)
                return false;
            int idCurTo = importCurrencyByChar(moneyTo.keys().first(), db);
            if (idCurTo==-1)
                return false;
            bool ok = db.addCurrencyConv(dt, idAcc,
                idCurFrom, moneyFrom.values().first(),
                idCurTo, moneyTo.values().first(),
                elRow.attribute("Note"), _idImp, sLine);
            DB_CHK(ok);
            break;
        }
        case Debtors:

            // TODO
            break;
        case Creditors:

            // TODO
            break;
        case IncomePlan:

            // TODO
            break;
        case ExpensePlan:
        default:
            // TODO
            break;
        }
        _importedRecordsCount++;
    }
    std::cout << "Total records " << records.count() << ", skipped " << skippedRecordCount << ", imported " << _importedRecordsCount << std::endl;
    if (skippedRecordCount>0)
        _errors << QObject::tr("Skipped %1 records").arg(skippedRecordCount);
    return true;
}

bool XmlHbFile::exportRecords(const QString &path, HwDatabase &db, SubTypeFlags subTypes)
{
    // TODO
    return false; //===>
}

XmlHbFile::SubType XmlHbFile::fileSubType()
{
    return _fileSubType;
}

void XmlHbFile::setFileSubType(SubType subType)
{
    _fileSubType = subType;
}

QString XmlHbFile::categorySamples()
{
    return _categorySamples;
}

void XmlHbFile::collectCatSamples(short maxRecordCount, const QString &fieldName)
{
    QDomNodeList records = elementsByTagName("RECORD");
    QStringList cs;
    for (int i=0; i<records.count(); i++) {
        QDomElement elRow = records.at(i).firstChildElement("ROW");
        QString cat = elRow.attribute(fieldName);
        if (!cs.contains(cat))
            cs << cat;
        if (cs.count()>=maxRecordCount)
            break;
    }
    _categorySamples = cs.join(", ");
}

int XmlHbFile::importAccount(const QString &attr, const QDomElement &elRow, HwDatabase::DictColl& accs, HwDatabase& db)
{
    QString accName = elRow.attribute(attr);
    int idAcc;
    if (accs.keys().contains(accName)) {
        idAcc = accs[accName];
    }
    else {
        db.addAccount(accName, "Auto-inserted");
        idAcc = db.accountId(accName);
        accs[accName] = idAcc;
        _errors << QObject::tr("Account %1 not found, inserted automatically").arg(accName);
    }
    return idAcc;
}

bool XmlHbFile::importNotNullMoney(HwDatabase::MultiCurrByChar& values, const QString &attrPrefix, const QDomElement &elRow, bool skipNulls)
{
    values.clear();
    for (int i=1; i<=elRow.attributes().count(); i++) {
        QString val = elRow.attribute(attrPrefix+QString::number(i));
        if (val.isEmpty())
            continue;
        // Extract digits and moneychars
        QString sCur;
        int sum = importOneMoneyAttr(val, sCur);
        if (!_fatalError.isEmpty())
            return false;
        if ((!sum) && skipNulls)
            continue; // normal case, 2 of 3
        values[sCur] = sum;
    }
    if (values.isEmpty()) {
        _fatalError = QObject::tr("Money attributes not found: line %1").arg(elRow.lineNumber());
        return false;
    }
    return true;
}

int XmlHbFile::importOneMoneyAttr(const QString &val, QString& sCur)
{
    if (!hbMoneySum.exactMatch(val)) {
        _fatalError = QObject::tr("Money sum doesn't match: %1").arg(val);
        return 0;
    }
    QString sSum = prepareDoubleImport(hbMoneySum.cap(1));
    sCur = hbMoneySum.cap(2);
    bool ok;
    int sum = sSum.toFloat(&ok)*100;
    if (!ok) {
        _fatalError = S_ERR_AMO_IMP.arg(sSum);
        return 0;
    }
    return sum;
}

int XmlHbFile::importCurrencyByChar(const QString &moneyChar, HwDatabase& db)
{
    QString _moneyChar = moneyChar;
    if (_moneyChar==QString::fromUtf8("р"))
        _moneyChar = QString::fromUtf8("₽"); // exception for old RUR symbol, appears in HB files
    int idCur = db.currencyIdByAbbr(_moneyChar);
    if (idCur==-1) {
        _fatalError = S_ERR_CUR_NOT_FOUND.arg(_moneyChar);
        return -1;
    }
    return idCur;
}
