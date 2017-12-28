#include "Query.h"
#include <QDate>
#include <QDateTime>
#include <QRegularExpression>
#include <QString>
#include <QSqlQuery>
#include <QTextStream>
enum type { QUERY_INSERT, QUERY_UPDATE, QUERY_SELECT };
Query::Query(QString query_text):
    queryString(query_text){
}

Query::~Query(){}

void                    Query::addSelection(QString field)  {   this->selectFields << field;}
void                    Query::setCriteria(QString where)   {   this->criteriaString = where;   }
void                    Query::setTable(QString table)      {   this->tableName = table;    }
QString                 Query::getTable() const             {   return tableName;           }
bool                    Query::mapEmpty() const             {   return data.isEmpty();      }
QMap<QString, QString>  Query::getData()  const             {   return data;                }
void                    Query::selectAll()                  {    selectFields.clear(); selectFields << "*"; }
void    Query::printQuery(){
    qDebug("Assembled Query:\n\t%s\n", qPrintable(queryString));
}

void Query::clear(){
    this->data.clear();
    this->criteria.clear();
    this->selectFields.clear();
    this->queryString.clear();
    this->criteriaString.clear();
}

bool Query::isEmpty() const{
    bool empty = true;
    if (!data.isEmpty()){
        empty = false;
    } else if (!criteria.isEmpty()) {
        empty = false;
    } else if (!queryString.isEmpty()){
        empty = false;
    }
    return empty;
}

QueryPtr Query::verifyData(DatabasePtr db){
    QueryPtr q = QSharedPointer<QSqlQuery>(0);
    if (!this->isEmpty()){
        qWarning("Error creating QSqlQuery object - Query object is Empty!");
    } else {
        q = QueryPtr(new QSqlQuery(*(db.data())));
        q->setForwardOnly(true);
    }
    return q;
}

QueryPtr Query::toSqlQuery(queryType type, DatabasePtr db, bool &ok){
    QueryPtr q;
    if (type == SQL_INSERT){
        q = this->toInsertQuery(db, ok);
    } else if (type == SQL_UPDATE){
        q = this->toUpdateQuery(db, ok);
    } else {
        q = this->toSelectQuery(db, ok);
    }
    return q;
}

/** \brief use the map of key/value criteria pairs to build a sub-expression in SQL syntax
 *          that can be used as the WHERE clause in an UPDATE or SELECT query.      */
QString Query::buildWhereClause(){
    QString whereClause("");
    QTextStream out(&whereClause);
    QMapIterator<QString, QString> it(criteria);
    while(it.hasNext()){
        out << it.key() << " = " << it.value() << (it.hasNext() ? " AND " : "");
    }
    out.flush();
    return whereClause;
}

/** \brief Use the member data stored to build an SQL Expression of the type specified in the
 *          queryType argument of this function. */
std::string Query::toPqxxQuery(queryType type, QString table){
    std::string text("");
    if (type == SQL_UPDATE){
        text = this->toPqxxUpdate(table);
    } else if (type == SQL_INSERT){
        text = this->toPqxxInsert(table);
    } else {
        text = this->toPqxxSelect(table);
    }
    return text;
}

/** \brief Build a std::string SQL Statement that can be used by the pqxx library to
 *          Select one or more entries from the appropriate table.                 */
std::string Query::toPqxxSelect(QString table){
    if (!table.isEmpty()){
        this->tableName = table;
    }
    this->queryString.clear();
    QTextStream out(&queryString);
    out << "SELECT ";
    if (!selectFields.isEmpty()){
        QStringListIterator it(selectFields);
        while(it.hasNext()){
            out << it.next() << (it.hasNext() ? ", " : "");
        }
    } else {
        out << "*";
    }
    out << " FROM " << tableName;
    if (!criteriaString.isEmpty()){
        out << " WHERE " << criteriaString;
    } else if (!criteria.isEmpty()){
        out << " WHERE " << buildWhereClause();
    }
    out.flush();
    printQuery();
    return queryString.toStdString();
}
/** \brief Build a std::string SQL Statement that can be used by the pqxx library to
 *          Update an entry in the appropriate table.                             */
