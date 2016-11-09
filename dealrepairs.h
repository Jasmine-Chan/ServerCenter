#ifndef DEALREPAIRS_H
#define DEALREPAIRS_H

#include <QDialog>

namespace Ui {
    class CDEALREPAIRS;
}

class CDEALREPAIRS : public QDialog
{
    Q_OBJECT

public:
    explicit CDEALREPAIRS(QWidget *parent = 0);
    ~CDEALREPAIRS();
    void _Update(QString strAddr,QString strType,QString strStime,QString strEtime);
private slots:
    void on_QpbtnDeal_clicked();

    void on_QpbtnClose_clicked();
signals:
    void SigDealRepairs();
private:
    Ui::CDEALREPAIRS *ui;
    QString m_strAddr,m_strStime;
};

#endif // DEALREPAIRS_H
