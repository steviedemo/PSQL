#include "Query.h"
#include <QDate>
#include <QDateTime>
#include <QString>
enum type { QUERY_INSERT, QUERY_UPDATE, QUERY_SELECT };
Query::Query(QString query_text):
    queryString(query_text){
}

Query::~Query(){}
void Query::setTable(QString table){
    this->tableName = table;
}
QString Query::getTable() const {   return tableName;       }
QMap    Query::getData()  const {   return data;            }
bool    Query::mapEmpty() const {   return data.isEmpty();  }
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
            q->addBindValue(param);
        }
        qDebug("Successfully Prepared Query: '%s'", qPrintable(queryString));
    }
    return q;
}

QueryPtr Query::toSelectQuery(QString query, QSqlDatabase db, bool &ok){
    QueryPtr q = QSharedPointer<QSqlQuery>(new QSqlQuery(db));
    q->setForwardOnly(true);
    ok = q->prepare(query);
    if (!ok){
        qWarning("Error Preparing SELECT Query: %s\n\tCommand: %s", qPrintable(db.lastError().text()), qPrintable(query));
    }
    return q;
}

QueryPtr Query::toUpdateQuery(QSqlDatabase db, bool &ok){
    QueryPtr q = verifyData(db);
    QStringList parameters;
    ok = !q.isNull();
    if (!ok) {  return q;   }

    QString fields(""), values(""), whereString("");
    QStringList args;
    QMapIterator<QString, QString> it(data);
    while(it.hasNext()){
        it.next();
        fields += it.key() + (it.hasNext() ? ", " : "");
        values += "?" + (it.hasNext()) ? ", " : "";
        args << it.value();
    }
    it = criteria;
    while(it.hasNext()){
        whereString.append(QString("%1 = ?").arg(it.key()));
        args << it.value();
        if (it.hasNext()){
            whereString.append(" AND ");
        }
    }
    QString text = QString("UPDATE %1 SET (%2) = (%3) WHERE %4;").arg(tableName).arg(fields).arg(values).arg(whereString);
    ok = q->prepare(text);
    if (!ok){
        qWarning("Error Preparing UPDATE Query '%s':\n\t%s", qPrintable(text), qPrintable(db.lastError().text()));
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
    int idx = 0;
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
        qWarning("Error Preparing INSERT Query: %s\n\tCommand: %s", qPrintable(db.lastError().text()), qPrintable(queryString));
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

void Query::add(QString key){
    data.insert(key, "");
}


void Query::add(QString key, double value)              {   data.insert(key, sqlSafe(value));   }
void Query::add(QString key, int value)                 {   data.insert(key, sqlSafe(value));   }
void Query::add(QString key, QString value)             {   data.insert(key, sqlSafe(value));   }
void Query::add(QString key, QDate value)               {   data.insert(key, sqlSafe(value));   }
void Query::add(QString key, QDateTime value)           {   data.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, double value)      {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, int value)         {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, QString value)     {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, QDate value)       {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, QDateTime value)   {   criteria.insert(key, sqlSafe(value));   }

QString Query::sqlSafe  (int i)         {   return QString("%1").arg(i);        }
QString Query::sqlSafe  (double d)      {   return QString::number(d, 'f', 2);  }
QString Query::sqlSafe  (QDateTime d)   {   return sqlSafe(d.date());           }
QString Query::sqlSafe  (QDate d)    {
    QString s("");
    if (d.isValid() && !d.isNull())
        s=d.toString("'yyyy-MM-dd'");
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
