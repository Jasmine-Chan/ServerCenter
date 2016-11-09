#include "findcard.h"
#include "ui_findcard.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include "connectionpoolmssql.h"
#include "connectionpoolmysql.h"
CFINDCARD::CFINDCARD(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFINDCARD)
{
    ui->setupUi(this);
    setWindowTitle(tr("查找IC卡"));
    ui->QlineTAddr->setMaxLength(20);
    ui->QlineIcCard->setMaxLength(20);
}

CFINDCARD::~CFINDCARD()
{
    delete ui;
}

void CFINDCARD::_Update()
{
    m_nFlag = 0;
    ui->QchkIcCard->setChecked(false);
    ui->QchkMAddr->setChecked(false);
    ui->QchkTAddr->setChecked(false);
    ui->QchkValidity->setChecked(false);
    ui->QlineIcCard->setEnabled(false);
    ui->QlineTAddr->setEnabled(false);
    ui->QcomMAddr->setEnabled(false);
    ui->QdateValidity->setEnabled(false);
    ui->QcomMAddr->clear();
    ui->QdateValidity->setDate(QDateTime::currentDateTime().date());
    QString SQL = "SELECT Middle_intra_addr from middle";
//    QSqlQuery query;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    if(query.exec(SQL))
    {
        while(query.next())
        {
            QString str = QString(query.value(0).toByteArray().data());
            if(str.left(1) == "Z"){
            }else{
            QString strItem = str.left(4) + "栋";
            str = str.right(str.length() - 4);
            strItem = strItem + str.left(3) + "单元";
            str = str.right(str.length() - 3);
            strItem = strItem + str.left(3) + "楼";
            str = str.right(str.length() - 3);
            strItem = strItem + str.left(3) + "房";
            str = str.right(str.length() - 3);
            strItem = strItem + str.left(3) + "号设备";

            ui->QcomMAddr->addItem(strItem);
            }
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}


void CFINDCARD::on_QpbtnFind_clicked()
{
    QString str;
    str.clear();
    if(m_nFlag){
        if(m_nFlag&4){
            QString str1 = ui->QcomMAddr->currentText();
            str = str1.left(4);
            str1 = str1.right(str1.length() - 5);

            str = str + str1.left(3);
            str1 = str1.right(str1.length() - 5);

            str = str + str1.left(3);
            str1 = str1.right(str1.length() - 4);

            str = str + str1.left(3);
            str1 = str1.right(str1.length() - 4);

            str = str + str1.left(3);
        }
        emit SigFindCard(m_nFlag,ui->QlineIcCard->text(),ui->QlineTAddr->text(),str,ui->QdateValidity->text());
        close();
    }else
        QMessageBox::information(this,tr("提示"),tr("请选择查询条件"));

}

void CFINDCARD::on_QchkIcCard_clicked(bool checked)
{
    if(checked)
    {
        m_nFlag += 1;
        ui->QlineIcCard->setEnabled(true);
         ui->QlineIcCard->setFocus();
    }else{
        m_nFlag -= 1;
        ui->QlineIcCard->setEnabled(false);
    }
}

void CFINDCARD::on_QchkTAddr_clicked(bool checked)
{
    if(checked)
    {
        m_nFlag += 2;
        ui->QlineTAddr->setEnabled(true);
        ui->QlineTAddr->setFocus();
    }else{
        m_nFlag -= 2;
        ui->QlineTAddr->setEnabled(false);
    }
}

void CFINDCARD::on_QchkMAddr_clicked(bool checked)
{
    if(checked)
    {
        m_nFlag += 4;
        ui->QcomMAddr->setEnabled(true);
    }else{
        m_nFlag -= 4;
        ui->QcomMAddr->setEnabled(false);
    }
}

void CFINDCARD::on_QpbtnClose_clicked()
{
    close();
}

void CFINDCARD::on_QchkValidity_clicked(bool checked)
{
    if(checked)
    {
        m_nFlag += 8;
        ui->QdateValidity->setEnabled(true);
    }else{
        m_nFlag -= 8;
        ui->QdateValidity->setEnabled(false);
    }
}
