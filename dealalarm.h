#ifndef DEALALARM_H
#define DEALALARM_H

#include <QDialog>
#include "STRUCT.h"
namespace Ui {
    class CDEALALARM;
}

class CDEALALARM : public QDialog
{
    Q_OBJECT

public:
    explicit CDEALALARM(QWidget *parent = 0);
    ~CDEALALARM();
    void _Update(SAlarmDevice data);
private slots:
    void on_Qrbtn1_clicked();

    void on_Qrbtn2_clicked();

    void on_QpbtnDeal_clicked();

    void on_QpbtnClose_clicked();
signals :
    void SigDealAlarm(int m_nAlarmType,QString strStime,QString strAlarmStatu,QString strDealHuman,QString strMessage);
private:
    Ui::CDEALALARM *ui;
    QString strStime;
    int m_nAlarmType;
};

#endif // DEALALARM_H
