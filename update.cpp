#include "update.h"
#include "ui_update.h"
#include <QDebug>
#include<QMessageBox>
CUPDATE::CUPDATE(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CUPDATE)
{
    ui->setupUi(this);
    setWindowTitle(tr("地址设置"));
    setWindowModality(Qt::ApplicationModal);
    m_pAddr = new char[20];
    ui->QlineIp->setInputMask("000.000.000.000");
    ui->QlineGateway->setInputMask("000.000.000.000");
    ui->QlineMac->setInputMask("hh:hh:hh:hh");
    ui->QlineSubnet->setInputMask("000.000.000.000");
    ui->QlineServerIp->setInputMask("000.000.000.000");
    ui->QlineAddr_2->setInputMask("0");
    m_nTimerId = m_nTimerId1 = 0;

}

CUPDATE::~CUPDATE()
{
    delete ui;
}

void CUPDATE::_UpdateAddr()
{
    if(m_nTimerId1 != 0)
        killTimer(m_nTimerId1);
    QMessageBox::information( this, tr("提示"),tr("写地址成功"));
}

void CUPDATE::_ReadUpdateA8(unsigned char *buf, QString strIp)
{
    char gAddr[20] = {0};
    memcpy(gAddr,buf+8,20);
    if((m_strIp == strIp)&&(memcmp(m_pAddr,gAddr,20) == 0))
    {
        if(m_nTimerId != 0)
            killTimer(m_nTimerId);
        QString str;
        int nSyn = 0;
        int i;
        switch(buf[8])      //解析设备地址
        {
            case 'S':
            case 'H':
                for(i = 0;i < 11;i++)
                {
                    int kk = buf[9+i] - 48;
                    str = str + QString::number(kk);
                    if(i==3||i==5||i==7||i==9)
                        str = str + "-";
                }
                break;
            case 'M':
                for(i = 0;i < 7;i++)
                {
                    int kk = buf[9+i] - 48;
                    str = str + QString::number(kk);
                    if(i==3||i==5)
                        str = str + "-";
                }
                break;
            case 'Z':
            case 'W':
                for(i = 0;i < 4;i++)
                {
                    int kk = buf[9+i] - 48;
                    str = str + QString::number(kk);
                }
                break;
            default :
                break;
        }
        nSyn = buf[31+i] - 48;
        ui->QlineAddr->setText(str);
        ui->QlineAddr_2->setText(QString::number(nSyn));

        str.clear();
        for(i = 2;i<6;i++)
        {
            int kk = buf[50+i];
            if(kk < 16) str = str + "0";
            str = str + QString::number(kk, 16).toUpper();
            if(i != 5) str = str + ":";
        }

        ui->QlineMac->setText(str);
        str.clear();
        for(i = 0;i<4;i++)
        {
            int kk = buf[56+i];
            str = str + QString::number(kk);
            if(i != 3) str = str + ".";
        }

        ui->QlineIp->setText(str);
        str.clear();
        for(i = 0;i<4;i++)
        {
            int kk = buf[60+i];
            str = str + QString::number(kk);
            if(i != 3) str = str + ".";
        }

        ui->QlineSubnet->setText(str);
        str.clear();
        for(i = 0;i<4;i++)
        {
            int kk = buf[64+i];
            str = str + QString::number(kk);
            if(i != 3) str = str + ".";
        }

        ui->QlineGateway->setText(str);
        str.clear();
        for(i = 0;i<4;i++)
        {
            int kk = buf[68+i];
            str = str + QString::number(kk);
            if(i != 3) str = str + ".";
        }

        ui->QlineServerIp->setText(str);
        QMessageBox::information( this, tr("提示"),tr("读地址成功"));
    }
}

