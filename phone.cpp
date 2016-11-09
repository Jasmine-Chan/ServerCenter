#include "phone.h"
#include "ui_phone.h"

CPHONE::CPHONE(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CPHONE)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    m_nFlag = 0;
    ui->QlineDepartment->setMaxLength(12);
    ui->QlineHuman->setMaxLength(8);
    ui->QlinePhone->setMaxLength(11);
}

CPHONE::~CPHONE()
{
    delete ui;
}

void CPHONE::_Update(QString strDepartment,QString strHuman,QString strPhone,int nFlag)
{
    m_nFlag = nFlag;
    ui->QlineDepartment->setText(strDepartment);
    ui->QlineHuman->setText(strHuman);
    ui->QlinePhone->setText(strPhone);
    m_strDepartment = strDepartment;
    m_strHuman = strHuman;
    m_strPhone = strPhone;
    switch(m_nFlag)
    {
    case 1:
        setWindowTitle("添加常用电话");
        ui->QpbtnOk->setText("添加");
        break;
    case 2:
        setWindowTitle("修改常用电话");
        ui->QpbtnOk->setText("修改");
        break;
    default :
        break;
    }
    ui->QlineDepartment->selectAll();
    ui->QlineDepartment->setFocus();
}

void CPHONE::on_QpbtnOk_clicked()
{
    CMYSQL CMysql;
    switch(m_nFlag)
    {
    case 1:
        if(ui->QlineHuman->text()!=NULL)
        {
            if(ui->QlinePhone->text()!=NULL)
            {
                if(CMysql._InsertPhone(ui->QlineDepartment->text(),ui->QlineHuman->text(),ui->QlinePhone->text()))
                {
                    QMessageBox::information( this, tr("提示"),tr("添加成功"));
                    emit SigPhone();
                    close();
                    return;
                }
            }else{
                QMessageBox::information( this, tr("提示"),tr("电话不能为空"));
                ui->QlinePhone->setFocus();
                return;
            }
        }else{
            QMessageBox::information( this, tr("提示"),tr("联系人不能为空"));
            ui->QlineHuman->setFocus();
            return;
        }
        break;
    case 2:
        if(ui->QlineHuman->text()!=NULL)
        {
            if(ui->QlinePhone->text()!=NULL)
            {
                if(CMysql._UpdatePhone(ui->QlineDepartment->text(),ui->QlineHuman->text(),ui->QlinePhone->text(),m_strDepartment,m_strHuman,m_strPhone))
                {
                    QMessageBox::information( this, tr("提示"),tr("修改成功"));
                    emit SigPhone();
                    close();
                    return;
                }
            }else{
                QMessageBox::information( this, tr("提示"),tr("电话不能为空"));
                ui->QlinePhone->setFocus();
                return;
            }
        }else{
            QMessageBox::information( this, tr("提示"),tr("联系人不能为空"));
            ui->QlineHuman->setFocus();
            return;
        }
        break;
    default :
        break;
    }
}

void CPHONE::on_QpbtnClose_clicked()
{
    close();
}
