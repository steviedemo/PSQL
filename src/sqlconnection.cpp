#include "sqlconnection.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>
sqlConnection::sqlConnection(QString host, QString username, QString password, QString database_name, QString connectionName):
    host(host), username(username), password(password), dbName(database_name), name(connectionName){
    this->db = new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL", connectionName));
}
sqlConnection::~sqlConnection(){
    if (db->isOpen()){
        db->close();
    }
    delete db;
}
void sqlConnection::disconnect(){
    db->close();
}

QSqlDatabase *sqlConnection::connect(){
    db->setConnectOptions();
    db->setHostName(host);
    db->setDatabaseName(dbName);
    db->setUserName(username);
    db->setPassword(password);
    if (!db->isValid()){
        qWarning("Error: Database is invalid!");
        return NULL;
    } else if (db->open){
        return db;
    }
    qWarning("Error Connecting to database");
    return NULL;
}

QSqlQuery *sqlConnection::newQuery(bool forwardOnly){
    QSqlQuery *query = new QSqlQuery(*db);
    query->setForwardOnly(forwardOnly);
    return query;
}

bool sqlConnection::execute(QSqlQuery *query){
    bool success = false;
    db->transaction();
    bool result = query->exec();
    if (query->lastError().type() != QSqlError::NoError || !result){
        qWarning("Error Executing query: %s", qPrintable(query->lastError().text()));
        db->rollback();
    } else {
        db->commit();
        success = true;
    }
    return success;
}



