#include <QtGui/QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <taesclass.h>
#include <QTextStream>
#include <QTextCodec>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include "connectionpoolmssql.h"
#include "connectionpoolmysql.h"
#include "loading.h"
#include "setodbc.h"
//#include "STRUCT.h"
//#include "mysql.h"
#include "log.h"

int decode()
{
    QString dataPassword = "TmtXQlJFSnhOVVp4T3tCMk5VRnhOVkNCTkVCeU5FQnlORUIxTkVCeVJGRXF2NVNCUkVHQlJFQnlO\r\nQj4+QU5FTkRB";
    QByteArray tempArray = QByteArray::fromBase64(dataPassword.toUtf8());
    QString str = QString(tempArray);
    qDebug()<<str;
    str = str.left(str.length()  - 6);
    QByteArray byte = str.toUtf8();
    char buf[1024] = {0};
    memcpy(buf,byte.data(),byte.size());
    for(int i = 0 ;i < byte.size();i++)
    {

       int t = buf[i];
       buf[i] = t - 1;
    }
    str = QString(buf);
    QByteArray tempArray1 = QByteArray::fromBase64(str.toUtf8());
    qDebug()<<QString::fromUtf8(tempArray1);
}

void customMessageHandler(QtMsgType type, const char *msg)
{
    static QMutex mutex;
    mutex.lock();
    QString txt;
    switch (type)
    {
        case QtDebugMsg:     //调试信息提示
           txt = QString("Debug: %1").arg(msg);
           break;
        case QtWarningMsg:  //一般的warning提示
           txt = QString("Warning: %1").arg(msg);
           break;
        case QtCriticalMsg:    //严重错误提示
           txt = QString("Critical: %1").arg(msg);
           break;
        case QtFatalMsg:      //致命错误提示
           txt = QString("Fatal: %1").arg(msg);
           abort();
        default:
           break;
    }
    QFile outFile("Log.txt");
//    QTextCodec *code1=QTextCodec::codecForName("utf-8");
//    QTextCodec::setCodecForTr(QTextCodec::codecForName("System"));
    outFile.open(QIODevice::WriteOnly|QIODevice::Text | QIODevice::Append);
    QTextStream ts(&outFile);
//    ts.setCodec("System");
    QDateTime NowTime = QDateTime::currentDateTime();
    txt = NowTime.toString("yyyy-MM-dd hh:mm:ss") + "--" + txt;
    ts << txt << "\n";  //QObject::tr(txt.toAscii())
    outFile.close();
    mutex.unlock();
}

//注册时间检测
int _TimeValidity()
{
    char buf[12] = {0};
    ::GetPrivateProfileStringA("Flag","STime","sa",buf,12,"C:\\WINDOWS\\UTSet.ini");
    QDate Date = QDate::fromString(QString(buf),"yyyy-MM-dd");
    if(Date.isNull())
    {
        QString s = QString::number(1);
        QByteArray byte = s.toUtf8();
        ::WritePrivateProfileStringA("Flag", "idex", byte.data(), "C:\\WINDOWS\\UtSet.ini");
        s = QString::number(0);
        byte.clear();
        byte = s.toUtf8();
        ::WritePrivateProfileStringA("Flag", "month", byte.data(), "C:\\WINDOWS\\UtSet.ini");
        ::WritePrivateProfileStringA("1", "AllDay", byte.data(), "C:\\WINDOWS\\UtSet.ini");
        QString strTime =  QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
        byte.clear();
        byte = strTime.toUtf8();
        ::WritePrivateProfileStringA("Flag", "STime", byte.data(), "C:\\WINDOWS\\UtSet.ini");
        return 1;
    }
    ::GetPrivateProfileStringA("Flag","STime","sa",buf,12,"C:\\WINDOWS\\UtSet.ini");
    Date = QDate::fromString(QString(buf),"yyyy-MM-dd");
    QDate NowDate = QDateTime::currentDateTime().date();
    int AllDay = Date.daysTo(NowDate);
    int nFlag  = ::GetPrivateProfileIntA("Flag","idex",0,"C:\\WINDOWS\\UtSet.ini");
    int nMonth = ::GetPrivateProfileIntA("Flag","month",0,"C:\\WINDOWS\\UtSet.ini");
    qDebug()<<AllDay<<nFlag<<nMonth<<NowDate<<Date;
    if(nFlag == 0)
    {
        QMessageBox::information(NULL,"提示1","软件维护，请联系厂家\n安恩达科技有限公司");
        return 0;
    }
    else if(nFlag == 1)
    {
        if((AllDay < (int)(::GetPrivateProfileIntA("1","AllDay",0,"C:\\WINDOWS\\UTSet.ini")))||(AllDay > (80 +30*nMonth)))
        {
            QMessageBox::information(NULL,"提示2","软件维护，请联系厂家\n安恩达科技有限公司");
            return 0;
        }
        else
        {
            ::WritePrivateProfileStringA("1", "AllDay", QString::number(AllDay).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
        }
    }
    else if(nFlag == 2)
    {
        if((AllDay < (int)(::GetPrivateProfileIntA("2","AllDay",0,"C:\\WINDOWS\\UTSet.ini")))||(AllDay > (900+30*nMonth)))
        {
            QMessageBox::information(NULL,"提示3","软件维护，请联系厂家\n安恩达科技有限公司");
            return 0;
        }
        else
        {
            ::WritePrivateProfileStringA("2", "AllDay", QString::number(AllDay).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
        }
    }
    return 1;
}

QString getIP()  //获取本机ip地址
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
       if(address.protocol() == QAbstractSocket::IPv4Protocol)
            return address.toString();
    }
    return 0;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
    CMYSQL MySql;
    CLOADING Loading;
    CLOG Log;
    CSETODBC SetOdbc;
    Log._Remove();
    //DEFINES += QT_NO_DEBUG_OUTPUT

//    qInstallMsgHandler(customMessageHandler);
    decode();
//    qDebug()<<"1111111";
    QString strIP = getIP();
    qDebug()<<"IP:"<<strIP;
//    if(_TimeValidity() == 0)        //注册时间检测
//        return 0;
    QFile file(":/button.qss");
    file.open(QFile::ReadOnly);
    QString strStyle = QLatin1String(file.readAll());
    a.setStyleSheet(strStyle);
    if(MySql._IsOpen())
    {
        if(MySql._OpenODBC())
        {
            Loading._Update();
            Loading.move ((QApplication::desktop()->width() - Loading.width())/2,(QApplication::desktop()->height() - Loading.height())/2);
            Loading.show();
        }
        else
        {
            QMessageBox::information(NULL,"提示","远程数据库连接失败无法启动!");
            CCONNECTIONPOOLMSSQL::release();
        }
    }
    else
    {
        QMessageBox::information(NULL,"提示","数据库连接失败无法启动,请配置数据库后重启!");
        CCONNECTIONPOOLMYSQL::release();
        SetOdbc._Update();
        SetOdbc.move ((QApplication::desktop()->width() - SetOdbc.width())/2,(QApplication::desktop()->height() - SetOdbc.height())/2);
        SetOdbc.show();
    }
    a.setQuitOnLastWindowClosed(false);
    CCONNECTIONPOOLMSSQL::release();
    CCONNECTIONPOOLMYSQL::release();
    return a.exec();
}
