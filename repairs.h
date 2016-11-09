#ifndef REPAIRS_H
#define REPAIRS_H

#include <QDialog>

namespace Ui {
    class CREPAIRS;
}

class CREPAIRS : public QDialog
{
    Q_OBJECT

public:
    explicit CREPAIRS(QWidget *parent = 0);
    ~CREPAIRS();
    void _Update(QString strRepairs,int nFlag);
private slots:
    void on_QpbtnOk_clicked();

    void on_QpbtnClose_clicked();
signals :
    void SigRepairs();
private:
    Ui::CREPAIRS *ui;
    QString m_strRepairs;
    int m_nFlag;
};

#endif // REPAIRS_H
