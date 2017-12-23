#ifndef QUERY_H
#define QUERY_H
#include <QMap>
#include <QSqlQuery>
#include <QSqlDatabase>
class Query
{
public:
    Query(QString query_text="");
    ~Query();
    QSqlQuery *toQSqlQuery(QSqlDatabase db);
    QSqlQuery *toInsertQuery(QString table, QSqlDatabase db, bool &ok);
    QSqlQuery *toUpdateQuery(QString table, QSqlDatabase db, bool &ok);
    QSqlQuery *toSelectQuery(QString table, QSqlDatabase db, bool &ok);
    void add            (QString key);
    void add            (QString key, double value);
    void add            (QString key, QString value);
    void add            (QString key, int value);
    void add            (QString key, class QDate value);
    void add            (QString key, class QDateTime value);
    void add            (QString key, class Height value);
    void add            (QString key, class FilePath value);
    void add            (QString key, class Rating value);

    void addCriteria    (QString key, double value);
    void addCriteria    (QString key, QString value);
    void addCriteria    (QString key, int value);
    void addCriteria    (QString key, class QDate value);
    void addCriteria    (QString key, class QDateTime value);
    void addCriteria    (QString key, class Height value);
    void addCriteria    (QString key, class FilePath value);
    void addCriteria    (QString key, class Rating value);

    static QString sqlSafe     (QString s);
    static QString sqlSafe     (QDate);
    static QString sqlSafe     (QDateTime d);
    static QString sqlSafe     (int i)              {   return QString("'%1'").arg(i);          }
    static QString sqlSafe     (double d)           {   return QString("'%1'").arg(d);          }
    static QString sqlSafe     (FilePath f);

private:
    QMap<QString, QString> data;
    QMap<QString, QString> criteria;
    QString queryString;
};

#endif // QUERY_H
