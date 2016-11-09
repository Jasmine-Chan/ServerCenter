#ifndef PIC_H
#define PIC_H

#include <QWidget>
#include <QDebug>

namespace Ui {
    class CPIC;
}

class CPIC : public QWidget
{
    Q_OBJECT

public:
    explicit CPIC(QWidget *parent = 0);
    ~CPIC();
    void _Update(QString strPath);

private slots:
    void on_QpbtnClose_clicked();

private:
    Ui::CPIC *ui;
};

#endif // PIC_H
