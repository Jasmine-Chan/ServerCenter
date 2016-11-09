#ifndef USER_H
#define USER_H

#include <QDialog>

namespace Ui {
    class CUSER;
}

class CUSER : public QDialog
{
    Q_OBJECT

public:
    explicit CUSER(QWidget *parent = 0);
    ~CUSER();
    void _Update(QString strName,QString strUser,int nFlag,int nGrade);
private slots:
    void on_QpbtnOk_clicked();

    void on_QpbtnClose_clicked();
    void on_QcomGrade_activated(int index);

signals:
    void SigUpdateUser();
private:
    Ui::CUSER *ui;
    QString m_strName,m_strUser;
    int m_nFlag,m_nGrade;
};

#endif // USER_H
