#ifndef FIND_H
#define FIND_H

#include <QDialog>

namespace Ui {
    class CFIND;
}

class CFIND : public QDialog
{
    Q_OBJECT

public:
    explicit CFIND(QWidget *parent = 0);
    ~CFIND();
    void _Update(int nPage);
private slots:
    void on_QpbtnOk_clicked();

    void on_QpbtnClose_clicked();

    void on_QchkFind_activated(int index);

    void on_QrbtnTime_clicked();
signals:
    void SigFind(int nIndex,int nTime,QString strFind,QString strStime,QString strEtime);
private:
    Ui::CFIND *ui;
    int m_nPage,m_nIndex,m_nTime;
};

#endif // FIND_H
