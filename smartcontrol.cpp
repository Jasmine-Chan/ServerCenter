#include "smartcontrol.h"
#include "ui_smartcontrol.h"
#include <QDebug>
CSMARTCONTROL::CSMARTCONTROL(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSMARTCONTROL)
{
    setWindowModality(Qt::ApplicationModal);
    ui->setupUi(this);
    m_pAddr = new char[20];
    for(int i = 0;i<6;i++)
    {m_flag[i] = 0;}
    ui->Qpush1->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_gray.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_led_yellow.png);}");
    ui->Qpush2->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_gray.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_led_yellow.png);}");
    ui->Qpush3->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_gray.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_led_yellow.png);}");
    ui->Qpush4->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_gray.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_led_yellow.png);}");
    ui->Qpush5->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_curtain_control_close_red.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_curtain_control_open_red.png);}");
    ui->Qpush6->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_air_condition_off.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_air_condition_on.png);}");

}

CSMARTCONTROL::~CSMARTCONTROL()
{
    delete ui;
}


void CSMARTCONTROL::_Updata(QString strIp,QString strAddr,QString strType)
{
    memset(m_pAddr,0,20);
    QString Addr;
    m_strIp.clear();
    m_strAddr.clear();
    m_strIp = strIp;
    m_strAddr = strAddr;
    Addr.clear();
    Addr = strAddr.left(3);
    strAddr = strAddr.right(strAddr.length() - 4);
    Addr = Addr + strAddr.left(3);
    strAddr = strAddr.right(strAddr.length() - 4);
    Addr = Addr + strAddr.left(3);
    strAddr = strAddr.right(strAddr.length() - 4);
    Addr = Addr + strAddr.left(3);
    strAddr = strAddr.right(strAddr.length() - 4);
    Addr = Addr + strAddr.left(3);

    if(strType == "室内机")
    {
        m_pAddr[0] = 83;
    }else     if(strType == "别墅室内机")
    {
        m_pAddr[0] = 66;
    }else     if(strType == "门口机")
    {
        m_pAddr[0] = 77;
    }else     if(strType == "围墙机")
    {
        m_pAddr[0] = 87;
    }else     if(strType == "中心机")
    {
        m_pAddr[0] = 90;
    }else    if(strType == "二次门口机")
    {
        m_pAddr[0] = 72;
    }
    QByteArray byte;
    byte = Addr.toAscii();

    memcpy(m_pAddr+1,byte.data(),byte.size());
}

void CSMARTCONTROL::on_Qpush1_clicked()
{
    if(m_flag[0] == 0){
    m_flag[0] = 1;
    ui->Qpush1->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_yellow.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_led_gray.png);}");
    }else{
        m_flag[0] = 0;
        ui->Qpush1->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_gray.png);}"
                                  "QPushButton:hover{background-image:url(:/pic/smart_home_led_yellow.png);}");
    }
    _Sendto(0);
}

void CSMARTCONTROL::on_Qpush2_clicked()
{
    if(m_flag[1] == 0){
    m_flag[1] = 1;
    ui->Qpush2->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_yellow.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_led_gray.png);}");
    }else{
        m_flag[1] = 0;
        ui->Qpush2->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_gray.png);}"
                                  "QPushButton:hover{background-image:url(:/pic/smart_home_led_yellow.png);}");
    }
    _Sendto(1);
}

void CSMARTCONTROL::on_Qpush3_clicked()
{
    if(m_flag[2] == 0){
    m_flag[2] = 1;
    ui->Qpush3->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_yellow.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_led_gray.png);}");
    }else{
        m_flag[2] = 0;
        ui->Qpush3->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_gray.png);}"
                                  "QPushButton:hover{background-image:url(:/pic/smart_home_led_yellow.png);}");
    }
    _Sendto(2);
}

void CSMARTCONTROL::on_Qpush4_clicked()
{
    if(m_flag[3] == 0){
    m_flag[3] = 1;
    ui->Qpush4->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_yellow.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_led_gray.png);}");
    }else{
        m_flag[3] = 0;
        ui->Qpush4->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_led_gray.png);}"
                                  "QPushButton:hover{background-image:url(:/pic/smart_home_led_yellow.png);}");
    }
    _Sendto(3);
}


 void CSMARTCONTROL::_Sendto(int flag)
 {
     char buf[29] = {0};
     memcpy(buf,m_pAddr,20);
     buf[20] = 1;
     switch(flag)
     {
     case 4:
         buf[21] = 3;//窗帘3 空调4
         break;
     case 5:
         buf[21] = 4;//窗帘3 空调4
         break;
     default :
         buf[21] = 1;//窗帘3 空调4
         break;
     }


     buf[22] = 0;
     buf[23] = flag + 1;
     buf[24] = 1;
     buf[25] = 1;
     buf[26] = 1;
     buf[28] = m_flag[flag];
     buf[27] = 0;
     emit SigSendLamp(buf,m_strIp);
 }

void CSMARTCONTROL::on_Qpush5_clicked()
{
    if(m_flag[4] == 0){
    m_flag[4] = 1;
    ui->Qpush5->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_curtain_control_open_red.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_curtain_control_close_red.png);}");
   ui->label->setStyleSheet("QLabel{background-image:url(:/pic/smart_home_curtain_open_2.png);}");
    }else{
        m_flag[4] = 0;
        ui->Qpush5->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_curtain_control_close_red.png);}"
                                  "QPushButton:hover{background-image:url(:/pic/smart_home_curtain_control_open_red.png);}");
     ui->label->setStyleSheet("QLabel{background-image:url(:/pic/smart_home_curtain_open_1.png);}");
    }
    _Sendto(4);
}

void CSMARTCONTROL::on_Qpush6_clicked()
{
    if(m_flag[5] == 0){
    m_flag[5] = 1;
    ui->Qpush6->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_air_condition_on.png);}"
                              "QPushButton:hover{background-image:url(:/pic/smart_home_air_condition_off.png);}");
    }else{
        m_flag[5] = 0;
        ui->Qpush6->setStyleSheet("QPushButton{background-image:url(:/pic/smart_home_air_condition_off.png);}"
                                  "QPushButton:hover{background-image:url(:/pic/smart_home_air_condition_on.png);}");
}
    _Sendto(5);
}
