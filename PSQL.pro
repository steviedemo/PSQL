QT += core
QT -= gui

CONFIG += c++11

TARGET = PSQL
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
OBJECTS_DIR = build

QMAKE_MACOSX_DEPLOYMENT_TARGET=10.9#-mmacosx-version-min
QMAKE_CXXFLAGS =        -std=c++0x
QMAKE_CFLAGS_DEBUG =    -std=gnu99
QMAKE_CFLAGS_RELEASE =  -std=gnu99
QMAKE_LFLAGS += -F/System/Library/Frameworks -L/usr/lib

INCLUDEPATH += \
    src	\
    example

LIBS += \
    -L/usr/local/opt/qt/lib

SOURCES += \
    example/main.cpp \
    src/qsqldbhelper.cpp \
    src/query.cpp \
    src/sql.cpp \
    src/sqlconnection.cpp

HEADERS += \
    src/qsqldbhelper.h \
    src/query.h \
    src/sql.h \
    src/sqlconnection.h
