#ifndef SYNCMSSQL_H
#define SYNCMSSQL_H

#include <QThread>
#include "connectionpoolmssql.h"
#include "connectionpoolmysql.h"
#include "log.h"
#include <QMutex>
#include "list.h"

class CSYNCMSSQLMIDDLE : public QThread
{
    Q_OBJECT
public:
    explicit CSYNCMSSQLMIDDLE(QObject *parent = 0);
    ~CSYNCMSSQLMIDDLE();
protected:
    void run();
signals:

public slots:
private:
    QMutex mutex;
};


class CSYNCMSSQLTENEMENT : public QThread
{
    Q_OBJECT
public:
    explicit CSYNCMSSQLTENEMENT(QObject *parent = 0);
    ~CSYNCMSSQLTENEMENT();
protected:
    void run();
signals:

public slots:
private:
    QMutex mutex;

};

class CSYNCMSSQLALARMRECORD : public QThread
{
    Q_OBJECT
public:
    explicit CSYNCMSSQLALARMRECORD(QObject *parent = 0);
    ~CSYNCMSSQLALARMRECORD();
protected:
    void run();
signals:

public slots:
private:
    QMutex mutex;
};

class CSYNCMSSQLREPAIRSRECORD : public QThread
{
    Q_OBJECT
public:
    explicit CSYNCMSSQLREPAIRSRECORD(QObject *parent = 0);
    ~CSYNCMSSQLREPAIRSRECORD();
protected:
    void run();
signals:

public slots:
private:
    QMutex mutex;
};


class CSYNCMSSQLCARDRECORD : public QThread
{
    Q_OBJECT
public:
    explicit CSYNCMSSQLCARDRECORD(QObject *parent = 0);
    ~CSYNCMSSQLCARDRECORD();
protected:
    void run();
signals:

public slots:
private:
    QMutex mutex;
};

class CSYNCMSSQLICCARD : public QThread
{
    Q_OBJECT
public:
    explicit CSYNCMSSQLICCARD(QObject *parent = 0);
    ~CSYNCMSSQLICCARD();
protected:
    void run();
signals:

public slots:
private:
    QMutex mutex;
};


class CSYNCMSSQLDELETECARD : public QThread
{
    Q_OBJECT
public:
    explicit CSYNCMSSQLDELETECARD(QObject *parent = 0);
    ~CSYNCMSSQLDELETECARD();
protected:
    void run();
signals:

public slots:
private:
    QMutex mutex;
};


class CSYNCMSSQLDELETEDEVICE : public QThread
{
    Q_OBJECT
public:
    explicit CSYNCMSSQLDELETEDEVICE(QObject *parent = 0);
    ~CSYNCMSSQLDELETEDEVICE();
protected:
    void run();
signals:

public slots:
private:
    QMutex mutex;
};
#endif // SYNCMSSQL_H
