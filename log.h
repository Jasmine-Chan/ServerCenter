#ifndef LOG_H
#define LOG_H
#include <QFile>
#include <QMutex>
int _Write(QString strMessage);
class CLOG
{
public:
    CLOG();
    void _Remove();
private:
    static QMutex mutex;
};

#endif // LOG_H