void CUPDATE::_ReadUpdate(unsigned char *buf,QString strIp)
{
    char gAddr[20] = {0};
    memcpy(gAddr,buf+8,20);
    if((m_strIp == strIp)&&(memcmp(m_pAddr,gAddr,20) == 0))
    {
        if(m_nTimerId != 0)
            killTimer(m_nTimerId);
        QString str;
        int nSyn = 0;
        int i;
        switch(buf[8])
        {
        case 'S':
#if 0
            for(i = 0;i < 11;i++)
            {
                int kk = buf[9+i] - 48;
                str = str + QString::number(kk);
                if(i==3||i==5||i==7||i==9)
                    str = str + "-";
            }
#endif
            for(i = 1;i < 11;i++)
            {
                int kk = buf[9+i] - 48;     //ASCLL 48表示0
                str = str + QString::number(kk);
                if(i==3||i==5||i==7||i==9)
                {
                    if(i==9)
                    {
                        str = str + "-";
                    }
                    else
                        str = str + "-0";
                }
            }
            break;
        case 'M':
#if 0
            for(i = 0;i < 7;i++)
            {
                int kk = buf[9+i] - 48;
                str = str + QString::number(kk);
                if(i==3||i==5)
                    str = str + "-";
            }
#endif
            for(i = 01;i < 7;i++)
            {
                int kk = buf[9+i] - 48;
                str = str + QString::number(kk);
                if(i==3||i==5)
                {
                    if(i==5)
                    {
                        str = str + "-";
                    }else
                        str = str + "-0";
                }
            }
            break;
        case 'Z':
        case 'W':
            for(i = 1;i < 5;i++)
            {
                int kk = buf[9+i] - 48;
                str = str + QString::number(kk);
                if(i==3)
                    str = str + "-";
            }
            break;
        default :
            break;
        }
        nSyn = buf[31+i] - 48;
        ui->QlineAddr->setText(str);
        ui->QlineAddr_2->setText(QString::number(nSyn));

        str.clear();
        for(i = 2;i<6;i++)
        {
            int kk = buf[50+i];
            if(kk < 16)
                str = str + "0";
            str = str + QString::number(kk, 16).toUpper();
            if(i != 5)
                str = str + ":";
        }

        ui->QlineMac->setText(str);
        str.clear();
        for(i = 0;i<4;i++)
        {
            int kk = buf[56+i];
            str = str + QString::number(kk);
            if(i != 3)
                str = str + ".";
        }

        ui->QlineIp->setText(str);
        str.clear();
        for(i = 0;i<4;i++)
        {
            int kk = buf[60+i];
            str = str + QString::number(kk);
            if(i != 3) str = str + ".";
        }

        ui->QlineSubnet->setText(str);
        str.clear();
        for(i = 0;i<4;i++)
        {
            int kk = buf[64+i];
            str = str + QString::number(kk);
            if(i != 3)
                str = str + ".";
        }

        ui->QlineGateway->setText(str);
        str.clear();
        for(i = 0;i<4;i++)
        {
            int kk = buf[68+i];
            str = str + QString::number(kk);
            if(i != 3)
                str = str + ".";
        }

        ui->QlineServerIp->setText(str);
        QMessageBox::information( this, tr("提示"),tr("读地址成功"));
    }
}

void CUPDATE::_Updata(QString strIp,QString strAddr,QString strType)
{
    memset(m_pAddr,0,20);
    m_nFlag = 0;
    QString str;
    m_strIp.clear();
    m_strAddr.clear();
    m_strIp = strIp;
    m_strAddr = strAddr;
    setWindowTitle(strType+"地址设置");
    if(strType == "室内机")
    {
        ui->QlineAddr->setInputMask("000-000-000-000-000");
        m_nFlag = 83;
        str.clear();

        str = "0" + strAddr.left(3);
        strAddr = strAddr.right(strAddr.length() - 5);
        str = str + strAddr.left(2);
        strAddr = strAddr.right(strAddr.length() - 4);
        str = str + strAddr.left(2);
        strAddr = strAddr.right(strAddr.length() - 4);
        str = str + strAddr.left(2);
        strAddr = strAddr.right(strAddr.length() - 3);
        str = str + strAddr.left(1);
    }else if(strType == "门口机")
    {
        ui->QlineAddr->setInputMask("000-000-0");
        m_nFlag = 77;
        str.clear();

        str = "0" + strAddr.left(3);
        strAddr = strAddr.right(strAddr.length() - 5);
        str = str + strAddr.left(2);
        strAddr = strAddr.right(strAddr.length() - 3);
        str = str + strAddr.left(1);
    }else if(strType == "围墙机")
    {
        ui->QlineAddr->setInputMask("000-0");
        m_nFlag = 87;
        str = "0" + strAddr.left(3);
        strAddr = strAddr.right(strAddr.length() - 4);
        str = str + strAddr.left(1);
    }else if(strType == "中心机")
    {
        ui->QlineAddr->setInputMask("000-0");
        m_nFlag = 90;
        str = "0" + strAddr.left(3);
        strAddr = strAddr.right(strAddr.length() - 4);
        str = str + strAddr.left(1);
    }else if(strType == "别墅室内机")
    {
        ui->QlineAddr->setInputMask("000-0");
        m_nFlag = 66;
        str = "0" + strAddr.left(3);
        strAddr = strAddr.right(strAddr.length() - 4);
        str = str + strAddr.left(1);
    }
    m_pAddr[0] = m_nFlag;
    ui->QlineIp->setText(m_strIp);
    ui->QlineAddr->setText(m_strAddr);

    QByteArray byte;
    byte = str.toAscii();

    memcpy(m_pAddr+1,byte.data(),byte.size());
    ui->QlineMac->clear();
    ui->QlineGateway->clear();
    ui->QlineSubnet->clear();
    ui->QlineServerIp->clear();
    ui->QlineAddr->selectAll();
    ui->QlineAddr->setFocus();

}

