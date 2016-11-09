#ifndef FINDDEVICE_H
#define FINDDEVICE_H

#include <QDialog>

namespace Ui {
    class CFINDDEVICE;
}

class CFINDDEVICE : public QDialog
{
    Q_OBJECT

public:
    explicit CFINDDEVICE(QWidget *parent = 0);
    ~CFINDDEVICE();
    void _Update(int nPage);
private slots:
    void on_QpbtnFind_clicked();

    void on_QpbtnClose_clicked();

    void on_QchkFind_activated(int index);
signals:
    void SigDeviceFind(int nIndex,QString strFind);
private:
    Ui::CFINDDEVICE *ui;
    int m_nIndex;
};

#endif // FINFDEVICE_H
