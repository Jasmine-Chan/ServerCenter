#include "card.h"
#include "mysql.h"
#include "ui_card.h"
#include <QMessageBox>
CCARD::CCARD(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CCARD)
{
    ui->setupUi(this);
    ui->QlineAddr->setInputMask("000栋000单元000楼000房000号设备;");
    ui->QlineCardNum->setInputMask("hh:hh:hh:hh;");
    m_nLimits = 0;
}

void CCARD::_Update(int nFlag,QString strCrad,QString strAddr,QString strTime)
{
    ui->QlineAddr->setText(strAddr);
    ui->QlineCardNum->setText(strCrad.right(11));
    m_nFlag = nFlag;
    m_strTime = strTime;
    switch(nFlag)
    {
        case 1:
            setWindowTitle(tr("添加IC卡"));
            ui->QpbtnIsOk->setText("添加");
            ui->QdateValidity->setDate(QDateTime::currentDateTime().date().addYears(2));
            init();
            break;
        case 2:     //修改
            m_strCard = strCrad.right(11);
            m_strAddr = strAddr;
            setWindowTitle(tr("修改IC卡信息"));
            ui->QpbtnIsOk->setText("修改");
            ui->QdateValidity->setDate(QDate::fromString(strTime, "yyyy-MM-dd"));
            init();
            break;
        default:
            break;
    }
    ui->QlineCardNum->selectAll();
    ui->QlineCardNum->setFocus();
}

CCARD::~CCARD()
{
    delete ui;
}

void CCARD::on_QpbtnIsOk_clicked()
{
    CMYSQL CMysql;
    if(ui->QlineCardNum->text().length() == 11)
    {
        switch(m_nFlag)
        {
            case 1:
                if(CMysql._SelectIcCard(ui->QlineCardNum->text()) == 0)
                {
                    if(CMysql._AddIcCard(m_nLimits,T,ui->QlineCardNum->text(),ui->QlineAddr->text(),ui->QdateValidity->text()))
                    {
                        QMessageBox::information(this,tr("提示"),tr("添加成功"));
                        ui->QlineCardNum->selectAll();
                        emit SigRefurbish("00:" + ui->QlineCardNum->text());
                    }
                }
                else
                {
                    QMessageBox::information(this,tr("提示"),tr("卡号已存在"));
                    ui->QlineCardNum->selectAll();
                }

                break;
            case 2:
                if(ui->QlineCardNum->text() != m_strCard)
                {
                    if(CMysql._SelectIcCard(ui->QlineCardNum->text()) != 0)
                    {
                        QMessageBox::information(this,tr("提示"),tr("卡号已存在"));
                        ui->QlineCardNum->selectAll();
                        return;
                    }
                }
                if(CMysql._UpdateToIcCard(ui->QlineCardNum->text(),ui->QlineAddr->text(),m_strCard,m_strAddr,ui->QdateValidity->text()))
                {
                    QMessageBox::information(this,tr("提示"),tr("修改成功"));
                    QDate Update = QDate::fromString(ui->QdateValidity->text(), "yyyy-MM-dd");
                    QDate Date = QDate::fromString(m_strTime, "yyyy-MM-dd");
                    QDate Now = QDateTime::currentDateTime().date();
                    if(((Update > Now) && (Date < Now)) || ((Update < Now) && (Date > Now)))
                    {
                        emit SigUpdateTime(ui->QlineCardNum->text());
                    }
                    close();
                    emit SigRefurbish("00:" + ui->QlineCardNum->text());
                }
                else
                {
                    QMessageBox::information(this,tr("提示"),tr("修改失败，请检查数据库连接!"));
                }
                break;
            default :
                break;
        }
    }
    else
    {
        QMessageBox::information(this,tr("提示"),tr("请输入完整的卡号\n不够位的请补零"));
        ui->QlineCardNum->selectAll();
    }
}

void CCARD::on_QpbtnClose_clicked()
{
    close();
}


void CCARD::init()
{
    ui->QcheckBox1->setChecked(false);
    ui->QcheckBox2->setChecked(false);
    ui->QcheckBox3->setChecked(false);
    ui->QcomCheck1->setEnabled(false);
    ui->QcomCheck2->setEnabled(false);
    ui->QcomCheck3->setEnabled(false);
    switch(m_nFlag){
    case 1:
        ui->QcheckBox1->setEnabled(true);
        ui->QcheckBox2->setEnabled(true);
        ui->QcheckBox3->setEnabled(true);
        break;
    case 2:
        ui->QcheckBox1->setEnabled(false);
        ui->QcheckBox2->setEnabled(false);
        ui->QcheckBox3->setEnabled(false);
        break;
    }

    CMYSQL MySql;
    ui->QcomCheck1->clear();
    ui->QcomCheck2->clear();
    ui->QcomCheck3->clear();

    T.clear();
    for(int i = 0;i < 4;i++)
    {
        nLimits[i] = 0;
    }
    if(MySql._GetAllMiddle(&T))
    {
        for(int i = 0;i < T.size();i++)
        {
            QString str = T.at(i);
            QString strItem = str.left(4) + "栋";
            str = str.right(str.length() - 4);
            strItem = strItem + str.left(3) + "单元";
            str = str.right(str.length() - 3);
            strItem = strItem + str.left(3) + "楼";
            str = str.right(str.length() - 3);
            strItem = strItem + str.left(3) + "房";
            str = str.right(str.length() - 3);
            strItem = strItem + str.left(3) + "号设备";
            ui->QcomCheck1->addItem(strItem);
            ui->QcomCheck2->addItem(strItem);
            ui->QcomCheck3->addItem(strItem);

        }
    }
}

void CCARD::on_QcheckBox1_clicked(bool checked)
{
    if(checked)
    {
        ui->QcomCheck1->setEnabled(true);
        nLimits[0] = 1;
        m_nLimits += 1;
    }else{
        ui->QcomCheck1->setEnabled(false);
        nLimits[0] = 0;
        m_nLimits -= 1;
    }
}

void CCARD::on_QcheckBox2_clicked(bool checked)
{
    if(checked)
    {
        ui->QcomCheck2->setEnabled(true);
        nLimits[1] = 1;
        m_nLimits += 2;
    }else{
        ui->QcomCheck2->setEnabled(false);
        nLimits[1] = 0;
        m_nLimits -= 2;
    }
}

void CCARD::on_QcheckBox3_clicked(bool checked)
{
    if(checked)
    {
        ui->QcomCheck3->setEnabled(true);
        nLimits[2] = 1;
        m_nLimits += 4;
    }else{
        ui->QcomCheck3->setEnabled(false);
        nLimits[2] = 0;
        m_nLimits -= 4;
    }
}