void CUPDATE::timerEvent( QTimerEvent *event )
{
    if(event->timerId()== m_nTimerId)
    {
        QMessageBox::information(this,tr("提示"),tr("读地址失败"));
        killTimer(m_nTimerId);
    }
    if(event->timerId()== m_nTimerId1)
    {
        QMessageBox::information(this,tr("提示"),tr("写地址失败"));
        killTimer(m_nTimerId1);
    }
}

void CUPDATE::on_QpbtnRead_clicked()
{
    emit SigRead(m_pAddr,m_strIp);
    m_nTimerId = startTimer(5000);
}

void CUPDATE::on_QpbtnWrite_clicked()
{
    char buf[64] = {0};
    memcpy(buf,m_pAddr,20);
    memcpy(buf+22,m_pAddr,1);
    int t = 0;
    QString str,str1,str2;
    str.clear();
    str1.clear();
    str2.clear();
    str = ui->QlineAddr->text();
    switch(m_nFlag)
    {
    case 83:
        if(str!="----")
        {
            t = t + 1;
            int j = str.indexOf("-");
            if(j == 0)
            {
                QMessageBox::information( this, tr("提示"),tr("地址输入不正确"));
                return;
            }
            str1 = str.left(j);
            for(int i = 0;i < (4-str1.length());i++)
            {
                str2 = str2 + "0";
            }
            str2 = str2 + str1;
            str = str.right(str.length() - j - 1);
            str1.clear();

            j = str.indexOf("-");
            str1 = str.left(j);
            if(j > 2)
            {
               str1 = str1.right(2);
            }else{
            for(int i = 0;i < (2-str1.length());i++)
            {
                str2 = str2 + "0";
            }
            }
            str2 = str2 + str1;
            str = str.right(str.length() - j - 1);
            str1.clear();

            j = str.indexOf("-");
            str1 = str.left(j);
            if(j > 2)
            {
               str1 = str1.right(2);
            }else{
            for(int i = 0;i < (2-str1.length());i++)
            {
                str2 = str2 + "0";
            }
            }
            str2 = str2 + str1;
            str = str.right(str.length() - j - 1);
            str1.clear();

            j = str.indexOf("-");
            str1 = str.left(j);
            if(j > 2)
            {
               str1 = str1.right(2);
            }else{
            for(int i = 0;i < (2-str1.length());i++)
            {
                str2 = str2 + "0";
            }
            }
            str2 = str2 + str1;
            str = str.right(str.length() - j - 1);
            str1.clear();

            str2 = str2 + str;

#if 0
            j = str.indexOf("-");
            str1 = str.left(j);
            for(int i = 0;i < (2-str1.length());i++)
            {
                str2 = str2 + "0";
            }
            str2 = str2 + str1;
            str = str.right(str.length() - j - 1);
            str1.clear();

            j = str.indexOf("-");
            str1 = str.left(j);
            for(int i = 0;i < (2-str1.length());i++)
            {
                str2 = str2 + "0";
            }
            str2 = str2 + str1;
            str = str.right(str.length() - j - 1);
            str1.clear();

            j = str.indexOf("-");
            str1 = str.left(j);
            for(int i = 0;i < (2-str1.length());i++)
            {
                str2 = str2 + "0";
            }
            str2 = str2 + str1;
            str = str.right(str.length() - j - 1);
            str1.clear();

            str2 = str2 + str;
#endif
        }
        break;
    case 77:
        if(str!="--")
        {
            t = t + 1;
            str2.clear();
            int j = str.indexOf("-");
            if(j == 0)
            {
                QMessageBox::information( this, tr("提示"),tr("地址输入不正确"));
                return;
            }
            str1 = str.left(j);
            for(int i = 0;i < (4-str1.length());i++)
            {
                str2 = str2 + "0";
            }
            str2 = str2 + str1;
            str = str.right(str.length() - j - 1);
            str1.clear();

            j = str.indexOf("-");
            str1 = str.left(j);
            if(j > 2)
            {
               str1 = str1.right(2);
            }else{
            for(int i = 0;i < (2-str1.length());i++)
            {
                str2 = str2 + "0";
            }
            }
            str2 = str2 + str1;
            str = str.right(str.length() - j - 1);
            str1.clear();

            str2 = str2 + str;
#if 0
            j = str.indexOf("-");
            str1 = str.left(j);
            for(int i = 0;i < (2-str1.length());i++)
            {
                str2 = str2 + "0";
            }
            str2 = str2 + str1;
            str = str.right(str.length() - j - 1);
            str1.clear();

            str2 = str2 + str;
#endif
        }

        break;
    case 87:
    case 90:
#if 0
        if(str!="")
        {
        for(int i = 0;i < (4-str.length());i++)
        {
            str2 = str2 + "0";
        }
        str2 = str2 +str;
        }
#endif
        if(str!="-")
        {
        t = t + 1;
        str2.clear();
        int j = str.indexOf("-");
        if(j == 0)
        {
            QMessageBox::information( this, tr("提示"),tr("地址输入不正确"));
            return;
        }
        str1 = str.left(j);
        for(int i = 0;i < (4-str1.length());i++)
        {
            str2 = str2 + "0";
        }
        str2 = str2 + str1;
        str = str.right(str.length() - j - 1);

        str2 = str2 + str;
        }
        break;
    default :
        break;
    }
    str2 = str2 + ui->QlineAddr_2->text();
    memcpy(buf+23,str2.toAscii().data(),str2.length());

    str.clear();
    str = ui->QlineMac->text();
    if(str!=":::")
    {
       t = t + 2;
       buf[42] = 0;
       buf[43] = 136;
       for(int i = 0;i < 3;i++)
       {
           int j = str.indexOf(":");
           str1 = str.left(j);
           bool ok;
           buf[44+i] = str1.toInt(&ok,16);
           str = str.right(str.length() - j - 1);
           if(i == 2)
           {
               buf[45+i] = str.toInt(&ok,16);
           }
       }
    }
    str.clear();
    str = ui->QlineIp->text();
    if(str!="...")
    {
       t = t + 4;
       for(int i = 0;i < 3;i++)
       {
           int j = str.indexOf(".");
           str1 = str.left(j);
           buf[48+i] = str1.toInt();
           str = str.right(str.length() - j - 1);
           if(i == 2)
           {
               buf[49+i] = str.toInt();
           }
       }
    }

    str.clear();
    str = ui->QlineSubnet->text();
    if(str!="...")
    {
       t = t + 8;
       for(int i = 0;i < 3;i++)
       {
           int j = str.indexOf(".");
           str1 = str.left(j);
           buf[52+i] = str1.toInt();
           str = str.right(str.length() - j - 1);
           if(i == 2)
           {
               buf[53+i] = str.toInt();
           }
       }
    }

    str.clear();
    str = ui->QlineGateway->text();
    if(str!="...")
    {
       t = t + 16;
       for(int i = 0;i < 3;i++)
       {
           int j = str.indexOf(".");
           str1 = str.left(j);
           buf[56+i] = str1.toInt();
           str = str.right(str.length() - j - 1);
           if(i == 2)
           {
               buf[57+i] = str.toInt();
           }
       }
    }

    str.clear();
    str = ui->QlineServerIp->text();
    if(str!="...")
    {
       t = t + 32;
       for(int i = 0;i < 3;i++)
       {
           int j = str.indexOf(".");
           str1 = str.left(j);
           buf[60+i] = str1.toInt();
           str = str.right(str.length() - j - 1);
           if(i == 2)
           {
               buf[61+i] = str.toInt();
           }
       }
    }
    buf[21] = t;
    switch( QMessageBox::question( this, tr("提示"),tr("确定要修改?"),tr("Yes"), tr("No"),0, 1 ) )
  {
  case 0:
    emit SigWrite(buf,m_strIp);
    m_nTimerId1 = startTimer(5000);
    break;
  case 1:
      break;
  default :
      break;
  }
}
