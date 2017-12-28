#include "sqlconnection.h"
#include "query.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QTextStream>
#include <pqxx/pqxx>
#include <pqxx/connection.hxx>
#include <pqxx/result.hxx>
#include <pqxx/tablereader.hxx>
using namespace pqxx;
sqlConnection::sqlConnection(std::string sqlStatement):
    host(HOST), username(USERNAME), password(PASSWORD), dbName(DB_NAME), name(DEFAULT_NAME), query_string(sqlStatement){
    setup();
}
sqlConnection::sqlConnection(const QString sqlStatement):
    host(HOST), username(USERNAME), password(PASSWORD), dbName(DB_NAME), name(DEFAULT_NAME), query_string(sqlStatement.toStdString()){
    setup();
}
sqlConnection::sqlConnection(Query query, queryType type) :
    host(HOST), username(USERNAME), password(PASSWORD), dbName(DB_NAME), name(DEFAULT_NAME), query_string(""){
    setup();
    this->setQuery(query, type);
}
//sqlConnection::sqlConnection(QueryPtr q): host(HOST), username(USERNAME), password(PASSWORD), dbName(DB_NAME), name(DEFAULT_NAME), open(false){
//    setup();
//    this->setQuery(q);
//}

void sqlConnection::setup(){
    this->lastResult = pqxx::result();
    this->credentialString = QString("dbname=%1 user=%2 password=%3 hostaddr=%4 port=5432").arg(dbName).arg(username).arg(password).arg(host).toStdString();
    this->databaseConnection = new pqxx::connection(credentialString);
    if (!databaseConnection->is_open()){
        qWarning("Error Opening Database Connection");
        return;
    }
}
sqlConnection::~sqlConnection(){
    this->databaseConnection->disconnect();
    this->databaseConnection->deactivate();
    delete databaseConnection;
}

/** \brief Check if any records were returned from the last query ran. */
bool sqlConnection::foundMatch(){
    return (countResults() > 0);
}

/** \brief Clear stored data pertaining to the last run query*/
void sqlConnection::clear(){
    this->lastResult = pqxx::result();
    this->query.clear();
    this->query_string.clear();
}

/** \brief Close the database Connection */
void sqlConnection::disconnect(){
    databaseConnection->disconnect();
}
/** \brief Set the SQL Statement that will be run when 'execute' is called. */
void sqlConnection::setQuery(const std::string query_text){
    this->query_string = query_text;
}
/** \brief Use a Query Object & queryType Enum to set up the SQL statement that will be run
 *          when 'execute' is called. */
void sqlConnection::setQuery(Query q, queryType type){
    this->query = q;
    this->query_string = q.toPqxxQuery(type);
}
/** \brief Return the sql statement that is currently stored */
QString sqlConnection::getQuery(){
    return this->query_string.c_str();
}
/** \brief Check if the database connection is still active. */
bool sqlConnection::isConnected(){
    return this->databaseConnection->is_open();
}

/** \brief Check that the query is not empty and the conneciton has been established. */
bool sqlConnection::verify(){
    bool success = false;
    if (query_string.empty()){
        qWarning("Cannot Run Empty Query");
    } else if (!databaseConnection->is_open()){
        qDebug("Connection Not Open. Attemting to Activate Connection...");
        databaseConnection->activate();
        if (databaseConnection->is_open()){
            qDebug("Success!");
            success = true;
        }  else {
            qWarning("Failed to activate the connection.");
        }
    } else {
        success = true;
        qDebug("pqxx connection okay.");
    }
    return success;
}


/** \brief Execute a PQXX Query that has been previously stored. */
bool sqlConnection::execute(){
    bool success = false;
    if (verify()){
        try{
            const char* sql_statement = this->query_string.c_str();
            nontransaction session(*databaseConnection);
            this->lastResult = pqxx::result(session.exec(sql_statement));
            session.commit();
            success = true;
        } catch (std::exception &e){
            qWarning("Caught Exception while running query '%s':\n\t%s\n",query_string.c_str(), e.what());
        }
    }
    return success;
}

/** \brief Run the provided SQL Statement on the database */
bool sqlConnection::execute(std::string sqlStatement){
    this->query_string = sqlStatement;
    return execute();
}

/** \brief Return the result of the last query */
pqxx::result sqlConnection::getResult(){
    return this->lastResult;
}

/** \brief Return the number of records that was returned by the last query */
int sqlConnection::countResults(){
    int records = 0;
    if (!this->lastResult.empty()){
        records = (int)lastResult.size();
    }
    return records;
}
