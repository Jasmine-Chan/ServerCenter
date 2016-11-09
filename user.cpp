#include "user.h"
#include "ui_user.h"
#include "mysql.h"
#include<QMessageBox>

CUSER::CUSER(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CUSER)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    ui->QlineOldPass->setEchoMode(QLineEdit::Password);
    ui->QlineNewPass->setEchoMode(QLineEdit::Password);
    ui->QlineNewPass2->setEchoMode(QLineEdit::Password);
    ui->QlineOldPass->setMaxLength(6);
    ui->QlineNewPass->setMaxLength(6);
    ui->QlineNewPass2->setMaxLength(6);
    ui->QlineName->setMaxLength(8);
    ui->QlineUser->setMaxLength(8);
}

CUSER::~CUSER()
{
    delete ui;
}

void CUSER::_Update(QString strName,QString strUser,int nFlag,int nGrade)
{
    m_strName = strName;
    m_strUser = strUser;
    ui->QlineName->setText(m_strName);
    ui->QlineUser->setText(m_strUser);
    m_nFlag = nFlag;
    m_nGrade = 3;
    ui->QlineNewPass->clear();
    ui->QlineNewPass2->clear();
    ui->QlineOldPass->clear();
    ui->QcomGrade->clear();
    switch(m_nFlag)
    {
    case 1:
        setWindowTitle(tr("添加用户"));
        ui->QpbtnOk->setText("添加");
        ui->label_3->hide();
        ui->QlineOldPass->hide();
        ui->label_6->show();
        ui->QcomGrade->show();
        ui->QcomGrade->addItem("操作员");
        if(nGrade == 1)
        {
            ui->QcomGrade->addItem("管理员");
        }

        break;
    case 2:
        setWindowTitle(tr("修改用户"));
        ui->QpbtnOk->setText("修改");
        ui->label_3->show();
        ui->QlineOldPass->show();
        ui->label_6->hide();
        ui->QcomGrade->hide();
        break;
    default :
        break;
    }
    ui->QlineUser->selectAll();
    ui->QlineUser->setFocus();
}

void CUSER::on_QpbtnOk_clicked()
{
    CMYSQL CMysql;
    switch(m_nFlag)
    {
    case 1:
        if(ui->QlineUser->text()!=NULL)
        {
            if(ui->QlineNewPass->text()!=NULL)
            {
                if(ui->QlineNewPass->text() == ui->QlineNewPass2->text())
                {
                    if(CMysql._InsertUser(ui->QlineName->text(),ui->QlineUser->text(),ui->QlineNewPass->text(),m_nGrade))
                    {
                        QMessageBox::information(this,tr("提示"),tr("添加成功"));
                        emit SigUpdateUser();
                        close();
                        return;
                    }else{
                       QMessageBox::information(this,tr("提示"),tr("添加失败，请检查数据库"));
                       return;
                    }
                }else{
                    QMessageBox::information( this, tr("提示"),tr("两次输入密码不一致"));
                    ui->QlineNewPass2->selectAll();
                    ui->QlineNewPass2->setFocus();
                    return;
                }
            }else{
                QMessageBox::information( this, tr("提示"),tr("密码不能为空"));
                ui->QlineNewPass->setFocus();
                return;
            }
        }else{
            QMessageBox::information( this, tr("提示"),tr("用户名不能为空"));
            ui->QlineName->setFocus();
            return;
        }
        break;
    case 2:
        if(ui->QlineUser->text()!=NULL)
        {
            if(CMysql._FindUser(m_strUser,ui->QlineOldPass->text()))
            {
                if(ui->QlineNewPass->text()!=NULL)
                {
                if(ui->QlineNewPass->text() == ui->QlineNewPass2->text())
                {
                    if(CMysql._UpdateUser(m_strUser,ui->QlineName->text(),ui->QlineUser->text(),ui->QlineNewPass->text()))
                    {
                        QMessageBox::information(this,tr("提示"),tr("修改成功"));
                        emit SigUpdateUser();
                        close();
                        return;
                    }else{
                       QMessageBox::information(this,tr("提示"),tr("修改失败，请检查数据库"));
                       return;
                    }
                }else{
                    QMessageBox::information( this, tr("提示"),tr("两次输入密码不一致"));
                    ui->QlineNewPass2->selectAll();
                    ui->QlineNewPass2->setFocus();
                    return;
                }
                }else{
                    QMessageBox::information( this, tr("提示"),tr("密码不能为空"));
                    ui->QlineOldPass->setFocus();
                    return;
                }
            }else{
                QMessageBox::information( this, tr("提示"),tr("密码不正确"));
                ui->QlineOldPass->selectAll();
                ui->QlineOldPass->setFocus();
                return;
            }
        }else{
            QMessageBox::information( this, tr("提示"),tr("用户名不能为空"));
            ui->QlineUser->setFocus();
            return;
        }
        break;
    default :
        break;
    }
}

void CUSER::on_QpbtnClose_clicked()
{
    close();
}

void CUSER::on_QcomGrade_activated(int index)
{
//    qDebug()<<"index"<<index;
    if(index == 0)
    {
        m_nGrade = 3;
    }else
        m_nGrade = 2;
}
