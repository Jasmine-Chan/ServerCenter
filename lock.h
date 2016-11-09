#ifndef LOCK_H
#define LOCK_H

#include <QDialog>

namespace Ui {
    class CLOCK;
}

class CLOCK : public QDialog
{
    Q_OBJECT

public:
    explicit CLOCK(QWidget *parent = 0);
    ~CLOCK();
    void _Update(QString strUser,QString strName);
private slots:
    void on_QpbtnDeblock_clicked();

private:
    Ui::CLOCK *ui;
    QString m_strUser,m_strName;
signals:
    void SigUnLock();
};

#endif // LOCK_H
