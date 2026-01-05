#include <iostream>
#include <sqlite3.h>

#include <QCoreApplication>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

QSqlDatabase db;

bool dbExec(const QString& sql)
{
    QSqlQuery q(db);
    std::cout << "Try: " << sql.toUtf8().data() << std::endl;
    if (!q.prepare(sql)) {
        std::cerr << "ERROR1: " << q.lastError().type() << " - " << q.lastError().text().toUtf8().data() << std::endl;
        return false;
    }
    if (!q.exec()) {
        std::cerr << "ERROR2: " << q.lastError().text().toUtf8().data() << std::endl;
        return false;
    }
    else
        return true;
}

bool dbExecAndType(const QString& sql)
{
    QSqlQuery qq;
    if (!qq.exec(sql)) {
        std::cerr << "ERROR: " << qq.lastError().text().toUtf8().data() << std::endl;
        return false;
    }
    qq.first();
    while(qq.isValid()) {
        std::cout << qq.value(0).toString().toUtf8().data() << "||"
                  << qq.value(1).toString().toUtf8().data() << std::endl;
        qq.next();
    }
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    db = QSqlDatabase::addDatabase("QSQLITE");
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        std::cerr << "Driver not available!" << std::endl;
        return 1;
    }
    db.setDatabaseName("testdb.sqlite");
    if (!db.open()) {
        std::cerr << "Can't open database" << std::endl;
        return 1;
    }
    sqlite3 *handle = *static_cast<sqlite3 **>(db.driver()->handle().data());
    if (!handle) {
        std::cerr << "Can't get SQLite handle" << std::endl;
        return 1;
    }
    if (!dbExec("create table t42 (id integer primary key autoincrement, name text)"))
        return 1;
    if (!dbExec("create table t43 (idd integer primary key autoincrement, abbr text)"))
        return 1;
    if (!dbExec("insert into t42(name) values ('one')"))
        return 1;
    if (!dbExec("insert into t42(name) values ('two')"))
        return 1;
    std::cout << "h1 " << sqlite3_last_insert_rowid(handle) << std::endl;
    if (!dbExec("insert into t43(abbr) values ('oneone')"))
        return 1;
    std::cout << "h2 " << sqlite3_last_insert_rowid(handle) << std::endl;
    if (!dbExec("insert into t42(name) values ('three')"))
        return 1;
    std::cout << "h3 " << sqlite3_last_insert_rowid(handle) << std::endl;
    if (!dbExecAndType("select * from t42"))
        return 1;
    if (!dbExecAndType("select * from t43"))
        return 1;
    return 0;
}