std::string Query::toPqxxUpdate(QString table){
    if (!table.isEmpty()){
        this->tableName = table;
    }
    this->queryString.clear();
    QString whereString(""), fields(""), values("");
    QMapIterator<QString, QString> it(data);
    while(it.hasNext()){
        it.next();
        if (!it.value().isEmpty() && it.key() != "NAME"){
            fields += it.key() + (it.hasNext() ? ", " : "");
            QString value = it.value();
            if (!value.startsWith("'")){
                value = QString("'%1'").arg(it.value());
            }
            values += value + ((it.hasNext()) ? ", " : "");
        }
    }
    if (!criteria.isEmpty()){
        it = criteria;
        while(it.hasNext()){
            if (!it.value().isEmpty()){
                whereString.append(QString("%1 = %2").arg(it.key()).arg(it.value()));
                if (it.hasNext()){
                    whereString.append(" AND ");
                }
            }
        }
    } else if (data.contains("NAME")){
        whereString = QString("NAME = %1").arg(data.value("NAME"));
    } else {
        qWarning("Error Making update SQL Statement - No Criteria provided.");
    }
    this->queryString = QString("UPDATE %1 SET (%2) = (%3) WHERE %4;").arg(tableName).arg(fields).arg(values).arg(whereString);
    printQuery();
    return queryString.toStdString();
}
/** \brief Build a std::string SQL Statement that can be used by the pqxx library to
 *          Insert an entry into the appropriate table.                             */
std::string Query::toPqxxInsert(QString table){
    if (!table.isEmpty()){
        this->tableName = table;
    }
    QString keys(""), values("");
    QTextStream keyOut(&keys), valueOut(&values);
    QMapIterator<QString, QString> it(data);
    while(it.hasNext()){
        it.next();
        if (!it.value().isEmpty()){
            keyOut   << it.key()   << (it.hasNext() ? ", " : "");
            valueOut << it.value() << (it.hasNext() ? ", " : "");
        }
    }
    keyOut.flush();
    valueOut.flush();
    this->queryString = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(this->tableName).arg(keys).arg(values);
    printQuery();
    return queryString.toStdString();
}


QueryPtr Query::toSelectQuery(DatabasePtr db, bool &ok){
    QueryPtr q = verifyData(db);
    QStringList parameters;
    ok = !q.isNull();
    if (!ok) {  return q;   }
    /// Set up Query String
    this->queryString = "SELECT ";
    QMapIterator<QString, QString> it(data);
    while(it.hasNext()){
        it.next();
        queryString += it.key() + (it.hasNext() ? ", " : "");
    }
    this->queryString.append(" FROM " + this->tableName);
    if (!criteria.isEmpty()){
        this->queryString.append(" WHERE ");
        it = criteria;
        while(it.hasNext()){
            it.next();
            this->queryString += QString("%1 = ?").arg(it.key());
            parameters << it.value();
            if (it.hasNext()){
                this->queryString.append(" AND ");
            }
        }
    }
    /// Set up QueryPointer Object
    if (!q->prepare(queryString)){
        qWarning("Error Preparing Query: '%s'", qPrintable(queryString));
        return q;
    } else {
        foreach(QString param, parameters){
            q->addBindValue(param.toLocal8Bit());
        }
        printQuery();
    }
    return q;
}

QueryPtr Query::toSelectQuery(QString query, DatabasePtr db, bool &ok){
    QueryPtr q = QSharedPointer<QSqlQuery>(new QSqlQuery(*(db.data())));
    q->setForwardOnly(true);
    ok = q->prepare(query);
    if (!ok){
        qWarning("Error Preparing SELECT Query:\n\t%s", qPrintable(query));
    }
    return q;
}

