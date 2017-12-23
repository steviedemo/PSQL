#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H
#include <QString>
#include <QSqlDatabase>
class sqlConnection
{
public:
    sqlConnection(QString host, QString username, QString password, QString databaseName, QString connectionName="default");
    ~sqlConnection();
    QSqlQuery *newQuery(bool forwardOnly=true);
    QSqlDatabase *connect();
    void disconnect();
    bool execute(QSqlQuery *);
private:
    QString host, username, password, dbName, name;
    QSqlDatabase *db;
};

#endif // SQLCONNECTION_H
