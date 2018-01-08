#include "Query.h"
#include <QDate>
#include <QDateTime>
#include <QRegularExpression>
#include <QString>
#include <QSqlQuery>
#include <QTextStream>
#include "Height.h"
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
 *          Insert an entry into the appropriate table.                             */
std::string Query::toPqxxInsert(QString table, bool verbose){
    if (!table.isEmpty()){
        this->tableName = table;
    }
    QString keyString(""), valueString("");
    QMapIterator<QString, QString> it(data);
    while(it.hasNext()){
        it.next();
        QString key = it.key();
        QString value = it.value();
        if (!value.isEmpty() && value != "0" && !value.contains("Unknown")){
            if(!value.startsWith('\'')) {   value.prepend('\'');    }
            if(!value.endsWith('\''))   {   value.append('\'');     }
            valueString += value + (it.hasNext() ? ", " : "");
            keyString   += key   + (it.hasNext() ? ", " : "");
          //  qDebug("Query Adding Field '%s' = %s", qPrintable(key), qPrintable(value));
        }
    }
    this->queryString = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(this->tableName).arg(keyString).arg(valueString);
    if (verbose){
        //printQuery();
    }
    return queryString.toStdString();
}

QSqlQuery Query::toQSqlUpdate(QString table){
    if (!table.isEmpty()){
        this->tableName = table;
    }
    bool useNameAsCriteria = (criteria.isEmpty() && data.contains("NAME"));
    this->queryString.clear();
    QString whereString(""), fields(""), valueField("");
    QMapIterator<QString, QString> it(data);
    QStringList values;
    while(it.hasNext()){
        it.next();
        QString key = it.key();
        QString value = it.value();
        if (!value.isEmpty()){
            if (key == "NAME" && useNameAsCriteria){
                // Skip this one if we're using it as criteria.
            } else {
                fields += key + (it.hasNext() ? ", " : "");
                valueField += QString(":%1").arg(key) + (it.hasNext() ? ", " : "");
                QString value = it.value();
                if (!value.startsWith("'")){
                    value = QString("'%1'").arg(it.value());
                    data[key] = value;
                }
            }
        }
    }
    QStringList whereArgs;
    if (!criteria.isEmpty()){
        it = criteria;
        while(it.hasNext()){
            it.next();
            if (!it.value().isEmpty()){
                whereString.append(QString("%1 = :%1").arg(it.key()).arg(it.value()));
                if (it.hasNext()){
                    whereString.append(" AND ");
                }
                whereArgs << it.value();
                data.insert(it.key(), it.value());
            }
        }
    } else if (data.contains("NAME")){
        QString name = data.value("NAME");
        if (!name.startsWith('\'')) {   name.prepend('\''); }
        if (!name.endsWith('\''))   {   name.append('\'');  }
        whereString = QString("NAME = :NAME");
    }
    QSqlQuery query;
    this->queryString = QString("UPDATE %1 SET (%2) = (%3) WHERE %4;").arg(tableName).arg(fields).arg(valueField).arg(whereString);
    if (!query.prepare(queryString)){
        qWarning("error preparing update string");
    }
    it = data;
    while(it.hasNext()){
        it.next();
        //query.addBindValue(QString(":%1").arg(it.key()), it.value());
    }
    return query;
}

/** \brief Build a std::string SQL Statement that can be used by the pqxx library to
 *          Update an entry in the appropriate table.                             */
std::string Query::toPqxxUpdate(QString table, bool verbose){
    if (!table.isEmpty()){
        this->tableName = table;
    }
  //  qDebug("Assembling Query Statement for Updating Actor Profile in Database");
    bool useNameAsCriteria = (criteria.isEmpty() && data.contains("NAME"));
    this->queryString.clear();
    QString whereString(""), fields(""), values("");
    QMapIterator<QString, QString> it(data);
    while(it.hasNext()){
        it.next();
        QString key = it.key();
        QString value = it.value();
   //     qDebug("%s :: %s", qPrintable(key), qPrintable(value));
        if (!value.isEmpty()){
            if (key == "NAME" && useNameAsCriteria){
                // Skip this one if we're using it as criteria.
            } else {
                fields += it.key() + (it.hasNext() ? ", " : "");
                QString value = it.value();

                if (!value.startsWith("'")){
                    value = QString("'%1'").arg(it.value());
                }
                values += value + ((it.hasNext()) ? ", " : "");
            }
        }
    }
 //   qDebug("Adding Criteria...");
    if (!criteria.isEmpty()){
        QMapIterator<QString, QString> crIt(criteria);
        while(crIt.hasNext()){
            crIt.next();
    //        qDebug("%s :: %s", qPrintable(crIt.key()), qPrintable(crIt.value()));
            if (!crIt.value().isEmpty()){
                whereString.append(QString("%1 = %2").arg(crIt.key()).arg(crIt.value()));
                if (crIt.hasNext()){
                    whereString.append(" AND ");
                }
            }
        }
    } else if (data.contains("NAME")){
        QString name = data.value("NAME");
        if (!name.startsWith('\'')) {   name.prepend('\''); }
        if (!name.endsWith('\''))   {   name.append('\'');  }
        whereString = QString("NAME = %1").arg(name);
    } else {
        qWarning("Error Making update SQL Statement - No Criteria provided.");
    }
    this->queryString = QString("UPDATE %1 SET (%2) = (%3) WHERE %4;").arg(tableName).arg(fields).arg(values).arg(whereString);
    if (verbose){
   //     printQuery();
    }
    return queryString.toStdString();
}

