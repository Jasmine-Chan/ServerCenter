#include "lock.h"
#include "ui_lock.h"
#include "mysql.h"
#include <QMessageBox>
#include <QPainter>
CLOCK::CLOCK(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CLOCK)
{
    ui->setupUi(this);
    setWindowTitle(tr("系统锁定"));
    setWindowModality(Qt::ApplicationModal);
    ui->QlinePass->setMaxLength(6);
    setWindowFlags(Qt::FramelessWindowHint|Qt::Tool );
    ui->QlinePass->setEchoMode(QLineEdit::Password);
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

CLOCK::~CLOCK()
{
    delete ui;
}
void CLOCK::_Update(QString strUser,QString strName)
{
    m_strUser.clear();
    m_strName.clear();
    m_strUser = strUser;
    m_strName = strName;
    ui->QlinePass->clear();
    ui->QlinePass->setFocus();
}

void CLOCK::on_QpbtnDeblock_clicked()
{
    CMYSQL CMySql;
    if(CMySql._Deblock(m_strUser,ui->QlinePass->text()))
    {
        CMySql._InsertSystem(4,m_strUser,m_strName);
        emit SigUnLock();
        close();
    }else{
        QMessageBox::information(this,tr("提示"),tr("密码错误"));
    }
}
