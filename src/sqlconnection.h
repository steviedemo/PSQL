#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H
#include "sql_definitions.h"
#include "query.h"
#include <QString>
#include <QSqlDatabase>
#define DEFAULT_NAME "default"
class sqlConnection
{
public:
    sqlConnection(QString connectionName=DEFAULT_NAME);
    sqlConnection(QueryPtr);
    sqlConnection(QString host, QString username, QString password, QString databaseName, QString connectionName="default");
    ~sqlConnection();
    DatabasePtr startConnection();
    DatabasePtr startConnection(bool &ok);
    void        disconnect();
    bool        setQuery(Query,     queryType);
    bool        setQuery(QueryPtr);
    QueryPtr    getQueryPointer(void);
    DatabasePtr getDatabase(void);
    bool        execute (QueryPtr );
    QueryPtr    execute (bool &ok);
    int         count   ();
    QueryPtr    newQuery(bool forwardOnly=true);
private:
    QString host, username, password, dbName, name;
    DatabasePtr db;
    Query       query;
    QueryPtr    queryPointer;
};

#endif // SQLCONNECTION_H
