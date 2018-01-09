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
    void                    clear();
    void                    setTable    (QString table);
    QString                 getTable    (void)  const;
    QMap<QString,QString>   getData     (void)  const;
    bool                    mapEmpty    (void)  const;
    bool                    isEmpty     (void)  const;
    void                    addSelection(QString field);
    void                    selectAll   (void);
    void                    setCriteria (QString whereClause);
    void                    add         (QString key);
    void                    add         (QString key, double value);
    void                    add         (QString key, QString value);
    void                    add         (QString key, int value);
    void                    add         (QString key, qint64 value);
    void                    add         (QString key, class QDate value);
    void                    add         (QString key, class QDateTime value);
    void                    add         (QString key, class Height);
    void                    addCriteria (QString key, double value);
    void                    addCriteria (QString key, QString value);
    void                    addCriteria (QString key, int value);
    void                    addCriteria (QString key, class QDate value);
    void                    addCriteria (QString key, class QDateTime value);
    void                    addCriteria (QString key, class Height value);

    /// using pqxx class
    std::string             toPqxxInsert    (QString table, bool verbose=true);
    std::string             toPqxxUpdate    (QString table, bool verbose=true);
    std::string             toPqxxSelect    (QString table, bool verbose=true);
    std::string             toPqxxQuery     (queryType type, QString table="");
    QSqlQuery               toQSqlInsert    (QString table="");
    QSqlQuery               toQSqlUpdate    (QString table="");
    QSqlQuery               toQSqlSelect    (QString table="");
    QSqlQuery               toQSqlQuery     (QString table="");
    static QString          sqlSafe         (int i);
    static QString          sqlSafe         (double d);
    static QString          sqlSafe         (QString s);
    static QString          sqlSafe         (class QDate);
    static QString          sqlSafe         (class QDateTime d);

private:
    QString                 buildWhereClause();

    void printQuery();
    QMap<QString, QString> data;
    QMap<QString, QString> criteria;
    QStringList selectFields;
    QString queryString;
    QString criteriaString;
    QString tableName;
};

#endif // QUERY_H
