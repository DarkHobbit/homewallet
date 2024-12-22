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
    //

    return sqlDb.open();
}

void GenericDatabase::close()
{
    sqlDb.close();
}

bool GenericDatabase::isOpen()
{
    return sqlDb.isOpen();
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
                             const QString &fieldName, const QString& idFieldName)
{
    QSqlQuery sqlColl(sqlDb);
    sqlColl.prepare(QString("select %1, %2 from %3 order by %2;")
                        .arg(idFieldName).arg(fieldName).arg(tableName));
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
                                const QString &tableName, const QString &fieldName, const QString &idFieldName)
{
    for (const QString& parentItem : coll.keys())
        if (!collectDict(subColl[parentItem], tableName, fieldName, idFieldName))
            return false;
    return true;
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

QVariant GenericDatabase::idOrNull(int id)
{
    return (id>0) ? QString::number(id) : QVariant();
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
    return value.isValid() ? value.toString("'yyyy.MM.dd'") : QVariant();
}

