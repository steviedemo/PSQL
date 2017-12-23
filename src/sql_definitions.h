#ifndef SQL_DEFINITIONS_H
#define SQL_DEFINITIONS_H
#include <QSharedPointer>
#define HOST        "localhost"
#define USERNAME    "scenequery"
#define PASSWORD    "smashed"
#define SCENE_DB    "scenes"
#define ACTOR_DB    "actors"
#define DB_NAME     "scenequery"


typedef QSharedPointer<class QSqlQuery>     QueryPtr;
typedef QSharedPointer<class QSqlDatabase>  DatabasePtr;
namespace Database{
    enum Operation  { OPERATION_FETCH, OPERATION_UPDATE, OPERATION_CLEAN, OPERATION_NONE };
    enum Table      { SCENE, ACTOR, THUMBNAIL, HEADSHOT, FILMOGRAPHY };
}
enum queryType  { SQL_INSERT, SQL_UPDATE, SQL_SELECT };


#define ADB_TABLE 	"CREATE TABLE IF NOT EXISTS ACTORS("\
                    "ID				smallserial 	primary key,"\
                    "NAME 			text 			unique not null,"\
                    "ALIASES		text,"\
                    "BIRTHDAY		date,"\
                    "CITY			text,"\
                    "COUNTRY		text,"\
                    "ETHNICITY		text,"\
                    "HEIGHT			integer,"\
                    "WEIGHT			integer,"\
                    "MEASUREMENTS	text,"\
                    "HAIR			text,"\
                    "EYES			text,"\
                    "TATTOOS		text,"\
                    "PIERCINGS		text,"\
                    "PHOTO			text)"
#define SDB_TABLE	"CREATE TABLE IF NOT EXISTS SCENES("\
                    "ID				serial			primary key,"\
                    "FILENAME		text			not null,"\
                    "FILEPATH		text			not null,"\
                    "TITLE			text,"\
                    "COMPANY		text,"\
                    "SERIES			text,"\
                    "SCENE_NO		integer,"\
                    "RATING			text,"\
                    "SIZE			float8,"\
                    "LENGTH			float8,"\
                    "WIDTH			integer,"\
                    "HEIGHT			integer,"\
                    "ADDED			date,"\
                    "CREATED		date,"\
                    "ACCESSED		date,"\
                    "ACTOR1			text,"\
                    "AGE1			integer,"\
                    "ACTOR2			text,"\
                    "AGE2			integer,"\
                    "ACTOR3			text,"\
                    "AGE3			integer,"\
                    "ACTOR4			text,"\
                    "AGE4			integer,"\
                    "URL			text,"\
                    "TAGS			text)"
// Store entries from online filmographies
#define FDB_TABLE	"create table if not exists filmographies("\
                    "ID				serial			primary key,"\
                    "actor			text			not null,"\
                    "title			text			not null,"\
                    "company		text			not null,"\
                    "year			integer			not null,"\
                    "scene			integer,"\
                    "tags			text)"
#define THUMBNAIL_DB	"create table if not exists thumbnails("\
                    "ID 			serial 			primary key,"\
                    "SCENEID 		integer			not null,"\
                    "FILENAME		text			not null,"\
                    "DATEADDED		text,"\
                    "SCENEPATH		text,"\
                    "SCENEFILE 		text,"\
                    "IMAGE			OID)"
#define HEADSHOT_DB	"create table if not exists thumbnails("\
                    "ID 			serial 		primary key,"\
                    "FILENAME		text			not null,"\
                    "DATEADDED		text,"\
                    "NAME 			text,"\
                    "IMAGE			bytes)"

#endif // SQL_DEFINITIONS_H
