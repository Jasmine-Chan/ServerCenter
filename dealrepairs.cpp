#include "dealrepairs.h"
#include "ui_dealrepairs.h"
#include "mysql.h"
#include <QMessageBox>

CDEALREPAIRS::CDEALREPAIRS(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDEALREPAIRS)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowCloseButtonHint&~Qt::WindowContextHelpButtonHint);
    setWindowModality(Qt::ApplicationModal);
    setWindowTitle(tr("报修处理"));
    ui->QlineHuman->setMaxLength(6);
}

CDEALREPAIRS::~CDEALREPAIRS()
{
    delete ui;
}

void CDEALREPAIRS::_Update(QString strAddr,QString strType,QString strStime,QString strEtime)
{
    m_strAddr.clear();
    m_strStime.clear();
    m_strAddr = strAddr;
    m_strStime = strStime;
    ui->QlblAddr->setText(strAddr);
    ui->QlblType->setText(strType);
    ui->QlblStime->setText(strStime);
    ui->QlblEtime->setText(strEtime);
    ui->QlineHuman->setFocus();
}

void CDEALREPAIRS::on_QpbtnDeal_clicked()
{
    CMYSQL CMysql;
    if(ui->QlineHuman->text()!=NULL)
    {
        QDateTime dateTime=QDateTime::currentDateTime();
        QString strDtime = dateTime.toString("yyyy-MM-dd hh:mm:ss");
        if(CMysql._DealRepairs(m_strAddr,m_strStime,strDtime,ui->QlineHuman->text(),ui->QtextMessage->toPlainText()))
        {
            QMessageBox::information(this,tr("提示"),tr("处理成功"));
            emit SigDealRepairs();
            close();
            return;
        }
    }else{
        QMessageBox::information(this,tr("提示"),tr("处理人不能为空"));
        ui->QlineHuman->setFocus();
        return;
    }
}

void CDEALREPAIRS::on_QpbtnClose_clicked()
{
    close();
}
