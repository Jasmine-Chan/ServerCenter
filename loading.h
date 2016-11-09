#ifndef LOADING_H
#define LOADING_H

#include <QDialog>
#include <QPixmap>
#include <QDateTime>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QPainter>
#include "servercenter.h"
namespace Ui {
    class CLOADING;
}

class CLOADING : public QDialog
{
    Q_OBJECT
public:
    explicit CLOADING(QWidget *parent = 0);
    ~CLOADING();
    void _Update();
    void closeEvent( QCloseEvent * event );
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    QList<QString> T;

private slots:
    void on_QpbtnLoad_clicked();
    void on_QpbtnClose_clicked();
    void on_QtbtnMin_clicked();
    void on_QtbtnClose_clicked();
    void time();

public slots:
    void SlotLoading();
    void lookedUp(const QHostInfo &host);

signals :
    void SigQuit();

private:
    Ui::CLOADING *ui;
    CSERVERCENTER *ServerCenter;
    int m_nFlag;
    QPoint pos;
    QMouseEvent *event;
    int protocol;

private:
    void _Interface();
};

#endif // LOADING_H
