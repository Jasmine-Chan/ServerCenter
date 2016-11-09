#include "log.h"
#include <QDebug>
#include <QDateTime>
CLOG::CLOG()
{
}
void CLOG::_Remove()
{
    QFile file1("LogMessage.txt");
    file1.remove();
    QFile file2("Log.txt");
    file2.remove();
}

int _Write(QString strMessage)
{
    QDateTime NowTime = QDateTime::currentDateTime();
    strMessage = NowTime.toString("yyyy-MM-dd hh:mm:ss") + strMessage;
    QFile file1("LogMessage.txt");
    if(!file1.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
           return 0;
    QTextStream out(&file1);
    out <<strMessage << "\n";
    file1.close();
    return 1;
}
