#include "connectionpoolmssql.h"
#include <QDebug>

QMutex CCONNECTIONPOOLMSSQL::mutex;
QWaitCondition CCONNECTIONPOOLMSSQL::waitConnection;
CCONNECTIONPOOLMSSQL* CCONNECTIONPOOLMSSQL::instance = NULL;
bool CCONNECTIONPOOLMSSQL::IsNetWork = false;       //判断数据是否正常连接

CCONNECTIONPOOLMSSQL::CCONNECTIONPOOLMSSQL()
{
    SetConfig();
    testOnBorrow = true;
    testOnBorrowSql = "SELECT 1";

    maxWaitTime  = 1000;
    waitInterval = 200;
    maxConnectionCount  = 20;
}
void CCONNECTIONPOOLMSSQL::SetConfig()
{
    QString path;
    QDir dir;
    path = dir.currentPath() + "/DBSet.ini";
    databaseType = "QODBC";

    QString strName,strIp;
    char buf[20] = {0};
    ::GetPrivateProfileStringA("MsSql","IP","localhost",buf,20,path.toAscii().data());
    strIp     = QString(buf);

    memset(buf,0,20);
    ::GetPrivateProfileStringA("MsSql","Name","sh",buf,20,path.toAscii().data());
    strName = QString(buf);

    memset(buf,0,20);
    databaseName = "DRIVER={SQL SERVER};SERVER="+strIp+";DATABASE="+strName+"";
    ::GetPrivateProfileStringA("MsSql","User","root",buf,20,path.toAscii().data());
    username     = QString(buf);

    memset(buf,0,20);
    ::GetPrivateProfileStringA("MsSql","Pass","anenda",buf,20,path.toAscii().data());
    password     = QString(buf);
}

CCONNECTIONPOOLMSSQL::~CCONNECTIONPOOLMSSQL()
{
    // 销毁连接池的时候删除所有的连接
    foreach(QString connectionName, usedConnectionNames)
    {
        QSqlDatabase::removeDatabase(connectionName);
    }

    foreach(QString connectionName, unusedConnectionNames)
    {
        QSqlDatabase::removeDatabase(connectionName);
    }
}

CCONNECTIONPOOLMSSQL& CCONNECTIONPOOLMSSQL::getInstance()
{
    if (NULL == instance)
    {
        QMutexLocker locker(&mutex);

        if (NULL == instance)
        {
            instance = new CCONNECTIONPOOLMSSQL();
        }
    }

    return *instance;
}

void CCONNECTIONPOOLMSSQL::release()
{
    QMutexLocker locker(&mutex);
    delete instance;
    instance = NULL;
}

QSqlDatabase CCONNECTIONPOOLMSSQL::openConnection()
{
    CCONNECTIONPOOLMSSQL& pool = CCONNECTIONPOOLMSSQL::getInstance();
    QString connectionName;

    QMutexLocker locker(&mutex);

    // 已创建连接数
    int connectionCount = pool.unusedConnectionNames.size() + pool.usedConnectionNames.size();

    // 如果连接已经用完，等待 waitInterval 毫秒看看是否有可用连接，最长等待 maxWaitTime 毫秒
    for (int i = 0;
         i < pool.maxWaitTime
         && pool.unusedConnectionNames.size() == 0 && connectionCount == pool.maxConnectionCount;
         i += pool.waitInterval)
    {
        waitConnection.wait(&mutex, pool.waitInterval);

        // 重新计算已创建连接数
        connectionCount = pool.unusedConnectionNames.size() + pool.usedConnectionNames.size();
    }

    if (pool.unusedConnectionNames.size() > 0)
    {
        // 有已经回收的连接，复用它们
        connectionName = pool.unusedConnectionNames.dequeue();
    }
    else if (connectionCount < pool.maxConnectionCount)
    {
        // 没有已经回收的连接，但是没有达到最大连接数，则创建新的连接
        connectionName = QString("Connection-%1").arg(connectionCount + 100);
    }
    else
    {
        IsNetWork = true;
        // 已经达到最大连接数
        return QSqlDatabase();
    }

    // 创建连接
    QSqlDatabase db = pool.createConnection(connectionName);

    // 有效的连接才放入 usedConnectionNames
//    qDebug()<<"4:"<<QDateTime::currentDateTime().time().second()<<QDateTime::currentDateTime().time().msec();
    if (db.isOpen())
    {
        IsNetWork = true;
//        qDebug()<<"5:"<<QDateTime::currentDateTime().time().second()<<QDateTime::currentDateTime().time().msec();
        pool.usedConnectionNames.enqueue(connectionName);
    }
    else
    {
        IsNetWork = false;
    }

    return db;
}

void CCONNECTIONPOOLMSSQL::closeConnection(QSqlDatabase connection)
{
    CCONNECTIONPOOLMSSQL& pool = CCONNECTIONPOOLMSSQL::getInstance();
    QString connectionName = connection.connectionName();

    // 如果是我们创建的连接，从 used 里删除，放入 unused 里
    if (pool.usedConnectionNames.contains(connectionName))
    {
        QMutexLocker locker(&mutex);
        pool.usedConnectionNames.removeOne(connectionName);
        pool.unusedConnectionNames.enqueue(connectionName);
        waitConnection.wakeOne();
    }
}

QSqlDatabase CCONNECTIONPOOLMSSQL::createConnection(const QString &connectionName)
{
    // 连接已经创建过了，复用它，而不是重新创建
    if (QSqlDatabase::contains(connectionName))
    {
        QSqlDatabase db1 = QSqlDatabase::database(connectionName);

//        qDebug()<<"1:"<<QDateTime::currentDateTime().time().second()<<QDateTime::currentDateTime().time().msec();
//        if (testOnBorrow)
        if(db1.open())
        {
            IsNetWork = true;
//            qDebug()<<"2:"<<QDateTime::currentDateTime().time().second()<<QDateTime::currentDateTime().time().msec();
            // 返回连接前访问数据库，如果连接断开，重新建立连接
//            qDebug() << "Test MSSQL connection on borrow, execute:" << testOnBorrowSql << ", for" << connectionName;
            QSqlQuery query(testOnBorrowSql, db1);
//            qDebug() << "db1.open()"<<db1.open()<<(query.lastError().type());
            if (query.lastError().type() != QSqlError::NoError && !db1.open())
            {
                IsNetWork = false;
//                qDebug()<<"3:"<<QDateTime::currentDateTime().time().second()<<QDateTime::currentDateTime().time().msec();
//                qDebug() << " MSSQL Open datatabase error:" << db1.lastError().text();
                return QSqlDatabase();
            }
        }
        else
        {
            IsNetWork = false;
            return QSqlDatabase();
        }
        return db1;

    }
    // 创建一个新的连接
    QSqlDatabase db = QSqlDatabase::addDatabase(databaseType, connectionName);
    db.setDatabaseName(databaseName);
    db.setUserName(username);
    db.setPassword(password);
    db.setConnectOptions("SQL_ATTR_ACCESS_MODE=SQL_MODE_READ_ONLY;SQL_ATTR_TRACE=SQL_OPT_TRACE_ON");
    if (!db.open())
    {
        IsNetWork = false;
        qDebug() << "Open datatabase error:" << db.lastError().text();
        return QSqlDatabase();
    }

    return db;
}
