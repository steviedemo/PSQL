#ifndef QUERY_H
#define QUERY_H
#include "sql_definitions.h"
#include <QMap>
#include <QSharedPointer>
#include <QSqlQuery>
#include <QSqlDatabase>
class Query
{
public:
    Query(QString query_text="");
    ~Query();

    void setTable       (QString table);
    QString getTable    (void)  const;
    QMap<QString,QString>    getData     (void)  const;
    bool    mapEmpty    (void)  const;
    bool    isEmpty     (void)  const;
    void add            (QString key);
    void add            (QString key, double value);
    void add            (QString key, QString value);
    void add            (QString key, int value);
    void add            (QString key, class QDate value);
    void add            (QString key, class QDateTime value);
    void addCriteria    (QString key, double value);
    void addCriteria    (QString key, QString value);
    void addCriteria    (QString key, int value);
    void addCriteria    (QString key, class QDate value);
    void addCriteria    (QString key, class QDateTime value);
    void addCriteria    (QString key, class Height value);
    QString sqlSafe     (int i);
    static QString sqlSafe     (double d);
    static QString sqlSafe     (QString s);
    static QString sqlSafe     (class QDate);
    static QString sqlSafe     (class QDateTime d);

    QueryPtr verifyData   (DatabasePtr db);
    QueryPtr toSqlQuery   (queryType type, DatabasePtr db, bool &ok);
    QueryPtr toSelectQuery(DatabasePtr db, bool &ok);
    QueryPtr toInsertQuery(DatabasePtr db, bool &ok);
    QueryPtr toUpdateQuery(DatabasePtr db, bool &ok);


private:
    QMap<QString, QString> data;
    QMap<QString, QString> criteria;
    QString queryString;
    QString tableName;
};

#endif // QUERY_H
