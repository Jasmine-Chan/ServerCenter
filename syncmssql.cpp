#include "syncmssql.h"
#include "STRUCT.h"
#include <QDebug>

//室内设备
CSYNCMSSQLTENEMENT::CSYNCMSSQLTENEMENT(QObject *parent) :
    QThread(parent)
{

}

CSYNCMSSQLTENEMENT::~CSYNCMSSQLTENEMENT()
{

}

void CSYNCMSSQLTENEMENT::run()
{
    usleep(1*NTIME);    //NTIME 10  休眠10毫秒
    while(1)
    {
        QString strSQL = "select * from tenement where Tenement_UpStatus != 0";
        QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
        QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
        QSqlQuery query(db),myquery(db),query1(db1);
        QString strIntraAddr,strType;
        if(query.exec(strSQL))
        {
            if(query.numRowsAffected () > 0)
            {
                while(query.next())
                {
                    int nStatu = query.value(12).toInt();   //Tenement_UpStatus
                    QString strSQL1,mySQL;
                    switch(nStatu)
                    {
                        case 1:
                            strSQL1 = "insert into Tenement (Tenement.Tenement_addr,"
                                    "Tenement.Tenement_addr_explain,Tenement.Tenement_name,Tenement.Tenement_phone1,"
                                    "Tenement.Tenement_phone2,Tenement.Tenement_mac_addr,Tenement.Tenement_ip_addr,"
                                    "Tenement.Tenement_type,Tenement.Tenement_fence_statu,Tenement.Tenement_intra_addr,Tenement.Tenement_Online,Tenement.Tenement_Areacode)"
                                    "values ('"+query.value(1).toString()+"','"+query.value(2).toString()+"','"+query.value(3).toString()+"',"
                                    "'"+query.value(4).toString()+"','"+query.value(5).toString()+"','"+query.value(6).toString()+"',"
                                    "'"+query.value(7).toString()+"','"+query.value(8).toString()+"','"+query.value(9).toString()+"',"
                                    "'"+query.value(10).toString()+"','"+query.value(11).toString()+"','"+CLIST::nIDtoString()+"')";
                            break;
                        case 2:
                            strSQL1 = "UPDATE Tenement set Tenement_addr = '"+query.value(1).toString()+"',Tenement_addr_explain = '"+query.value(2).toString()+"',Tenement_name = '"+query.value(3).toString()+"',"
                                    "Tenement_phone1 = '"+query.value(4).toString()+"',Tenement_phone2 = '"+query.value(5).toString()+"',Tenement_ip_addr='"+query.value(7).toString()+"',Tenement_intra_addr = '"+query.value(10).toString()+"', "
                                    "Tenement_type='"+query.value(8).toString()+"',Tenement_fence_statu='"+query.value(9).toString()+"',Tenement_Online='"+query.value(11).toString()+"' "
                                    "WHERE  Tenement_mac_addr = '"+query.value(6).toString()+"' AND Tenement_Areacode = '"+CLIST::nIDtoString()+"'";
                            break;
                        case 3:
                            strSQL1 = "";
                            break;
                        default:
                            break;
                    }
                    bool b = query1.exec(strSQL1);  //更新远程数据库
                    qDebug()<<"tenement:"<<strSQL1<<b;
                    if(b)           //更新完远程数据库之后，将本地数据库的状态字段重置
                    {
                        mySQL = "UPDATE tenement set Tenement_UpStatus = '"+QString::number(0)+"' WHERE Tenement_intra_addr = '"+query.value(10).toString()+"' AND Tenement_mac_addr = '"+query.value(6).toString()+"'";
                        myquery.exec(mySQL);
                        usleep(1);      //休眠1毫秒
                    }
                }
            }
            else
            {
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                CCONNECTIONPOOLMSSQL::closeConnection(db1);
                sleep(10*NTIME);    //休眠100秒
            }
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(db1);
    }
}

//中间设备
CSYNCMSSQLMIDDLE::CSYNCMSSQLMIDDLE(QObject *parent) :
    QThread(parent)
{

}

CSYNCMSSQLMIDDLE::~CSYNCMSSQLMIDDLE()
{

}

void CSYNCMSSQLMIDDLE::run()
{
    usleep(2*NTIME);        //休眠20毫秒
    while(1)
    {
        QString strSQL = "select * from middle where Middle_UpStatus != 0";
        QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
        QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
        QSqlQuery query(db),myquery(db),query1(db1);
        if(query.exec(strSQL))
        {
            if(query.numRowsAffected () > 0)
            {
                while(query.next())
                {
                    int nStatu = query.value(8).toInt();    //Middle_UpStatus
                    QString strSQL1,mySQL;
                    switch(nStatu)
                    {
                        case 1:
                            strSQL1 = "INSERT INTO Tenement (Tenement.Tenement_addr,Tenement.Tenement_addr_explain,Tenement.Tenement_mac_addr,"
                                    "Tenement.Tenement_ip_addr,Tenement.Tenement_type,Tenement.Tenement_intra_addr,"
                                    "Tenement.Tenement_Online,Tenement.Tenement_Areacode) VALUES ('"+query.value(1).toString()+"','"+query.value(2).toString()+"',"
                                    "'"+query.value(3).toString()+"','"+query.value(4).toString()+"','"+query.value(5).toString()+"','"+query.value(6).toString()+"',"
                                    "'"+query.value(9).toString()+"','"+CLIST::nIDtoString()+"')";
                            break;
                        case 2:
                            strSQL1 = "UPDATE Tenement SET Tenement_addr = '"+query.value(1).toString()+"',Tenement_addr_explain = '"+query.value(2).toString()+"',"
                                    "Tenement_ip_addr = '"+query.value(4).toString()+"',Tenement_type = '"+query.value(5).toString()+"',Tenement_intra_addr = '"+query.value(6).toString()+"',"
                                    "Tenement_Online = '"+query.value(9).toString()+"' WHERE Tenement_mac_addr = '"+query.value(3).toString()+"'   AND Tenement_Areacode = '"+CLIST::nIDtoString()+"'";
                            break;
                        case 3:
                            strSQL1 = "";
                            break;
                        default:        //0
                            break;
                    }
                    qDebug()<<"Middle:"<<strSQL1;
                    if(query1.exec(strSQL1))    //更新远程数据库
                    {
                        mySQL = "UPDATE middle SET Middle_UpStatus = '"+QString::number(0)+"' WHERE Middle_mac_addr = '"+query.value(3).toString()+"' AND Middle_intra_addr = '"+query.value(6).toString()+"'";
                        myquery.exec(mySQL);
                        usleep(1);      //休眠1毫秒
                    }

                }
            }
            else
            {
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                CCONNECTIONPOOLMSSQL::closeConnection(db1);
                sleep(10*NTIME);
            }
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(db1);
    }
}

//报警记录
CSYNCMSSQLALARMRECORD::CSYNCMSSQLALARMRECORD(QObject *parent) :
    QThread(parent)
{

}

CSYNCMSSQLALARMRECORD::~CSYNCMSSQLALARMRECORD()
{

}
void CSYNCMSSQLALARMRECORD::run()
{
    usleep(3*NTIME);    //休眠30毫秒
    while(1)
    {
        QString strSQL = "select *from alarm where Alarm_UpStatus != 0";
        QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
        QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
        QSqlQuery query(db),myquery(db),query1(db1);
        if(query.exec(strSQL))
        {
            if(query.numRowsAffected () > 0)
            {
                while(query.next())
                {
                    int nStatu = query.value(16).toInt();   //Alarm_UpStatus
                    QString strSQL1,mySQL;
                    switch(nStatu)
                    {
                        case 1:
                            strSQL1 = "INSERT INTO Alarm (Alarm.Alarm_mac_addr,Alarm.Alarm_ip_addr,Alarm.Alarm_fence,Alarm.Alarm_type,Alarm.Alarm_stime,"
                                    "Alarm.Alarm_etime,Alarm.Alarm_dtime,Alarm.Alarm_status,Alarm.Alarm_intra_addr,Alarm.Alarm_deal_human,Alarm.Alarm_deal_message,"
                                    "Alarm.Alarm_name,Alarm.Alarm_phone,Alarm.Alarm_addr,Alarm.Alarm_device_type,Alarm.Alarm_Areacode) VALUES ("
                                    "'"+query.value(1).toString()+"','"+query.value(2).toString()+"','"+query.value(3).toString()+"','"+query.value(4).toString()+"','"+query.value(5).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"',"
                                    "'"+query.value(6).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"','"+query.value(7).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"','"+query.value(8).toString()+"','"+query.value(9).toString()+"','"+query.value(10).toString()+"',"
                                    "'"+query.value(11).toString()+"','"+query.value(12).toString()+"','"+query.value(13).toString()+"','"+query.value(14).toString()+"','"+query.value(15).toString()+"','"+CLIST::nIDtoString()+"')";
                            break;
                        case 2:
                            strSQL1 = "UPDATE Alarm SET Alarm_ip_addr = '"+query.value(2).toString()+"',Alarm_fence = '"+query.value(3).toString()+"',Alarm_type = '"+query.value(4).toString()+"',"
                                    "Alarm_etime = '"+query.value(6).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"',Alarm_dtime = '"+query.value(7).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"',Alarm_status = '"+query.value(8).toString()+"',Alarm_deal_human = '"+query.value(10).toString()+"',Alarm_deal_message = '"+query.value(11).toString()+"',"
                                    "Alarm_phone = '"+query.value(13).toString()+"',Alarm_name = '"+query.value(12).toString()+"',Alarm_addr = '"+query.value(14).toString()+"',Alarm_device_type = '"+query.value(15).toString()+"'  "
                                    "WHERE Alarm_mac_addr = '"+query.value(1).toString()+"' AND Alarm_intra_addr = '"+query.value(9).toString()+"' AND Alarm_stime = '"+query.value(5).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"' AND Alarm_Areacode ='"+CLIST::nIDtoString()+"'";
                            break;
                        case 3:
                            strSQL1 = "";
                            break;
                        default:
                            break;
                    }
                    qDebug()<<"Alarm:"<<strSQL1;
                    if(query1.exec(strSQL1))
                    {
                        mySQL = "UPDATE alarm SET Alarm_UpStatus = '"+QString::number(0)+"' WHERE Alarm_mac_addr = '"+query.value(1).toString()+"' AND Alarm_intra_addr = '"+query.value(9).toString()+"'";
                        myquery.exec(mySQL);
                        usleep(1);
                    }
                }
            }
            else
            {
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                CCONNECTIONPOOLMSSQL::closeConnection(db1);
                sleep(10*NTIME);
            }
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(db1);
    }
}

//报修记录
CSYNCMSSQLREPAIRSRECORD::CSYNCMSSQLREPAIRSRECORD(QObject *parent) :
    QThread(parent)
{

}

CSYNCMSSQLREPAIRSRECORD::~CSYNCMSSQLREPAIRSRECORD()
{

}

void CSYNCMSSQLREPAIRSRECORD::run()
{
    usleep(4*NTIME);        //休眠40毫秒
    while(1)
    {
        QString strSQL = "select *from repairs_record where Repairs_Record_UpStatus != 0";
        QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
        QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
        QSqlQuery query(db),myquery(db),query1(db1);
        if(query.exec(strSQL))
        {
            if(query.numRowsAffected () > 0)
            {
                while(query.next())
                {
                    int nStatu = query.value(10).toInt();       //Repairs_Record_UpStatus
                    QString strSQL1,mySQL;
                    switch(nStatu)
                    {
                        case 1:
                            strSQL1 = "INSERT INTO Repairs_Record (Repairs_Record.Repairs_Record_addr,Repairs_Record.Repairs_Record_type,Repairs_Record.Repairs_Record_stime,Repairs_Record.Repairs_Record_etime,Repairs_Record.Repairs_Record_dtime,"
                                    "Repairs_Record.Repairs_Record_deal_human,Repairs_Record.Repairs_Record_message,Repairs_Record.Repairs_Record_intra_addr,Repairs_Record.Repairs_Record_device_type,Repairs_Record.Repairs_Record_Areacode) VALUES ("
                                    "'"+query.value(1).toString()+"','"+query.value(2).toString()+"','"+query.value(3).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"','"+query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"','"+query.value(5).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"',"
                                    "'"+query.value(6).toString()+"','"+query.value(7).toString()+"','"+query.value(8).toString()+"','"+query.value(9).toString()+"','"+CLIST::nIDtoString()+"')";
                            break;
                        case 2:
                            strSQL1 = "UPDATE repairs_record SET Repairs_Record_addr = '"+query.value(1).toString()+"',Repairs_Record_etime = '"+query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"',"
                                    "Repairs_Record_dtime = '"+query.value(5).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"',Repairs_Record_deal_human = '"+query.value(6).toString()+"',Repairs_Record_message = '"+query.value(7).toString()+"',Repairs_Record_device_type = '"+query.value(9).toString()+"'"
                                    " WHERE Repairs_Record_stime = '"+query.value(3).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"' and Repairs_Record_type = '"+query.value(2).toString()+"' AND Repairs_Record_intra_addr = '"+query.value(8).toString()+"' AND Repairs_Record_Areacode = '"+CLIST::nIDtoString()+"'";
                            break;
                        case 3:
                            strSQL1 = "";
                            break;
                        default:
                            break;
                    }
                    qDebug()<<"repairs_record:"<<strSQL1;
                    if(query1.exec(strSQL1))
                    {
                        mySQL = "UPDATE repairs_record SET Repairs_Record_UpStatus = '"+QString::number(0)+"' WHERE Repairs_Record_stime = '"+query.value(3).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"' and Repairs_Record_type = '"+query.value(2).toString()+"' AND Repairs_Record_intra_addr = '"+query.value(8).toString()+"'";
                        myquery.exec(mySQL);
                        usleep(1);
                    }
                }
            }
            else
            {
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                CCONNECTIONPOOLMSSQL::closeConnection(db1);
                sleep(10*NTIME);
            }
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(db1);
    }
}

//刷卡记录
CSYNCMSSQLCARDRECORD::CSYNCMSSQLCARDRECORD(QObject *parent) :
    QThread(parent)
{

}

CSYNCMSSQLCARDRECORD::~CSYNCMSSQLCARDRECORD()
{

}
void CSYNCMSSQLCARDRECORD::run()
{
    usleep(5*NTIME);        //休眠50毫秒
    while(1)
    {
        QString strSQL = "select * from card_record where Card_Record_UpStatus != 0";
        QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
        QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
        QSqlQuery query(db),myquery(db),query1(db1);
        if(query.exec(strSQL))
        {
            if(query.numRowsAffected () > 0)
            {
                while(query.next())
                {
                    int nStatu = query.value(7).toInt();
                    QString strSQL1,mySQL;
                    switch(nStatu)
                    {
                        case 1:
                            strSQL1 = "insert into Card_Record (Card_Record_Addr,Card_Record_Card_Num,Card_Record_Device_Type,Card_Record_Time,Card_addr,Card_Record_Intra_Addr)"
                                    "VALUES ('"+query.value(1).toString()+"','"+query.value(2).toString()+"','"+query.value(3).toString()+"','"+query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"','"+query.value(5).toString()+"','"+query.value(6).toString()+"')";
                            break;
                        case 2:
                            strSQL1 = "UPDATE card_record SET Card_Record_addr = '"+query.value(1).toString()+"',Card_Record_card_num = '"+query.value(2).toString()+"',Card_Record_device_type = '"+query.value(3).toString()+"',Card_addr = '"+query.value(5).toString()+"',Card_Record_UpStatus = '"+QString::number(0)+"' "
                                    "WHERE Card_Record_time = '"+query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"' AND Card_Record_intra_addr = '"+query.value(6).toString()+"'";
                            break;
                        case 3:
                            strSQL1 = "";
                            break;
                        default:
                            break;
                    }
                    qDebug()<<"card_record:"<<strSQL1;
                    if(query1.exec(strSQL1))
                    {
                        mySQL = "UPDATE card_record SET Card_Record_UpStatus = '"+QString::number(0)+"' WHERE Card_Record_card_num = '"+query.value(2).toString()+"' and Card_Record_time = '"+query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")+"' AND Card_intra_addr = '"+query.value(6).toString()+"'";
                        qDebug()<<strSQL1<<"card_record"<<mySQL;
                        myquery.exec(mySQL);
                        usleep(1);
                    }
                }
            }
            else
            {
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                CCONNECTIONPOOLMSSQL::closeConnection(db1);
                sleep(10*NTIME);
            }
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(db1);
    }
}

//IC卡
CSYNCMSSQLICCARD::CSYNCMSSQLICCARD(QObject *parent) :
    QThread(parent)
{

}

CSYNCMSSQLICCARD::~CSYNCMSSQLICCARD()
{

}
void CSYNCMSSQLICCARD::run()
{
    usleep(6*NTIME);        //休眠60毫秒
    while(1)
    {
        QString strSQL = "select * from ic_card where IC_Card_UpStatus != 0";
        QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
        QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
        QSqlQuery query(db),myquery(db),query1(db1);
        if(query.exec(strSQL))
        {
            if(query.numRowsAffected () > 0)
            {
                while(query.next())
                {
                    int nStatu = query.value(4).toInt();
                    QString strSQL1,mySQL;
                    switch(nStatu)
                    {
                        case 1:
                            strSQL1 = "INSERT into IC_Card (IC_Card.IC_Card_addr,IC_Card.IC_Card_num,IC_Card.IC_Card_time,IC_Card.IC_Card_Areacode) VALUES ("
                                    "'"+query.value(1).toString()+"','"+query.value(2).toString()+"','"+query.value(3).toDateTime().toString("yyyy-MM-dd")+"','"+CLIST::nIDtoString()+"')";
                            break;
                        case 2:
                            strSQL1 = "UPDATE IC_Card SET IC_Card_addr = '"+query.value(1).toString()+"',ic_card.IC_Card_time = '"+query.value(3).toString()+"',IC_Card_UpStatus = '"+QString::number(0)+"' "
                                    "WHERE IC_Card_num = '"+query.value(2).toString()+"' AND IC_Card_Areacode = '"+CLIST::nIDtoString()+"'";
                            break;
                        case 3:
                            strSQL1 = "";
                            break;
                        default:
                            break;
                    }
                    qDebug()<<"IC_Card:"<<strSQL1;
                    if(query1.exec(strSQL1))
                    {
                        mySQL = "UPDATE ic_card SET IC_Card_UpStatus = '"+QString::number(0)+"' WHERE IC_Card_num = '"+query.value(2).toString()+"'";
                        myquery.exec(mySQL);
                        usleep(1);
                    }
                }
            }
            else
            {
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                CCONNECTIONPOOLMSSQL::closeConnection(db1);
                sleep(10*NTIME);
            }
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(db1);
    }
}

//删除卡
CSYNCMSSQLDELETECARD::CSYNCMSSQLDELETECARD(QObject *parent) :
    QThread(parent)
{

}

CSYNCMSSQLDELETECARD::~CSYNCMSSQLDELETECARD()
{

}
void CSYNCMSSQLDELETECARD::run()
{
    usleep(3*30);       //休眠90毫秒
    while(1)
    {
        QString strSQL = "select * from Upiccard ";
        QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
        QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
        QSqlQuery query(db),myquery(db),query1(db1);
        if(query.exec(strSQL))
        {
            if(query.numRowsAffected () > 0)
            {
                while(query.next())
                {
                    QString strSQL1,mySQL;
                    strSQL1 = "delete from IC_Card where IC_Card_num = '"+query.value(1).toString()+"'";
                    if(query1.exec(strSQL1))        //删除远程数据
                    {
                        mySQL = "delete from Upiccard where UpIcCard_num = '"+query.value(1).toString()+"'";
                        myquery.exec(mySQL);
                        qDebug()<<strSQL1<<"ic_card"<<mySQL;
                        usleep(1);
                    }
                }
            }
            else
            {
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                CCONNECTIONPOOLMSSQL::closeConnection(db1);
                sleep(10*NTIME);
            }
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(db1);
    }
}

//删除设备
CSYNCMSSQLDELETEDEVICE::CSYNCMSSQLDELETEDEVICE(QObject *parent) :
    QThread(parent)
{

}

CSYNCMSSQLDELETEDEVICE::~CSYNCMSSQLDELETEDEVICE()
{

}
void CSYNCMSSQLDELETEDEVICE::run()
{
    usleep(1*30);       //休眠30毫秒
    while(1)
    {
        QString strSQL = "select *from UpDevice ";
        QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
        QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
        QSqlQuery query(db),myquery(db),query1(db1);
        if(query.exec(strSQL))
        {
            if(query.numRowsAffected () > 0)
            {
                while(query.next())
                {
                    QString strSQL1,mySQL;
                    strSQL1 = "delete from Tenement where Tenement_mac_addr = '"+query.value(1).toString()+"' and Tenement_Areacode ='"+CLIST::nIDtoString()+"'";
                    if(query1.exec(strSQL1))    //删除远程数据
                    {
                        mySQL = "delete from UpDevice where UpDevice_mac_addr = '"+query.value(1).toString()+"'";
                        myquery.exec(mySQL);
                        qDebug()<<strSQL1<<"device"<<mySQL;
                        usleep(1);
                    }
                }
            }
            else
            {
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                CCONNECTIONPOOLMSSQL::closeConnection(db1);
                sleep(10*NTIME);
            }
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(db1);
    }
}
