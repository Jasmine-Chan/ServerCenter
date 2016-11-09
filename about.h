#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui {
    class CABOUT;
}

class CABOUT : public QDialog
{
    Q_OBJECT

public:
    explicit CABOUT(QWidget *parent = 0);
    ~CABOUT();

private slots:
    void on_pushButton_clicked();

private:
    Ui::CABOUT *ui;
};

#endif // ABOUT_H
