#ifndef SETODBC_H
#define SETODBC_H

#include <QDialog>

namespace Ui {
    class CSETODBC;
}

class CSETODBC : public QDialog
{
    Q_OBJECT

public:
    explicit CSETODBC(QWidget *parent = 0);
    ~CSETODBC();
    void closeEvent( QCloseEvent * event );
    void _Update();
private slots:
    void on_QpbtnOK_clicked();

    void on_QpbtnClose_clicked();
signals :
    void SigQuit();
private:
    Ui::CSETODBC *ui;
};

#endif // SETODBC_H
