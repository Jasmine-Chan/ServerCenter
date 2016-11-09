#include "find.h"
#include "ui_find.h"
#include <QDebug>
#include <QMessageBox>

CFIND::CFIND(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFIND)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    ui->QlineFind->setMaxLength(20);
}

CFIND::~CFIND()
{
    delete ui;
}

void CFIND::_Update(int nPage)
{
    m_nPage = nPage;
    m_nIndex = 0;
    ui->QlineFind->clear();
    ui->QchkFind->clear();
    switch(m_nPage)
    {
    case 3:
        setWindowTitle(tr("报警记录查找"));
        ui->QchkFind->addItem(QWidget::tr("地址"));
        ui->QchkFind->addItem(QWidget::tr("IP地址"));
        ui->QchkFind->addItem(QWidget::tr("未处理"));
        break;
    case 4:
        setWindowTitle(tr("报修记录查找"));
        ui->QchkFind->addItem(QWidget::tr("地址"));
        ui->QchkFind->addItem(QWidget::tr("报修类型"));
        ui->QchkFind->addItem(QWidget::tr("未处理"));
        break;
    case 7:
        setWindowTitle(tr("系统日志查找"));
        ui->QchkFind->addItem(QWidget::tr("用户"));
        ui->QchkFind->addItem(QWidget::tr("姓名"));
        ui->QchkFind->addItem(QWidget::tr("操作类型"));
        break;
    case 8:
        setWindowTitle(tr("设备日志查找"));
        ui->QchkFind->addItem(QWidget::tr("地址"));
        ui->QchkFind->addItem(QWidget::tr("IP地址"));
        ui->QchkFind->addItem(QWidget::tr("状态"));
        break;
    case 9:
        setWindowTitle(tr("刷卡记录查找"));
        ui->QchkFind->addItem(QWidget::tr("卡号"));
        ui->QchkFind->addItem(QWidget::tr("设备地址"));
        ui->QchkFind->addItem(QWidget::tr("所属地址"));
        break;
    default :
        break;
    }

    QDateTime dateTime=QDateTime::currentDateTime();
    ui->QdateETime->setDateTime(dateTime);  //setDate(dateTime.date());
    dateTime = dateTime.addDays(-5);
    ui->QdateSTime->setDateTime(dateTime);  //setDate(dateTime.date());
    ui->QchkFind->setCurrentIndex(m_nIndex);
    ui->QlineFind->show();
    ui->QlineFind->setFocus();
    m_nTime = 0;
    ui->QrbtnTime->setChecked(false);
    ui->QdateETime->setEnabled(false);
    ui->QdateSTime->setEnabled(false);
}

void CFIND::on_QpbtnOk_clicked()
{
       if(ui->QdateETime > ui->QdateSTime)
       {
           emit SigFind(m_nIndex,m_nTime,ui->QlineFind->text(),ui->QdateSTime->text(),ui->QdateETime->text());
           close();
       }
       else
       {
           QMessageBox::information(this,tr("提示"),tr("开始时间不能大于结束时间"));
           ui->QdateSTime->setFocus();
       }
}

void CFIND::on_QpbtnClose_clicked()
{
    close();
}

void CFIND::on_QchkFind_activated(int index)
{
    m_nIndex = index;
    if((m_nPage == 3 || m_nPage == 4)&&m_nIndex == 2)
    {
        ui->QlineFind->hide();
    }
    else
        ui->QlineFind->show();
}

void CFIND::on_QrbtnTime_clicked()
{
    if(ui->QrbtnTime->isChecked())
    {
        m_nTime = 1;
        ui->QdateETime->setEnabled(true);
        ui->QdateSTime->setEnabled(true);
        ui->QdateSTime->setFocus();
    }
    else
    {
        m_nTime = 0;
        ui->QdateETime->setEnabled(false);
        ui->QdateSTime->setEnabled(false);
    }
}
