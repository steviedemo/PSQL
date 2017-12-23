#include "sqlconnection.h"
#include "query.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QTextStream>
sqlConnection::sqlConnection(QString connectionName):
    host(HOST), username(USERNAME), password(PASSWORD), dbName(DB_NAME), name(connectionName){
    this->db = DatabasePtr(new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL", name)));
}
sqlConnection::sqlConnection(QueryPtr q):
    host(HOST), username(USERNAME), password(PASSWORD), dbName(DB_NAME), name(connectionName){
    this->db = DatabasePtr(new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL", name)));
    this->setQuery(q);
}

sqlConnection::sqlConnection(QString host, QString username, QString password, QString connectionName):
    host(host), username(username), password(password), dbName(DB_NAME), name(connectionName){
    this->db = QSharedPointer<QSqlDatabase>(new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL", connectionName)));
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

DatabasePtr sqlConnection::startConnection(bool &ok){
    db->setConnectOptions();
    db->setHostName(host);
    db->setDatabaseName(dbName);
    db->setUserName(username);
    db->setPassword(password);
    if (!db->isValid()){
        qWarning("Error: Database is invalid!");
        ok = false;
        return NULL;
    } else if (db->open){
        ok = true;
        return db;
    }
    ok = false;
    qWarning("Error Connecting to database");
    return NULL;
}

DatabasePtr sqlConnection::startConnection(){
    bool ok = false;
    return startConnection(ok);
}
DatabasePtr sqlConnection::getDatabase()    {   return db;  }
QueryPtr sqlConnection::newQuery(bool forwardOnly){
    QueryPtr query = QSharedPointer<QSqlQuery>(new QSqlQuery(*(db.data())));
    query->setForwardOnly(forwardOnly);
    return query;
}

bool sqlConnection::setQuery(Query q, queryType type){
    bool success = false;
    this->query = query;
    this->queryPointer = q.toSqlQuery(type, db, success);
    return success;
}

bool sqlConnection::setQuery(QueryPtr q)    {   this->queryPointer = q;     }
QueryPtr sqlConnection::getQueryPointer()   {   return this->queryPointer;  }


/** \brief Execute the currently stored query. */
QueryPtr sqlConnection::execute(bool &ok){
    ok = execute(this->queryPointer);
    return queryPointer;
}

/** \brief Execute the Query passed on the current Database. */
bool sqlConnection::execute(QueryPtr query){
    bool success = false;
    if (verify(query)){
        db->transaction();
        bool result = query->exec();
        if (query->lastError().type() != QSqlError::NoError || !result){
            qWarning("Error Executing query: %s", qPrintable(query->lastError().text()));
            db->rollback();
        } else {
            db->commit();
            success = true;
        }
    }
    return success;
}

/** \brief Check the query and the database to ensure there will not be any fundamental issues
 *          before running the Query.
 */
bool sqlConnection::verify(QueryPtr query){
    bool queryOkay = true;
    QString s("");
    QTextStream out(&s);
    if (!query->isValid())
        out << "QSqlQuery object is invalid!" << endl;
    if (!db->isValid())
        out << "Invalid Database '" << db->databaseName() << "'" << endl;
    if (!db->isOpen())
        out << "Database " << db->databaseName() << " is not Open!" << endl;
    if (!s.isEmpty()){
        qWarning("Unable to run Query, Errors were found:\n%s", qPrintable(s));
        queryOkay = false;
    }
    return queryOkay;
}


/** \brief Count the number of rows that the provided SELECT query returns */
int sqlConnection::count(){

    bool rows = -1;
    bool ok = queryPointer->exec();
    if (queryPointer->lastError().type() != QSqlError::NoError || !ok){
        qWarning("Error Counting matching results: %s", qPrintable(queryPointer->lastError().text()));
    } else {
        rows = queryPointer->size();
    }
    return rows;
}
