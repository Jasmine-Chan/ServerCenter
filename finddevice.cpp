#include "finddevice.h"
#include "ui_finddevice.h"
#include <QMessageBox>

CFINDDEVICE::CFINDDEVICE(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFINDDEVICE)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    ui->QlineFind->setMaxLength(20);
}

CFINDDEVICE::~CFINDDEVICE()
{
    delete ui;
}

void CFINDDEVICE::_Update(int nPage)
{
    m_nIndex = 0;
    ui->QlineFind->clear();
    ui->QchkFind->clear();
    switch(nPage)
    {
    case 1:
        setWindowTitle(tr("住户设备查找"));
        ui->QchkFind->addItem(QWidget::tr("地址"));
        ui->QchkFind->addItem(QWidget::tr("IP地址"));
        ui->QchkFind->addItem(QWidget::tr("Mac地址"));
        ui->QchkFind->addItem(QWidget::tr("姓名"));
        ui->QchkFind->addItem(QWidget::tr("电话"));
        break;
    case 2:
        setWindowTitle(tr("中间设备查找"));
        ui->QchkFind->addItem(QWidget::tr("地址"));
        ui->QchkFind->addItem(QWidget::tr("IP地址"));
        ui->QchkFind->addItem(QWidget::tr("Mac地址"));
        break;
    }
    ui->QlineFind->setFocus();
    ui->QchkFind->setCurrentIndex(m_nIndex);
}

void CFINDDEVICE::on_QpbtnFind_clicked()
{
    if(ui->QlineFind->text() != NULL)
    {
        emit SigDeviceFind(m_nIndex,ui->QlineFind->text());
        close();
    }else{
        QMessageBox::information(this,tr("提示"),tr("查找条件不能为空"));
        ui->QlineFind->setFocus();
    }
}

void CFINDDEVICE::on_QpbtnClose_clicked()
{
    close();
}

void CFINDDEVICE::on_QchkFind_activated(int index)
{
    m_nIndex = index;
}
