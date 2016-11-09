#ifndef UPDATE_H
#define UPDATE_H

#include <QDialog>
#include <QTimer>

namespace Ui {
    class CUPDATE;
}

class CUPDATE : public QDialog
{
    Q_OBJECT

public:
    explicit CUPDATE(QWidget *parent = 0);
    ~CUPDATE();
    void _Updata(QString strIp,QString strAddr,QString strType);
    void _ReadUpdateA8(unsigned char *buf, QString strIp);
    void _ReadUpdate(unsigned char *buf,QString strIp);
    void _UpdateAddr();

private slots:
    void on_QpbtnRead_clicked();

    void on_QpbtnWrite_clicked();
    void timerEvent( QTimerEvent *event );

private:
    Ui::CUPDATE *ui;
    QString m_strIp,m_strAddr;
    int m_nFlag;
    char *m_pAddr;
    int m_nTimerId,m_nTimerId1;

signals :
    void SigRead(char *buf,QString strIp);
    void SigWrite(char *buf,QString strIp);
};

#endif // UPDATE_H
