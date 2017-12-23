#include "Query.h"
#include "Height.h"
#include "FilePath.h"
#include "Rating.h"
#include <QDate>
#include <QDateTime>
#include <QString>
enum type { QUERY_INSERT, QUERY_UPDATE, QUERY_SELECT };
Query::Query(QString query_text):
    queryString(query_text){
}


QSqlQuery *Query::toSelectQuery(QString query, QSqlDatabase db, bool &ok){
    QSqlQuery *q = new QSqlQuery(db);
    q->setForwardOnly(true);
    ok = q->prepare(query);
    if (!ok){
        qWarning("Error Preparing SELECT Query: %s\n\tCommand: %s", qPrintable(db.lastError().text()), qPrintable(query));
    }
    return q;
}

QSqlQuery *Query::toUpdateQuery(QString tableName, QSqlDatabase db, bool &ok){
    QSqlQuery *q = new QSqlQuery(db);
    QString fields(""), values(""), whereString("");
    QStringList args;
    QMapIterator<QString, QString> dataIt(data);
    while(dataIt.hasNext()){
        dataIt.next();
        fields.append(dataIt.key());
        values.append("?");
        if (dataIt.hasNext()){
            fields.append(", ");
            values.append(", ");
        }
        args << dataIt.value();
    }
    QMapIterator<QString,QString> whereIt(criteria);
    while(whereIt.hasNext()){
        whereString.append(QString("%1 = ?").arg(whereIt.key()));
        args << whereIt.value();
        if (whereIt.hasNext()){
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

}

QSqlQuery *Query::toInsertQuery(QString tableName, QSqlDatabase db, bool &ok){
    QSqlQuery *q = new QSqlQuery(db);
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
        QMapIterator<QString, QString> valIt(data);
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

void Query::add(QString key, double value)      {   data.insert(key, sqlSafe(value));   }
void Query::add(QString key, int value)         {   data.insert(key, sqlSafe(value));   }
void Query::add(QString key, QString value)     {   data.insert(key, sqlSafe(value));   }
void Query::add(QString key, QDate value)       {   data.insert(key, sqlSafe(value));   }
void Query::add(QString key, QDateTime value)   {   data.insert(key, sqlSafe(value));   }
void Query::add(QString key, FilePath value)    {   data.insert(key, sqlSafe(value));   }
void Query::add(QString key, Height value)      {   data.insert(key, sqlSafe(value.toString());  }
void Query::add(QString key, Rating value)      {   data.insert(key, sqlSafe(value.toString())); }
void Query::addCriteria(QString key, double value)      {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, int value)         {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, QString value)     {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, QDate value)       {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, QDateTime value)   {   criteria.insert(key, sqlSafe(value));   }
void Query::addCriteria(QString key, Height value)      {   criteria.insert(key, value.sqlSafe());  }
void Query::addCriteria(QString key, FilePath value)    {   criteria.insert(key, sqlSafe(value));   }


QString Query::sqlSafe(QDateTime d){
    QString s("");
    if (valid(d)){
        s=d.toString("'yyyy-MM-dd'");
    }
    return s;
}

QString Query::sqlSafe(QDate d)    {
    QString s("");
    if (d.isValid() && !d.isNull())
        s=d.toString("'yyyy-MM-dd'");
    return s;
}

QString Query::sqlSafe(FilePath f){
    return sqlSafe(f.absolutePath());
}

QString Query::sqlSafe(QString s){
    QString sql("");
    if (s.isEmpty())
        return sql;
    int f_offset = 0, b_offset = 0;
    sql.append("'");
    try{
        if (s.startsWith("'"))  {   f_offset = 1;   }
        if (s.endsWith("'"))    {   b_offset = 1;   }
        for (int i = 0; i < s.size(); ++i){
            QChar c = s.at(i);
            if (c == '\'')                    // escape apostrophes
                sql.append("''");
            else if (c != '\"' && c != ';') // don't include semicolons or quotation marks
                sql.append(c);
         }
         int lastPosition = sql.size() - 1;
         while(sql.at(lastPosition) == '\''){
            sql.remove(lastPosition);
            lastPosition = sql.size() - 1;
         }
    } catch (std::exception &e) {
        qDebug("Caught Exception While Creating Sql Safe String from '%s':\t%s", qPrintable(s), e.what());
    } catch (...) {
        qDebug("Caught Unknown Exception while creating SQL safe string from '%s'", qPrintable(s));
    }
    sql.append("'");
    return sql;
}
