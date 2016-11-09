#include "pic.h"
#include "ui_pic.h"

CPIC::CPIC(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CPIC)
{
    ui->setupUi(this);
    setWindowTitle(tr("系统锁定"));
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::FramelessWindowHint|Qt::Tool );
}

CPIC::~CPIC()
{
    delete ui;
}

void CPIC::_Update(QString strPath)
{
    QPalette palette;
    QPixmap pixmap(strPath);
    palette.setBrush(QPalette::Window, QBrush(pixmap));
    this->setPalette(palette);
}

void CPIC::on_QpbtnClose_clicked()
{
    close();
}
