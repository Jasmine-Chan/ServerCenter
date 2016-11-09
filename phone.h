#ifndef PHONE_H
#define PHONE_H

#include <QDialog>
#include "mysql.h"
namespace Ui {
    class CPHONE;
}

class CPHONE : public QDialog
{
    Q_OBJECT

public:
    explicit CPHONE(QWidget *parent = 0);
    ~CPHONE();
    void _Update(QString strDepartment,QString strHuman,QString strPhone,int nFlag);
private slots:
    void on_QpbtnOk_clicked();

    void on_QpbtnClose_clicked();
signals :
    void SigPhone();
private:
    Ui::CPHONE *ui;
    int m_nFlag;
    QString m_strDepartment,m_strHuman,m_strPhone;
};

#endif // PHONE_H
