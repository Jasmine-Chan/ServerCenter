#include "setodbc.h"
#include "ui_setodbc.h"
#include <QMessageBox>
#include "log.h"
#include <QtGui/QApplication>
#include "loading.h"
CSETODBC::CSETODBC(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSETODBC)
{
    ui->setupUi(this);
    ui->QlinePass->setEchoMode(QLineEdit::Password);
    ui->QlinePass1->setEchoMode(QLineEdit::Password);
    connect(this,SIGNAL(SigQuit()), qApp, SLOT(quit()));
//    ui->tabWidget->setStyleSheet("QTabWidget:pane {border-top:1px solid #e8f3f9;background:  transparent; } ");
}

CSETODBC::~CSETODBC()
{
    delete ui;
}

void CSETODBC::_Update()
{
    QString path;
    QDir dir;
    path = dir.currentPath() + "/DBSet.ini";
    char buf[20] = {0};
    int nFlag = ::GetPrivateProfileIntA("Flag","idex",0,path.toAscii().data());//没有Flag字段
    ui->comboBox->setCurrentIndex(nFlag);

    ::GetPrivateProfileStringA("MySql","Name","sa",buf,20,path.toAscii().data());
    ui->QlineName->setText(QString(buf));
    memset(buf,0,20);

    ::GetPrivateProfileStringA("MySql","User","sa",buf,20,path.toAscii().data());
    ui->QlineUser->setText(QString(buf));
    memset(buf,0,20);

    ::GetPrivateProfileStringA("MySql","Pass","sa",buf,20,path.toAscii().data());
    ui->QlinePass->setText(QString(buf));
    memset(buf,0,20);

    ::GetPrivateProfileStringA("MySql","IP","sa",buf,20,path.toAscii().data());
    ui->QlineIP->setText(QString(buf));
    memset(buf,0,20);

    ::GetPrivateProfileStringA("MsSql","Name","sa",buf,20,path.toAscii().data());
    ui->QlineName1->setText(QString(buf));
    memset(buf,0,20);

    ::GetPrivateProfileStringA("MsSql","User","sa",buf,20,path.toAscii().data());
    ui->QlineUser1->setText(QString(buf));
    memset(buf,0,20);

    ::GetPrivateProfileStringA("MsSql","Pass","sa",buf,20,path.toAscii().data());
    ui->QlinePass1->setText(QString(buf));
    memset(buf,0,20);

    ::GetPrivateProfileStringA("MsSql","IP","sa",buf,20,path.toAscii().data());
    ui->QlineIP1->setText(QString(buf));
    memset(buf,0,20);
}

void CSETODBC::on_QpbtnOK_clicked()
{
    QString path;
    QDir dir;
    path = dir.currentPath() + "/DBSet.ini";
    int t = ui->comboBox->currentIndex();
    QString ss = QString::number(t);
    QByteArray byte1 = ss.toUtf8();
    ::WritePrivateProfileStringA("MySql", "idex", byte1.data(), path.toAscii().data());

    QString s = ui->QlineName->text();
    QByteArray byte = s.toUtf8();
    ::WritePrivateProfileStringA("MySql", "Name", byte.data(), path.toAscii().data());

    s.clear();
    byte.clear();
    s = ui->QlineUser->text();
    byte = s.toUtf8();
    ::WritePrivateProfileStringA("MySql", "User", byte.data(), path.toAscii().data());

    s.clear();
    byte.clear();
    s = ui->QlineIP->text();
    byte = s.toUtf8();
    ::WritePrivateProfileStringA("MySql", "IP", byte.data(), path.toAscii().data());

    s.clear();
    byte.clear();
    s = ui->QlinePass->text();
    byte = s.toUtf8();
    ::WritePrivateProfileStringA("MySql", "Pass", byte.data(), path.toAscii().data());

    s.clear();
    byte.clear();
    s = ui->QlineName1->text();
    byte = s.toUtf8();
    ::WritePrivateProfileStringA("MsSql", "Name", byte.data(), path.toAscii().data());

    s.clear();
    byte.clear();
    s = ui->QlineUser1->text();
    byte = s.toUtf8();
    ::WritePrivateProfileStringA("MsSql", "User", byte.data(), path.toAscii().data());

    s.clear();
    byte.clear();
    s = ui->QlineIP1->text();
    byte = s.toUtf8();
    ::WritePrivateProfileStringA("MsSql", "IP", byte.data(), path.toAscii().data());

    s.clear();
    byte.clear();
    s = ui->QlinePass1->text();
    byte = s.toUtf8();
    ::WritePrivateProfileStringA("MsSql", "Pass", byte.data(), path.toAscii().data());

    QMessageBox::information( this, tr("提示"),tr("修改成功"));
    CMYSQL MySql;
    if(MySql._IsOpen())
    {
        qDebug()<<"88888888";
    }

}

void CSETODBC::on_QpbtnClose_clicked()
{
    close();
}

//关闭提示函数
void CSETODBC::closeEvent( QCloseEvent * event )
{
    switch( QMessageBox::information( this, tr("提示"),tr("系统选项"),tr("重新启动"), tr("退出"),0, 1 ) )
    {
        case 0:
        {
            QDir dir;
//            QString strPath = dir.currentPath() + "/ServerCenter.exe";
            QString strPath = dir.currentPath() + "/debug/ServerCenter.exe";
            QProcess::startDetached(strPath,QStringList());
            emit SigQuit();//系统退出信号发送
        }
            break;
        case 1:
            emit SigQuit();
            break;
        default:
            event->ignore();
            break;
    }
}
