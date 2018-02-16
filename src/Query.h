#ifndef QUERY_H
#define QUERY_H
#include "sql_definitions.h"
#include <QMap>
#include <QStringList>
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
    void                    add         (const QString &key, const double &value);
    void                    add         (const QString &key, const QString &value);
    void                    add         (const QString &key, const int &value);
    void                    add         (const QString &key, const qint64 &value);
    void                    add         (const QString &key, const class QDate &value);
    void                    add         (const QString &key, const class QDateTime &value);
    void                    add         (QString key, class Height);
    void                    addCriteria (const QString &key, const double &value);
    void                    addCriteria (const QString &key, const QString &value);
    void                    addCriteria (const QString &key, const int &value);
    void                    addCriteria (const QString &key, const class QDate &value);
    void                    addCriteria (const QString &key, const class QDateTime &value);
    void                    addCriteria (const QString &key, const class Height &value);

    /// using pqxx class
    std::string             toPqxxInsert    (QString table, bool verbose=true);
    std::string             toPqxxUpdate    (QString table, bool verbose=true);
    std::string             toPqxxSelect    (QString table, bool verbose=true);
    std::string             toPqxxQuery     (queryType type, QString table="");
#ifdef QSQLQUERY
    QSqlQuery               toQSqlInsert    (QString table="");
    QSqlQuery               toQSqlUpdate    (QString table="");
    QSqlQuery               toQSqlSelect    (QString table="");
    QSqlQuery               toQSqlQuery     (QString table="");
#endif
    static QString          sqlSafe (const int i) ;
    static QString          sqlSafe (const double d) ;
    static QString          sqlSafe (const QString s) ;
    static QString          sqlSafe (const class QDate) ;
    static QString          sqlSafe (const class QDateTime d) ;

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
