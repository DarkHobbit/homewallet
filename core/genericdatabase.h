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

#ifndef GENERICDATABASE_H
#define GENERICDATABASE_H

#include <sqlite3.h>
#include <QDateTime>
#include <QMap>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QString>
#include <QVariant>

#define S_CANT_CREAT_DB QObject::tr("Cannot create database in %1:\n%2")
#define S_CANT_OPEN_DB QObject::tr("Cannot open database in %1:\n%2")
#define S_EMPTY_INIT_FILE QObject::tr("Database create file is empty, contact author:\n%1")

class GenericDatabase
{
public:
    typedef QMap<QString, int> DictColl; // key - dictionary item name, value -  id
    typedef QMap<QString, DictColl> SubDictColl;
    typedef QMap<QString, DictColl> TableRefColl;
    typedef QMap<int, QString> RevDictColl; // key - id, value - dictionary item name
    GenericDatabase();
    ~GenericDatabase();
    virtual QString fileName()=0;
    QString lastError();
    QStringList warnings();
    bool exists(const QString& path);
    bool open(const QString& path);
    void close();
    bool isOpen();
    QString dbInfo();
    QSqlDatabase& sqlDbRef();
    // Tools
    int queryRecCount(QSqlQuery& query);
    bool prepQuery(QSqlQuery& q, const QString& sql);
    bool execQuery(QSqlQuery& q);
    bool collectDict(DictColl& coll, const QString& tableName,
        const QString& fieldName = "name", const QString& idFieldName = "id",
        const QString& where = "");
    bool collectSubDict(const DictColl& coll, SubDictColl& subColl, const QString& tableName,
        const QString& fieldName = "name", const QString& idFieldName = "id",
        const QString& upIdFieldName = "");
    bool collectRevDict(RevDictColl& coll, const QString& tableName,
        const QString& fieldName = "name", const QString& idFieldName = "id",
        const QString& where = "");
    bool isTableEmpty(const QString& tableName,
        const QString& fieldName = "name", const QString& idFieldName = "id");
    int getLastSequenceValue(const QString& tableName);

protected:
    QSqlDatabase sqlDb;
    QString _lastError;
    QStringList _warnings;
    bool isICUSupported;
    sqlite3 *handle;
    bool loadSqlFile(const QString& filePath);
    bool checkTablePresence(const QString& tableName);
    //bool checkFieldPresence(const QString& tableName);
    int dictId(QSqlQuery& q);
    QString dictName(QSqlQuery& q);
    QVariant idOrNull(int id);
    QVariant idOrNull(int id, bool notNull);
    QVariant intOrNull(int value, bool notNull);
    QVariant strOrNull(const QString& s);
    QVariant dateOrNull(const QDateTime& value);

private:
    bool checkForICU();
    bool checkForICUonly();
};

#endif // GENERICDATABASE_H
