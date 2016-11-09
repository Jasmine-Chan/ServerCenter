#include "about.h"
#include "ui_about.h"
#include <QPainter>
#include <QBitmap>
CABOUT::CABOUT(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CABOUT)
{
    ui->setupUi(this);
    setWindowTitle(tr("管理软件1.0"));
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::FramelessWindowHint);
    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setRenderHint(QPainter::Antialiasing);
    int arcR = 10;
    QRect rect = this->rect();
    QPainterPath path;
    //逆时针
    path.moveTo(arcR, 0);
    path.arcTo(0, 0, arcR * 2, arcR * 2, 90.0f, 90.0f);

    path.lineTo(0, rect.height()-arcR);
    path.arcTo(0, rect.height()-arcR*2, arcR * 2, arcR * 2, 180.0f, 90.0f);

    path.lineTo(rect.width()-arcR, rect.height());
    path.arcTo(rect.width()-arcR*2, rect.height()-arcR*2, arcR * 2, arcR * 2, 270.0f, 90.0f);

    path.lineTo(rect.width(), arcR);
    path.arcTo(rect.width() - arcR * 2, 0, arcR * 2, arcR * 2, 0.0f, 90.0f);
    path.lineTo(arcR, 0);

    p.drawPath(path);
    p.fillPath(path, QBrush(Qt::red)); //arm和windows平台没有这行代码将显示一个透明的空空的框
    setMask(bmp);
}

CABOUT::~CABOUT()
{
    delete ui;
}

void CABOUT::on_pushButton_clicked()
{
    close();
}
