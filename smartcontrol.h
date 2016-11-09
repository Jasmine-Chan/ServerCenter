#ifndef SMARTCONTROL_H
#define SMARTCONTROL_H

#include <QDialog>

namespace Ui {
    class CSMARTCONTROL;
}

class CSMARTCONTROL : public QDialog
{
    Q_OBJECT

public:
    explicit CSMARTCONTROL(QWidget *parent = 0);
    ~CSMARTCONTROL();
    void _Updata(QString strIp,QString strAddr,QString strType);
private slots:
    void on_Qpush1_clicked();

    void on_Qpush2_clicked();

    void on_Qpush3_clicked();

    void on_Qpush4_clicked();
    void on_Qpush5_clicked();

    void on_Qpush6_clicked();

signals :
    void SigSendLamp(char *buf,QString strIp);

private:
    Ui::CSMARTCONTROL *ui;
    int m_flag[8];
    QString m_strIp,m_strAddr;
    char *m_pAddr;
    int m_nFlag;

private:
    void _Sendto(int flag);
};

#endif // SMARTCONTROL_H
