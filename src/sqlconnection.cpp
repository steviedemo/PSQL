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
#define trace(); //qDebug("%s::%s::%d", __FILE__, __FUNCTION__, __LINE__);
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
    try{
        this->lastResult = pqxx::result();
        this->credentialString = QString("dbname=%1 user=%2 password=%3 hostaddr=%4 port=5432").arg(dbName).arg(username).arg(password).arg(host).toStdString();
        this->databaseConnection = new pqxx::connection(credentialString);
        if (!databaseConnection->is_open()){
            qWarning("Error Opening Database Connection");
            return;
        }
    } catch(std::exception &e){
        qWarning("Caught Exception while attempting to set up new sqlConnection object: %s", e.what());
    }
}
sqlConnection::~sqlConnection(){
    try{
        this->databaseConnection->disconnect();
        this->databaseConnection->deactivate();
        if (databaseConnection){
            delete databaseConnection;
        }
    } catch (std::exception &e){
        qWarning("Caught Exception in sqlConnection Destructor: %s", e.what());
    }
}

/** \brief Check if any records were returned from the last query ran. */
bool sqlConnection::foundMatch(){
    return (countResults() > 0);
}

/** \brief Clear stored data pertaining to the last run query*/
void sqlConnection::clear(){
    try{
        this->lastResult = pqxx::result();
        this->query.clear();
        this->query_string.clear();
    } catch(std::exception &e){
        qWarning("Caught Exception while trying to clear out member data. %s", e.what());
    }
}

/** \brief Close the database Connection */
void sqlConnection::disconnect(){
    try{
        databaseConnection->disconnect();
    } catch(std::exception &e){
        qWarning("Caught Exception while attempting to close database Connection: %s", e.what());
    }
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
    QString currentQuery("");
    try{
        if (!query_string.empty()){
            currentQuery = QString::fromStdString(query_string);
        }
    } catch (std::exception &e){
        qWarning("Caught Exception while attempting to retrieve the currently set SQL statement: %s", e.what());
    }
    return currentQuery;
}
/** \brief Check if the database connection is still active. */
bool sqlConnection::isConnected(){
    bool open = false;
    try{
        open = this->databaseConnection->is_open();
    } catch(std::exception &e){
        qWarning("Caught Exception while checking if database connection is open: %s", e.what());
    }
    return open;
}

/** \brief Check that the query is not empty and the conneciton has been established. */
bool sqlConnection::verify(){
    bool success = false;
    try{
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
        }
    } catch (std::exception &e){
        qWarning("Caught Exception while verifying database connection and SQL Statement: %s", e.what());
        success = false;
    }
    return success;
}

/** \brief Execute a PQXX Query based on the provided Query Object & queryType specifier. */
bool sqlConnection::execute(Query q, queryType t){
    std::string sql_statement = q.toPqxxQuery(t, "scenes");
    return this->execute(sql_statement);
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
    bool success = false;
    if (!sqlStatement.empty()){
        this->query_string = sqlStatement;
        try{
            success = execute();
        } catch(std::exception &e){
            qWarning("Caught Exception while executing %s: %s", sqlStatement.c_str(), e.what());
            success = false;
        }
    }
    return success;
}

/** \brief Return the result of the last query */
pqxx::result sqlConnection::getResult(){
    pqxx::result previous;
    try{
        previous = lastResult;
    } catch(std::exception &e){
        qWarning("Caught Exception while attempting to obtain the last result. %s", e.what());
    }
    return previous;
}

/** \brief Return the number of records that was returned by the last query */
int sqlConnection::countResults(){
    int records = 0;
    try {
        if (!this->lastResult.empty()){
            records = (int)lastResult.size();
        }
    } catch(std::exception &e){
        qWarning("Caught Exception in sqlConnection::countResults - %s", e.what());
    }

    return records;
}