/** \brief Build a std::string SQL Statement that can be used by the pqxx library to
 *          Select one or more entries from the appropriate table.
 *  \param  QString table:  name of table to query. Defaults to the current default table name.
 *  \param  bool verbose:   defaults to true. Print out query after building.
 *  \return std::string     string literal containing SQL SELECT statement.
 */
std::string Query::toPqxxSelect(QString table, bool verbose){
    this->queryString.clear();
    if (!table.isEmpty()){
        this->tableName = table;
    }
    bool proceed = false, useNameAsCriteria = false;
    if (criteria.isEmpty()){
        proceed = data.contains("NAME") && !data.value("NAME").isEmpty();
        useNameAsCriteria = proceed;
    } else {
        proceed = true;
        useNameAsCriteria = false;
    }
    if (!proceed){
        qWarning("Can't create SELECT query with empty criteria");
    } else {
        this->queryString = "SELECT ";
        QMapIterator<QString, QString> it(data);
        while(it.hasNext()){
            it.next();
            QString key = it.key();
            if (!key.isEmpty()){
                queryString += key + (it.hasNext() ? " AND " : "");
            }
        }
        queryString.append(QString("FROM %1 WHERE ").arg(tableName));
        if (useNameAsCriteria){
            QString name = data.value("NAME");
            if (!name.startsWith('\'')) {   name.prepend('\''); }
            if (!name.endsWith('\''))   {   name.append('\'');  }
            queryString += "NAME = " + name;
        } else {
            it = criteria;
            while(it.hasNext()){
                it.next();
                QString key = it.key();
                QString value = it.value();
                if (!key.isEmpty() && !value.isEmpty()){
                    queryString += QString("%1 = %2").arg(key).arg(value) + (it.hasNext() ? " AND " : "");
                }
            }
        }
    }
    if (verbose){
    //    printQuery();
    }
    return this->queryString.toStdString();
}

void Query::add(QString key, QDate value) {
    QString sqlValue("");
    if (!value.isNull() && value.isValid()){
        sqlValue = value.toString("yyyy-MM-dd");
        sqlValue.prepend('\'');
        sqlValue.append('\'');
        data.insert(key, sqlValue);
    }
}
void Query::add(QString key, QDateTime value) {
    QDate date = value.date();
    add(key, date);
}

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
void Query::add(QString key, Height height){
    if (height.getFeet() == 0){
        QString value = QString("\'%1\'\'%2\"\'").arg(height.getFeet()).arg(height.getInches());
    //    qDebug("Adding Height to Query: %s", qPrintable(value));
        data.insert(key, value);
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
            && value != "/." && !value.contains(QRegularExpression("[\\<\\>]"))\
            && value != "0'00\""){
        QString entry = QString("%1").arg(value);
        if(entry.startsWith('\''))
            entry.remove(0, 1);
        if (entry.endsWith('\''))
            entry.remove(QRegularExpression("\\\'$"));
        entry.replace('\'', "\'\'");
        QString newEntry = QString("\'%1\'").arg(entry);
        if (newEntry != "''"){
            data.insert(key, newEntry);
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
        s=d.toString("\'yyyy-MM-dd\'");
    else
        s = "''";
    return s;
}

QString Query::sqlSafe(QString s){
    // Any apostrophe needs to be escaped with a second apostrophe. Semicolons and Quotation marks must be removed outright.
    QString temp = s.replace('\'', "\'\'");
    temp = temp.remove('\"');
    temp = temp.remove(';');
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
    temp.prepend("\'");
    temp.append("\'");
    return temp;
}
