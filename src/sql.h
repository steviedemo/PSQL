#ifndef SQL_H
#define SQL_H
#include "definitions.h"
#include "sql_definitions.h"
#include "FilePath.h"
#include "sqlconnection.h"
#include <QMutex>
#include <QtSql>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include <QMap>


#define DEFAULT_NAME "default"
struct operation_count {
    int idx, added, updated;
    operation_count() :
        idx(0), added(0), updated(0){}
    int total(){ return added + updated;    }
    void reset(){
        idx = 0;
        added = 0;
        updated = 0;
    }
    void addInsert(){
        idx++;
        added++;
    }
    void addUpdate(){
        idx++;
        updated++;
    }
};


class SQL : public QObject {
    Q_OBJECT
public:
    SQL(QString connectionName=DEFAULT_NAME);
    ~SQL();
    static void purgeScenes(void);
    bool hasMatch       (QueryPtr q);
    bool hasScene       (ScenePtr s, bool &queryRan);
    bool hasActor       (ActorPtr a, bool &queryRan);
    bool insertOrUpdateActor(ActorPtr);
    bool insertOrUpdateScene(ScenePtr);
    bool modifyDatabase (QueryPtr q);
    QueryPtr queryDatabase(QString queryText, QStringList args);
    QueryPtr assembleQuery(QString queryText, QStringList args, bool &ok);

    void updateDatabase (ActorList actorList);
    void updateDatabase (SceneList sceneList);
    bool makeTable      (Database::Table);
    bool dropTable      (Database::Table);
    bool sceneSql(ScenePtr S, queryType type);
    bool actorSql(ActorPtr A, queryType type);
    // Static Functions
    static void    sqlAppend   (QString &fields, QStringList &list, QString name, QString item);
    static void    sqlAppend   (QString &fields, QString &values, QStringList &list, QString name, QString item);
    static void    sqlAppend   (QString &fields, QString &values, QString name, QString item, bool prev);
    static void    sqlAppend   (QString &fields, QString name, QString item, bool &prev);
    static void loadActorList  (QVector<QSharedPointer<class Actor>> &actors);
    static void loadSceneList  (QVector<QSharedPointer<class Scene>> &scenes);
private:
    operation_count count;
    QSqlDatabase db;
    QString connectionName;
    sqlConnection connection;
    QMutex mx;
    void startPostgres();
signals:
    void startProgress(int total);
    void updateProgress(int);
    void closeProgress();
};


#endif // SQL_H
