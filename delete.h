#ifndef DELETE_H
#define DELETE_H

#include <QDialog>

namespace Ui {
    class CDELETE;
}

class CDELETE : public QDialog
{
    Q_OBJECT

public:
    explicit CDELETE(QWidget *parent = 0);
    ~CDELETE();
    void _Update(int nPage);
private slots:
    void on_QpbtnDelete_clicked();

    void on_QpbtnClose_clicked();
signals:
    void SigDelete();
private:
    Ui::CDELETE *ui;
    int m_nPage;
};

#endif // DELETE_H
