#ifndef FINDCARD_H
#define FINDCARD_H

#include <QDialog>

namespace Ui {
    class CFINDCARD;
}

class CFINDCARD : public QDialog
{
    Q_OBJECT

public:
    explicit CFINDCARD(QWidget *parent = 0);
    ~CFINDCARD();
    void _Update();
private slots:
   void on_QpbtnFind_clicked();
   void on_QchkIcCard_clicked(bool checked);
   void on_QchkTAddr_clicked(bool checked);
   void on_QchkMAddr_clicked(bool checked);
   void on_QpbtnClose_clicked();

   void on_QchkValidity_clicked(bool checked);

private:
    Ui::CFINDCARD *ui;
    int m_nFlag;
signals :
    void SigFindCard(int nFlag,QString strCard,QString strTAddr,QString strMAddr,QString strTime);
};

#endif // FINDCARD_H
