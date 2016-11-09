#include "repairs.h"
#include "mysql.h"
#include "ui_repairs.h"

CREPAIRS::CREPAIRS(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CREPAIRS)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    ui->QlineRepairs->setMaxLength(8);
}

CREPAIRS::~CREPAIRS()
{
    delete ui;
}

void CREPAIRS::on_QpbtnOk_clicked()
{
    CMYSQL CMysql;
    switch(m_nFlag)
    {
    case 1:
        if(ui->QlineRepairs->text()!=NULL)
        {
            if(CMysql._InsertRepairs(ui->QlineRepairs->text()))
            {
                QMessageBox::information( this, tr("提示"),tr("添加成功"));
                emit SigRepairs();
                close();
                return;
            }
        }
        else
        {
            QMessageBox::information( this, tr("提示"),tr("报修类型不能为空"));
            return;
        }
        break;
    case 2:
        if(ui->QlineRepairs->text()!=NULL)
        {
            if(CMysql._UpdateRepairs(ui->QlineRepairs->text(),m_strRepairs))
            {
                QMessageBox::information( this, tr("提示"),tr("修改成功"));
                emit SigRepairs();
                close();
                return;
            }
        }
        else
        {
            QMessageBox::information( this, tr("提示"),tr("报修类型不能为空"));
            ui->QlineRepairs->setFocus();
            return;
        }
        break;
    default :
        break;
    }
}

void CREPAIRS::on_QpbtnClose_clicked()
{
close();
}

void CREPAIRS::_Update(QString strRepairs,int nFlag)
{
    m_nFlag = nFlag;
    m_strRepairs = strRepairs;
    ui->QlineRepairs->setText(m_strRepairs);
    switch(m_nFlag)
    {
    case 1:
        setWindowTitle("添加报修类型");
        ui->QpbtnOk->setText("添加");
        break;
    case 2:
        setWindowTitle("修改报修类型");
        ui->QpbtnOk->setText("修改");
        break;
    default :
        break;
    }
    ui->QlineRepairs->selectAll();
    ui->QlineRepairs->setFocus();
}
