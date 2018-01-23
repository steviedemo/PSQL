#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H
#include "sql_definitions.h"
#include "Query.h"
#include <QString>
#include <QSqlDatabase>
#include <pqxx/pqxx>
#include <string>
#define DEFAULT_NAME "default"
class sqlConnection
{
public:
    sqlConnection(std::string sqlStatement="");
    sqlConnection(const QString sqlStatement);
 //   sqlConnection(QueryPtr);
    sqlConnection(Query, queryType);

    ~sqlConnection();
    void        disconnect();
    void        clear();
    void        setQuery(Query,     queryType);
    void        setQuery(const std::string text);
    QString     getQuery(void);
    bool        execute ();
    bool        execute (std::string sqlStatement);
    bool        execute (Query q, queryType t);
    pqxx::result getResult();
    bool        getRawConnection(pqxx::connection *dbConnection);
    int         countResults();
    bool        foundMatch();
    bool        isConnected();
private:
    void setup();
    bool verify(QueryPtr);
    bool verify();
    QString host, username, password, dbName, name;
    std::string query_string;
    std::string credentialString;
    Query       query;

    pqxx::connection *databaseConnection;
    pqxx::result lastResult;

};

#endif // SQLCONNECTION_H
