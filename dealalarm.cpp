#include "dealalarm.h"
#include "ui_dealalarm.h"

CDEALALARM::CDEALALARM(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDEALALARM)
{
    ui->setupUi(this);
    setWindowTitle(tr("报警处理"));
    setWindowModality(Qt::ApplicationModal);
    ui->QlineDealHuman->setMaxLength(6);
    m_nAlarmType = 0;
}

CDEALALARM::~CDEALALARM()
{
    delete ui;
}


void CDEALALARM::_Update(SAlarmDevice data)
{
    strStime.clear();
    strStime = QString(data.gcStime);
    ui->QlblAddr->setText(data.gcAddr);
    ui->QlblEtime->setText(data.gcEtime);
    ui->QlblFence->setText("防区" + QString::number(data.nFenceId));
    ui->QlblIp->setText(data.gcIpAddr);
    ui->QlblName->setText(data.gcName);
    ui->QlblPhone->setText(data.gcPhone1);
    ui->QlblStime->setText(data.gcStime);
    QString str6;
    str6.clear();
    m_nAlarmType = data.nAlarmType;
    switch(data.nAlarmType)
    {
    case 0:
//        str6 = "特殊报警";
        str6 = "火警";
        break;
    case 2:
        str6 = "红外";
        break;
    case 3:
        str6 = "门铃";
        break;
    case 4:
        str6 = "烟感";
        break;
    case 6:
        str6 = "门磁1";
        break;
    case 7:
        str6 = "遥控SOS";
        break;
    case 8:
        str6 = "门磁2";
        break;
    case 10:
        str6 = "水浸";
        break;
    case 12:
        str6 = "煤气";
        break;
    case 13:
        str6 = "门磁3";
        break;
    case 14:
        str6 = "胁迫开门";
        break;
    case 15:
        str6 = "设备强拆";
        break;
    case 16:
        str6 = "开门超时";
        break;
    default :
        str6 = "错误";
        break;
    }
    ui->QlblType->setText(str6);
    ui->Qrbtn1->setChecked(true);
    ui->QlineDealHuman->setFocus();
}

void CDEALALARM::on_Qrbtn1_clicked()
{
    if(ui->Qrbtn1->isChecked())
    {
        ui->Qrbtn2->setChecked(false);
    }
}

void CDEALALARM::on_Qrbtn2_clicked()
{
    if(ui->Qrbtn2->isChecked())
    {
        ui->Qrbtn1->setChecked(false);
    }
}

void CDEALALARM::on_QpbtnDeal_clicked()
{
    QString  strAlarmStatu = "误报";
    if(ui->Qrbtn2->isChecked())
    {
        strAlarmStatu = "非误报";
    }
    QString strDealHuman = ui->QlineDealHuman->text();
    QString strMessage = ui->QtextMessage->toPlainText();
    if(strDealHuman != NULL)
    {
        emit SigDealAlarm(m_nAlarmType,strStime,strAlarmStatu,strDealHuman,strMessage);
        QMessageBox::information( this, tr("提示"),tr("处理成功"));
        close();
    }else{
        QMessageBox::information( this, tr("提示"),tr("处理人不能为空"));
        ui->QlineDealHuman->setFocus();
    }
}

void CDEALALARM::on_QpbtnClose_clicked()
{
    close();
}
