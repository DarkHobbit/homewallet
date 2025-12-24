/* Home Wallet
 *
 * Module: Universal application database class
 *
 * Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <iostream>
#include <sqlite3.h>

#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QStringList>
#include <QTextStream>

#include "genericdatabase.h"
#include "globals.h"

GenericDatabase::GenericDatabase()
{
    sqlDb = QSqlDatabase::addDatabase("QSQLITE");
}

GenericDatabase::~GenericDatabase()
{
    sqlDb.close();

}

QString GenericDatabase::lastError()
{
    if (!_lastError.isEmpty()) {
        QString res = _lastError;
        _lastError.clear();
        return res;
    }
    else
        return sqlDb.lastError().text();
}

QStringList GenericDatabase::warnings()
{
    return _warnings;
}

bool GenericDatabase::exists(const QString& path)
{
    return QFile::exists(path+QDir::separator()+fileName());
}

bool GenericDatabase::open(const QString& path)
{
    if (!sqlDb.isDriverAvailable(sqlDb.driverName())) {
        _lastError = QObject::tr("Driver not available: ")+sqlDb.driverName();
        return false;
    }
    // TODO here divide C/S and local databases
    QDir d(path);
    if (!d.exists()) {
        if (!d.mkpath(path)) {
            _lastError = QObject::tr("Can't create path: ")+path;
            return false;
        }
    }
    sqlDb.setDatabaseName(path+QDir::separator()+fileName());
    bool res = sqlDb.open();
    if (res)
        res = checkForICU();
    return res;
}

void GenericDatabase::close()
{
    sqlDb.close();
}

bool GenericDatabase::isOpen()
{
    return sqlDb.isOpen();
}

QString GenericDatabase::dbInfo()
{
    return QObject::tr("Driver: %1; database: %2; ICU support: %3")
        .arg(sqlDb.driverName())
        .arg(sqlDb.databaseName())
        .arg(isICUSupported ? QObject::tr("yes") : QObject::tr("no"));
}

QSqlDatabase &GenericDatabase::sqlDbRef()
{
    return sqlDb;
}

int GenericDatabase::queryRecCount(QSqlQuery &query)
{
    if (query.driver()->hasFeature(QSqlDriver::QuerySize))
        return query.size();
    else {
        // Must be open; after this, caller must seek to appr.pos.
        query.last();
        int res = query.at();
        // Filter BeforeFirstRecord and AfterLastRecord
        return res<0 ? 0 : res+1;
    }
}

bool GenericDatabase::collectDict(GenericDatabase::DictColl& coll, const QString &tableName,
    const QString &fieldName, const QString& idFieldName, const QString& where)
{
    QSqlQuery sqlColl(sqlDb);
    if (!sqlColl.prepare(QString("select %1, %2 from %3 %4 order by %2;")
          .arg(idFieldName).arg(fieldName).arg(tableName).arg(where))) {
        _lastError = sqlColl.lastError().text();
        return false;
    }
    if (!sqlColl.exec()) {
        _lastError = sqlColl.lastError().text();
        return false;
    }
    sqlColl.first();
    while (sqlColl.isValid()) {
        coll[sqlColl.value(1).toString()] = sqlColl.value(0).toInt();
        sqlColl.next();
    }
    return true;
}

bool GenericDatabase::collectSubDict(const DictColl &coll, SubDictColl &subColl,
    const QString &tableName, const QString &fieldName, const QString &idFieldName, const QString& upIdFieldName)
{
    for (const QString& parentItem : coll.keys()) {
        int idUp = coll[parentItem];
        QString where = QString("where %1=%2").arg(upIdFieldName).arg(idUp);
        if (!collectDict(subColl[parentItem], tableName, fieldName, idFieldName, where))
            return false;
    }
    return true;
}

bool GenericDatabase::collectRevDict(RevDictColl &coll, const QString &tableName, const QString &fieldName, const QString &idFieldName, const QString &where)
{
    QSqlQuery sqlColl(sqlDb);
    if (!sqlColl.prepare(QString("select %1, %2 from %3 %4 order by %2;")
          .arg(idFieldName).arg(fieldName).arg(tableName).arg(where))) {
        _lastError = sqlColl.lastError().text();
        return false;
    }
    if (!sqlColl.exec()) {
        _lastError = sqlColl.lastError().text();
        return false;
    }
    sqlColl.first();
    while (sqlColl.isValid()) {
        coll[sqlColl.value(0).toInt()] = sqlColl.value(1).toString();
        sqlColl.next();
    }
    return true;
}

bool GenericDatabase::isTableEmpty(const QString &tableName, const QString &fieldName, const QString &idFieldName)
{
    DictColl coll;
    if (!collectDict(coll, tableName, fieldName, idFieldName))
        std::cerr << "Error check table " << tableName.toUtf8().data()
                  << lastError().toUtf8().data() << std::endl;
    return coll.isEmpty();
}

bool GenericDatabase::loadSqlFile(const QString& filePath)
{
    QStringList queries;
    // Read SQL file
    QFile sqlF(filePath);
    if (!sqlF.open(QIODevice::ReadOnly)) {
        _lastError = S_READ_ERR.arg(filePath);
        return false;
    }
    QTextStream sqlS(&sqlF);
    sqlS.setCodec("UTF-8"); // Some dictionaries may contain non-latin chars
    bool newQ = true;
    while (!sqlS.atEnd()) {
        QString l = sqlS.readLine().trimmed();
        if (l.isEmpty())       // drop empty lines
            continue;
        if (l.startsWith("--")) // drop full-line SQL comments
            continue;
        if (l.contains("--")) // skip SQL comments on line end
            l = l.left(l.indexOf("--")).trimmed();
        if (newQ || queries.isEmpty())
            queries << l;
        else
            queries.last() += " " + l;
        newQ = l.endsWith(";");
    }
    sqlF.close();
    if (queries.isEmpty()) {
        _lastError = S_EMPTY_INIT_FILE.arg(filePath);
        return false;
    }
    // Execute DDL statements
    QSqlQuery ddlQuery(sqlDb);
    for(int i=0; i<queries.count(); i++)
        try {
            if (!ddlQuery.exec(queries[i])) {
                _lastError = ddlQuery.lastError().text() + "\n\n" + queries[i];
                close();
                return false;
            }
        }
        catch (...) {
            _lastError = ddlQuery.lastError().text() + "\n\n" + queries[i];
            close();
            return false;
        }
    return true;
}

bool GenericDatabase::checkTablePresence(const QString &tableName)
{
    try {
        QSqlQuery q(QString("select * from %1;").arg(tableName), sqlDb);
        return (q.lastError().type()==QSqlError::NoError);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool GenericDatabase::prepQuery(QSqlQuery &q, const QString &sql)
{
    bool res = q.prepare(sql);
    if (!res) {
        _lastError = q.lastError().text() + ":\n" + q.lastQuery();
    }
    return res;
}

bool GenericDatabase::execQuery(QSqlQuery &q)
{
    bool res = q.exec();
    if (!res) {
        _lastError = q.lastError().text() + ":\n" + q.lastQuery();
        if (!q.boundValues().isEmpty()) {
            QMapIterator<QString, QVariant> i(q.boundValues());
            while (i.hasNext()) {
               i.next();
               _lastError += QString("\n    %1=%2").arg(i.key()).arg(i.value().toString());
            }
        }
    }
    return res;
}

int GenericDatabase::dictId(QSqlQuery &q)
{
    if (!q.exec()) {
        _lastError = q.lastError().text();
        return -1;
    }
    if (queryRecCount(q)==0)
        return -1;
    q.first();
    return q.value(0).toInt();
}

QString GenericDatabase::dictName(QSqlQuery &q)
{
    if (!q.exec()) {
        _lastError = q.lastError().text();
        return "";
    }
    if (queryRecCount(q)==0)
        return "";
    q.first();
    return q.value(0).toString();
}

QVariant GenericDatabase::idOrNull(int id)
{
    return (id>0) ? QString::number(id) : QVariant();
}

QVariant GenericDatabase::idOrNull(int id, bool notNull)
{
    return (notNull) ? QString::number(id) : QVariant();
}

QVariant GenericDatabase::intOrNull(int value, bool notNull)
{
    return (notNull) ? QString::number(value) : QVariant();
}

QVariant GenericDatabase::strOrNull(const QString &s)
{
    return !s.isEmpty() ? s : QVariant();
}

QVariant GenericDatabase::dateOrNull(const QDateTime &value)
{
    return value.isValid() ? value.toString("yyyy-MM-dd") : QVariant();
}

bool GenericDatabase::checkForICU()
{
    if (sqlDb.driverName()=="QSQLITE") {
        // Check for internal non-latin support testing on cyrillic "Sputnik" word
        if (checkForICUonly()) {
            isICUSupported = true;
            return true;
        }
        // Try load ICU extension
        sqlite3 *handle = *static_cast<sqlite3 **>(sqlDb.driver()->handle().data());
#ifdef WITH_SQLITE_EXTENSIONS
        if (handle) {
            int res = sqlite3_enable_load_extension(handle, 1);
            if (res!=SQLITE_OK) {
                _lastError = QObject::tr("Can't enable extension load, code: %1").arg(res);
                return false;
            }
            char* errMsg;
            res = sqlite3_load_extension(handle, "libsqliteicu", "sqlite3_icu_init", &errMsg);
            if (res!=SQLITE_OK) {
                _warnings << QObject::tr("Can't load ICU extension, code %1:\n%2\nRebuild SQLite with ICU support or provide ICU library as extension")
                .arg(res).arg(QString::fromUtf8(errMsg));
                std::cerr << _lastError.toUtf8().data() << std::endl;
                sqlite3_free(errMsg);
                isICUSupported = false;
                return true;
            }
            if (!checkForICUonly()) {
                _warnings << QObject::tr("ICU extension load failed");
                isICUSupported = false;
                return true;
            }
        }
#else
        return false;
#endif
    }    
    isICUSupported = true;
    return true;
}

bool GenericDatabase::checkForICUonly()
{
    QSqlQuery sqlCheck(sqlDb);
    sqlCheck.prepare(QString::fromUtf8("select upper('Спутник')='СПУТНИК'"));
    // sqlCheck.prepare(QString::fromUtf8("select upper('Table')='TABLE'"));
    if (!sqlCheck.exec())
        _lastError = QObject::tr("Can't check ICU presence");
    sqlCheck.first();
    return sqlCheck.value(0).toBool();
}