QueryPtr Query::toUpdateQuery(DatabasePtr db, bool &ok){
    QueryPtr q = verifyData(db);
    ok = !q.isNull();
    if (!ok) {  return q;   }

    QString fields(""), values(""), whereString("");
    QStringList args;
    QMapIterator<QString, QString> it(data);
    while(it.hasNext()){
        it.next();
        if (!it.value().isEmpty() && it.key() != "NAME"){
            fields += it.key() + (it.hasNext() ? ", " : "");
            values += QString("?") + ((it.hasNext()) ? ", " : "");
            args << it.value();
        }
    }
    if (!criteria.isEmpty()){
        it = criteria;
        while(it.hasNext()){
            whereString.append(QString("%1 = ?").arg(it.key()));
            args << it.value();
            if (it.hasNext()){
                whereString.append(" AND ");
            }
        }
    } else if (data.contains("NAME")){
        whereString = QString("NAME = %1").arg(data.value("NAME"));
    }
    QString text = QString("UPDATE %1 SET (%2) = (%3) WHERE %4;").arg(tableName).arg(fields).arg(values).arg(whereString);
    ok = q->prepare(text);
    if (!ok){
        qWarning("Error Preparing UPDATE Query '%s'", qPrintable(text));
        return q;
    } else {
        foreach(QString arg, args){
            q->addBindValue(arg);
        }
    }
    return q;
}


QueryPtr Query::toInsertQuery(DatabasePtr db, bool &ok){
    QueryPtr q = verifyData(db);
    ok = !q.isNull();
    if (!ok) {  return q;   }
    QString fields(""), values("");
    QMapIterator<QString, QString>it(data);
    while(it.hasNext()){
        it.next();
        fields.append(it.key());
        values.append("?");
        if (it.hasNext()){
            fields.append(", ");
            values.append(", ");
        }
    }
    QString queryString = QString("INSERT INTO %1 ( %2 ) VALUES ( %3 )").arg(tableName).arg(fields).arg(values);
    q->setForwardOnly(true);
    if (!q->prepare(queryString)){
        qWarning("Error Preparing INSERT Query\n\t%s", qPrintable(queryString));
        ok = false;
        return q;
    } else {
        it = data;
        while(it.hasNext()){
            it.next();
            q->addBindValue(it.value());
        }
        ok = true;
    }
    return q;
}


void Query::add(QString key, QDate value)               {   data.insert(key, sqlSafe(value));   }
void Query::add(QString key, QDateTime value)           {   data.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, double value)      {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, int value)         {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, QString value)     {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, QDate value)       {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, QDateTime value)   {   criteria.insert(key, sqlSafe(value));   }

void Query::add(QString key){
    data.insert(key, "");
}

void Query::add(QString key, int value){
    if (value > 0){
        data.insert(key, QString::number(value));
    }
}

void Query::add(QString key, double value){
    if (value > 0.0){
        data.insert(key, QString::number(value, 'f', 2));
    }
}
/** \brief Add a QString Value to the map of Query Parameters */
void Query::add(QString key, QString value)             {
    if (!value.isNull() && !value.isEmpty() \
            && value != "/." && !value.contains(QRegularExpression("[\\[\\]\\<\\>]"))\
            && value != "0'00\""){
        QString sqlSafeValue = sqlSafe(value);
        if (sqlSafeValue != "''"){
            data.insert(key, value);
        }
    }
}


QString Query::sqlSafe  (int i) {
    QString value("");
    if (i != 0){
        value = QString("%1").arg(i);
    }
    return value;
}

QString Query::sqlSafe  (double d) {
    QString value("");
    if (d != 0.0){
        value = QString::number(d, 'f', 2);
    }
    return value;
}
QString Query::sqlSafe  (QDateTime d)   {   return sqlSafe(d.date());           }
QString Query::sqlSafe  (QDate d)       {
    QString s("");
    if (d.isValid() && !d.isNull())
        s=d.toString("'yyyy-MM-dd'");
    else
        s = "''";
    return s;
}

QString Query::sqlSafe(QString s){
    // Any apostrophe needs to be escaped with a second apostrophe. Semicolons and Quotation marks must be removed outright.
    QString temp = s.replace("'", "''").remove('\"').remove(';');
    // Remove any starting/ending apostrophes.
    try{
        while(temp.startsWith("'")){
            temp = temp.remove(0, 1);
        }
        while(temp.endsWith("'")){
            int lastIndex = temp.size() - 1;
            temp = temp.remove(lastIndex, 1);
        }
    } catch (std::exception &e) {
        qWarning("Caught Exception While Creating Sql Safe String from '%s':\t%s", qPrintable(temp), e.what());
    } catch (...) {
        qWarning("Caught Unknown Exception while creating SQL safe string from '%s'", qPrintable(temp));
    }
    return QString("'%1'").arg(temp);
}
