#ifndef CARD_H
#define CARD_H

#include <QDialog>

namespace Ui {
    class CCARD;
}

class CCARD : public QDialog
{
    Q_OBJECT

public:
    explicit CCARD(QWidget *parent = 0);
    ~CCARD();
    void _Update(int nFlag,QString strCrad,QString strAddr,QString strTime);
private slots:
    void on_QpbtnIsOk_clicked();
    void on_QpbtnClose_clicked();
    void on_QcheckBox1_clicked(bool checked);
    void on_QcheckBox2_clicked(bool checked);
    void on_QcheckBox3_clicked(bool checked);

signals:
    void SigUpdateTime(QString strCardNum);
    void SigRefurbish(QString strCardNum);
private:
    Ui::CCARD *ui;
    QString m_strCard,m_strAddr,m_strTime;
    int m_nFlag;
    void init();
    QList<QString> T;
    int nLimits[4];
    int m_nLimits;
};

#endif // CARD_H
