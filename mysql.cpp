#include "mysql.h"
//#include "log.h"
#include "loading.h"

CMYSQL::CMYSQL()
{
}

/*========================================================================
        Name:		打开数据库并连接。
    ----------------------------------------------------------
        returns:	返回数据库连接状况。
    ----------------------------------------------------------
        Remarks:
==========================================================================*/
bool CMYSQL::_OpenMySql()
{
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    int i = query.exec("SELECT 1");
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    if(i)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CMYSQL::_OpenODBC()
{

    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    int i = query.exec("SELECT 1");
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    if(i)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CMYSQL::_IsOpen()
{
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    bool bdb = db.isOpen();
    CCONNECTIONPOOLMYSQL::closeConnection(db);

    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    bool bdb1 = db1.isOpen();

    CCONNECTIONPOOLMSSQL::closeConnection(db1);

    qDebug()<<bdb<<bdb1;
    return bdb;
}
/*========================================================================
        Name:		查询城市的编码。
    ----------------------------------------------------------
        returns:	返回城市的编码。
    ----------------------------------------------------------
        Remarks:
    ----------------------------------------------------------
        Params:         strCity 城市的名称
==========================================================================*/
QString CMYSQL::_GetCityNum(QString strCity)
{
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);

    QString SQL = "Select * From City where City_name = '"+strCity+"'";
    query.clear();
    QString strCityNum;
    strCityNum.clear();
    if(query.exec(SQL))
    {
        if(query.first())
        {
            strCityNum = query.value(2).toString();
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return strCityNum;
}

/*========================================================================
        Name:		获取所有的用户名。
    ----------------------------------------------------------
        returns:	返回获取的用户的成功。
    ----------------------------------------------------------
        Remarks:
    ----------------------------------------------------------
        Params:         用户名链表容器
==========================================================================*/
bool CMYSQL::_GetAllUser(QList<QString> *T)
{
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    QString SQL = "select User_user from user";
     if(query.exec(SQL))
     {
         int i = 0;
         while(query.next())
         {
             (*T).insert(i,QString(query.value(0).toByteArray().data()));
             i++;
         }
         CCONNECTIONPOOLMYSQL::closeConnection(db);
         return true;
     }
     else
     {
         CCONNECTIONPOOLMYSQL::closeConnection(db);
         return false;
     }
}

/*========================================================================
        Name:		判定用户名密码是否正确并查询城市的编码。
    ----------------------------------------------------------
        returns:	返回城市的编码。
    ----------------------------------------------------------
        Remarks:        查询用户名密码是否正确，正确则查询城市对应的编码
    ----------------------------------------------------------
        Params:         sUser 用户的结构体
==========================================================================*/
bool CMYSQL::_GetUser(SUser &sUser)
{
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    QString SQL = "Select * From User where User_user = '"+sUser.strUser+"' and User_pass = MD5('"+sUser.strPass+"')";
    if(query.exec(SQL))
    {
        if(query.numRowsAffected () > 0)
        {
            query.next();
            sUser.strName = query.value(3).toString();
            sUser.nGrade = query.value(4).toUInt();
            SQL.clear();
            SQL = "Select * From City where City_name = '"+sUser.strCity+"'";
            query.clear();
            if(query.exec(SQL))
            {
                if(query.first())
                {
                    sUser.strCityNum = query.value(2).toString();
                }
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                return true;
            }
            else
            {
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                return false;
            }
        }
        else
        {
            CCONNECTIONPOOLMYSQL::closeConnection(db);
            return false;
        }
    }
    else
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        return false;
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return false;
}

/*========================================================================
        Name:		更新中间设备表。
    ----------------------------------------------------------
        returns:	返回保存是否成功。
    ----------------------------------------------------------
        Remarks:
    ----------------------------------------------------------
        Params:         nMark 操作类型
                        strUser 用户名
                        strName 用户姓名
==========================================================================*/
bool CMYSQL::_UpdateMiddle(int nMark,SMiddleDevice sMiddleDevice)   //???
{
    QString str = QString(sMiddleDevice.gcAddr);
    QString str1 = QString(sMiddleDevice.gcAddrExplain);
    QString str2 = QString(sMiddleDevice.gcMacAddr);
    QString str3 = QString(sMiddleDevice.gcIpAddr);
    QString str4 = QString(sMiddleDevice.gcType);
    QString str5 = QString(sMiddleDevice.gcIntraAddr);
    QString str6 = "连接";
    QDateTime dateTime = QDateTime::currentDateTime();
    QString str7 = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString SQL = "UPDATE middle SET Middle_addr = '"+str+"',Middle_addr_explain = '"+str1+"',Middle_ip_addr = '"
            +str3+"',Middle_type = '"+str4+"',Middle_intra_addr = '"+str5+"',Middle_Online = '"+QString::number(1)+"'"
            " ,Middle_UpStatus = '"+QString::number(2)+"' where Middle_mac_addr = '"+str2+"'";
    QString SQLN = "UPDATE Tenement SET Tenement_addr = '"+str+"',Tenement_addr_explain = '"+str1+"',Tenement_ip_addr = '"
            +str3+"',Tenement_type = '"+str4+"',Tenement_intra_addr = '"+str5+"',Tenement_Online = '"+QString::number(1)+"'"
            "  where Tenement_mac_addr = '"+str2+"' and Tenement_Areacode = '"+CLIST::nIDtoString()+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase dbn = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    int j = 0;
    if(sMiddleDevice.nState > 0)
    {
        j = 1;
    }
    else
    {
        QString SQL1 = "insert into device (Device_addr,Device_ip_addr,Device_status,Device_time,Device_intra_addr,Device_type) VALUES ('"+str1+"','"+str3+"','"+str6+"','"+str7+"','"+str5+"','"+str4+"')";
        j = query.exec(SQL1);
    }

    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(dbn.open())          //
//        {
            QSqlQuery queryn(dbn);
            if(queryn.exec(SQLN))
            {
                QString strSQL = "UPDATE middle SET Middle_UpStatus = '"+QString::number(0)+"' where Middle_mac_addr = '"+str2+"'";
                query.exec(strSQL);
            }
//        }
    }

    if(bSucceed && j)
    {
        if(nMark == -3)     //更新
        {
            QString str8 = QString(sMiddleDevice.gcOldAddr);
            QString SQL9 = "SELECT count(*) FROM middle WHERE Middle_intra_addr = '"+str8+"'";
            int nCount = 0;
            if(query.exec(SQL9))
            {
                while(query.next())
                {
                    nCount = query.value(0).toInt();
                }
            }
            if(nCount == 0)
            {
                SQL.clear();
                SQL = "alter table ic_card change "+str8+" "+str5+" int(20)";
                if(str4 == "中心机")
                {
                }
                else
                {
                    query.exec(SQL);
                }
            }
            else
            {
                SQL.clear();
                SQL = "alter table ic_card add "+str5+" int default 0";
                if(str4 == "中心机")
                {
                }
                else
                {
                    query.exec(SQL);
                }
            }
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return true;
    }
    else
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return false;
    }
}
//插入中间设备表
bool CMYSQL::_InsertMiddle(SMiddleDevice sMiddleDevice)
{
    QString str = QString(sMiddleDevice.gcAddr);
    QString str1 = QString(sMiddleDevice.gcAddrExplain);
    QString str2 = QString(sMiddleDevice.gcMacAddr);
    QString str3 = QString(sMiddleDevice.gcIpAddr);
    QString str4 = QString(sMiddleDevice.gcType);
    QString str5 = QString(sMiddleDevice.gcIntraAddr);
    QString str6 = "连接";
    QDateTime dateTime = QDateTime::currentDateTime();
    QString str7 = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString SQL = "insert into middle (Middle_addr,Middle_addr_explain,Middle_ip_addr,Middle_mac_addr,Middle_type,Middle_intra_addr,Middle_Online,Middle_UpStatus)VALUES ('"+
            str+"','"+str1+"','"+str3+"','"+str2+"','"+str4+"','"+str5+"','"+QString::number(1)+"','"+QString::number(1)+"')";

    QString SQLN = "insert into Tenement (Tenement_addr,Tenement_addr_explain,Tenement_ip_addr,Tenement_mac_addr,Tenement_type,Tenement_intra_addr,Tenement_Online,Tenement_Areacode)VALUES ('"+
            str+"','"+str1+"','"+str3+"','"+str2+"','"+str4+"','"+str5+"','"+QString::number(1)+"','"+CLIST::nIDtoString()+"')";

    QString SQL1 = "insert into device (Device_addr,Device_ip_addr,Device_status,Device_time,Device_intra_addr,Device_type) VALUES ('"+str1+"','"+str3+"','"+str6+"','"+str7+"','"+str5+"','"+str4+"')";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase dbn = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    QString SQL2 = "alter table ic_card add "+str5+" int default 0";
    if(str4 == "中心机")
    {
    }
    else
    {
        query.exec(SQL2);
    }

    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(dbn.open())      //
//        {
            QSqlQuery queryMs(dbn);
            if(queryMs.exec(SQLN))
            {
                QString strSQL = "UPDATE middle SET Middle_UpStatus = '"+QString::number(0)+"' where Middle_mac_addr = '"+str2+"'";
                query.exec(strSQL);
            }
//        }
    }
    if(bSucceed && query.exec(SQL1))
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return true;
    }
    else
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return false;
    }
}
//修改常用电话
bool CMYSQL::_UpdatePhone(QString strNewDepartment,QString strNewHuman,QString strNewPhone,QString strOldDepartment,QString strOldHuman,QString strOldPhone)
{
    QString SQL = "update useful_phone set Useful_Phone_department = '"+strNewDepartment+"' ,\
            Useful_Phone_human = '"+strNewHuman+"',Useful_Phone_phone = '"+strNewPhone+"' where\
            Useful_Phone_department = '"+strOldDepartment+"' and Useful_Phone_human = '"+strOldHuman+"' and Useful_Phone_phone = '"+strOldPhone+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//插入常用电话
bool CMYSQL::_InsertPhone(QString strDepartment,QString strHuman,QString strPhone)
{
    QString SQL = "insert into useful_phone (Useful_Phone_department,Useful_Phone_human,Useful_Phone_phone) VALUES ('"+strDepartment+"','"+strHuman+"','"+strPhone+"')";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//删除常用电话
bool CMYSQL::_DeletePhone(QString strDepartment,QString strHuman,QString strPhone)
{
    QString SQL = "delete from useful_phone where Useful_Phone_department = '"+strDepartment+"' and \
            Useful_Phone_human = '"+strHuman+"' and Useful_Phone_phone = '"+strPhone+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//修改报修类型
bool CMYSQL::_UpdateRepairs(QString strNewType,QString strOldType)
{
    QString SQL = "update repairs set Repairs_type = '"+strNewType+"'  where Repairs_type = '"+strOldType+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//插入报修类型
bool CMYSQL::_InsertRepairs(QString strType)
{
    QString SQL = "insert into repairs (Repairs_type) VALUES ('"+strType+"')";
    QSqlDatabase db  = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//删除报修类型
bool CMYSQL::_DeleteRepairs(QString strType)
{
    QString SQL = "delete from repairs where Repairs_type = '"+strType+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//插入新用户
bool CMYSQL::_InsertUser(QString strName,QString strUser,QString strPass,int nGrade)
{
    QString SQL = "insert into user (User_name,User_user,User_grade,User_pass) VALUES ('"+strName+"','"+strUser+"','"+QString::number(nGrade)+"',MD5('"+strPass+"'))";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//修改新用户
bool CMYSQL::_UpdateUser(QString strOldUser,QString strName,QString strUser,QString strPass)
{
    QString SQL = "update user set User_name = '"+strName+"' ,User_user = '"+strUser+"',\
            User_pass = MD5('"+strPass+"') where User_user = '"+strOldUser+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//删除用户
bool CMYSQL::_DeleteUser(QString strName,QString strUser)
{
    QString SQL = "delete from user where User_name = '"+strName+"' and User_user = '"+strUser+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//删除报修记录
 bool CMYSQL::_DeleteRepairsRecord(QString strAddr,QString strStimeS,QString strStimeE)
 {
     QString SQL;
     SQL.clear();
     if(strAddr != NULL)    //Ms比My多一个Repairs_Record_Areacode字段
     {
         SQL = "delete from repairs_record where (Repairs_Record_addr like '%"+strAddr+"%' or Repairs_Record_intra_addr like '%"+strAddr+"%') and Repairs_Record_stime > '"+strStimeS+"' and Repairs_Record_stime < '"+strStimeE+"' and  Repairs_Record_etime is NOT NULL OR Repairs_Record_dtime IS NOT NULL";
     }
     else
     {
         SQL = "delete from repairs_record where Repairs_Record_stime > '"+strStimeS+"' and Repairs_Record_stime < '"+strStimeE+"' and  Repairs_Record_etime is NOT NULL OR Repairs_Record_dtime IS NOT NULL";
     }
     qDebug()<<SQL;
     QSqlDatabase Mydb = CCONNECTIONPOOLMYSQL::openConnection();
     QSqlQuery Myquery(Mydb);
     bool MybSucceed = Myquery.exec(SQL);
     CCONNECTIONPOOLMYSQL::closeConnection(Mydb);
     return MybSucceed;
 }
//删除设备日志
bool CMYSQL::_DeleteDeviceRecord(QString strAddr,QString strStimeS,QString strStimeE)
{
    QString SQL;
    SQL.clear();
    if(strAddr != NULL)
    {
        SQL = "delete from device where (Device_addr like '%"+strAddr+"%' or Device_intra_addr like '%"+strAddr+"%') and Device_time > '"+strStimeS+"' and Device_time < '"+strStimeE+"'";
    }
    else
    {
        SQL = "delete from device where Device_time > '"+strStimeS+"' and Device_time < '"+strStimeE+"'";
    }
    qDebug()<<SQL;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//删除系统日志
bool CMYSQL::_DeleteSystemRecord(QString strAddr,QString strStimeS,QString strStimeE)
{
    QString SQL;
    SQL.clear();
    if(strAddr != NULL)
    {
        SQL = "delete from system where System_name like '%"+strAddr+"%' and System_time > '"+strStimeS+"' and System_time < '"+strStimeE+"'";
    }
    else
    {
        SQL = "delete from system where System_time > '"+strStimeS+"' and System_time < '"+strStimeE+"'";
    }
    qDebug()<<SQL;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//删除刷卡日志
bool CMYSQL::_DeleteCardRecord(QString strAddr,QString strStimeS,QString strStimeE)
{
    QString SQL;
    SQL.clear();
    if(strAddr != NULL)
    {
        SQL = "delete from card_record where (Card_Record_addr like '%"+strAddr+"%' or Card_Record_intra_addr like '%"+strAddr+"%') and Card_Record_time > '"+strStimeS+"' and Card_Record_time < '"+strStimeE+"'";
    }
    else
    {
        SQL = "delete from card_record where Card_Record_time > '"+strStimeS+"' and Card_Record_time < '"+strStimeE+"'";
    }
    qDebug()<<SQL;
    QSqlDatabase Mydb = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery Myquery(Mydb);
    bool MybSucceed = Myquery.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(Mydb);
    return MybSucceed;
}

//删除中间设备
bool CMYSQL::_DeleteMiddle(QString strMac,QString strIp,QString strAddr,QString strType)
{
    QString SQL = "delete from  middle  where Middle_addr = '"+strAddr+"' and Middle_mac_addr = '"+strMac+"' and \
            Middle_ip_addr = '"+strIp+"' and Middle_type = '"+strType+"' ";
    QString str;
    str =  strAddr.left(3);
    strAddr = strAddr.right(strAddr.length() - 4);
    str = str + strAddr.left(3);
    strAddr = strAddr.right(strAddr.length() - 4);
    str = str + strAddr.left(3);
    strAddr = strAddr.right(strAddr.length() - 4);
    str = str + strAddr.left(3);
    strAddr = strAddr.right(strAddr.length() - 4);
    str = str + strAddr.left(3);

    if(strType == "门口机")
    {
        str = "M" +str;
    }
    else if(strType == "围墙机")
    {
        str = "W" +str;
    }
    else if(strType == "中心机")
    {
        str = "Z" +str;
    }
    else if(strType == "二次门口机")
    {
        str = "H" +str;
    }

    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)    //删除成功
    {
//        if(db1.open())          //
//        {
            QSqlQuery queryMsSql(db1);
            //MSSQL比MySQL多一个小区编号的字段
            QString strMsSql = "delete from tenement where Tenement_mac_addr = '"+strMac+"' and Tenement_Areacode = '"+CLIST::nIDtoString()+"'";
            bool bMsSucceed = queryMsSql.exec(strMsSql);
            if(!bMsSucceed)
            {
                QString strMySql = "insert into Updevice (UpDevice_mac_addr) values ('"+strMac+"')";
                query.exec(strMySql);
            }
            else
            {
                CCONNECTIONPOOLMSSQL::closeConnection(db1);
                CCONNECTIONPOOLMYSQL::closeConnection (db);
                return true;
            }
//        }
    }

    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    CCONNECTIONPOOLMYSQL::closeConnection (db);
    return false;

//    int nHas = 0;
//    QString SQL2 = "select count(*) from middle where Middle_intra_addr = '"+str+"' ";
//    if(query.exec(SQL2))
//    {
//        while(query.next())
//        {
//            nHas = query.value(0).toInt();
//        }
//    }
//    if(nHas == 0)
//    {
//        QString SQL1 = "alter table ic_card DROP column "+str+"";
//        CCONNECTIONPOOLMYSQL::closeConnection(db);
//        return query.exec(SQL1);
//    }
//    else
//    {
//        CCONNECTIONPOOLMYSQL::closeConnection(db);
//        return true;
//    }
}

QString CMYSQL::_AddrToAddrExplain(QString Addr)
{
    QString str1 = Addr;
    QString str;
    str = str1.left(4) + "栋";
    str1 = str1.right(str1.length() - 4);

    str = str + str1.left(3) + "单元";
    str1 = str1.right(str1.length() - 3);

    str = str + str1.left(3) + "楼";
    str1 = str1.right(str1.length() - 3);

    str = str + str1.left(3) + "房";
    str1 = str1.right(str1.length() - 3);

    str = str + str1.left(3) + "号设备";
    return str;
}

QString CMYSQL::_AddrExplainToAddr(QString AddrExplain)
{
    QString str1 = AddrExplain;
    QString str;
    str = str1.left(4);
    str1 = str1.right(str1.length() - 5);

    str = str + str1.left(3);
    str1 = str1.right(str1.length() - 5);

    str = str + str1.left(3);
    str1 = str1.right(str1.length() - 4);

    str = str + str1.left(3);
    str1 = str1.right(str1.length() - 4);

    str = str + str1.left(3);
    return str;
}

//添加IC卡
bool CMYSQL::_AddIcCard(int m_nLimits,QList<QString> T,QString strCard,QString strAddr,QString strTime)
{
    QString SQL,SQL1;
    if(strAddr == "栋单元楼房号设备")
        strAddr = "";
    switch(m_nLimits)
    {
        case 1:
        {
            SQL = "insert into ic_card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_UpStatus,"+T.at(0)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+QString::number(1)+"',1)";
        }   break;
        case 2:
        {
            SQL = "insert into ic_card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_UpStatus,"+T.at(1)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+QString::number(1)+"',1)";
        }
            break;
        case 3:
        {
            SQL = "insert into ic_card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_UpStatus,"+T.at(0)+","+T.at(1)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+QString::number(1)+"',1,1)";
        }
            break;
        case 4:
        {
            SQL = "insert into ic_card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_UpStatus,"+T.at(2)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+QString::number(1)+"',1)";
        }
            break;
        case 5:
        {
            SQL = "insert into ic_card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_UpStatus,"+T.at(0)+","+T.at(2)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+QString::number(1)+"',1,1)";
        }
            break;
        case 6:
        {
            SQL = "insert into ic_card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_UpStatus,"+T.at(2)+","+T.at(1)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+QString::number(1)+"',1,1)";
        }
            break;
        case 7:
        {
            SQL = "insert into ic_card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_UpStatus,"+T.at(2)+","+T.at(0)+","+T.at(1)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+QString::number(1)+"',1,1,0)";
        }
            break;
        default:
            SQL = "insert into ic_card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_UpStatus) values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+QString::number(1)+"')";
            break;
    }
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
        switch(m_nLimits)
        {
            case 1:
            {
                SQL1 = "insert into IC_Card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_Areacode,"+T.at(0)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+CLIST::nIDtoString()+"',1)";
            }
                break;
            case 2:
            {
                SQL1 = "insert into IC_Card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_Areacode,"+T.at(1)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+CLIST::nIDtoString()+"',1)";
            }
                break;
            case 3:
            {
                SQL1 = "insert into IC_Card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_Areacode,"+T.at(0)+","+T.at(1)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+CLIST::nIDtoString()+"',1,1)";
            }
                break;
            case 4:
            {
                SQL1 = "insert into IC_Card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_Areacode,"+T.at(2)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+CLIST::nIDtoString()+"',1)";
            }
                break;
            case 5:
            {
                SQL1 = "insert into IC_Card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_Areacode,"+T.at(0)+","+T.at(2)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+CLIST::nIDtoString()+"',1,1)";
            }
                break;
            case 6:
            {
                SQL1 = "insert into IC_Card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_Areacode,"+T.at(2)+","+T.at(1)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+CLIST::nIDtoString()+"',1,1)";
            }
                break;
            case 7:
            {
                SQL1 = "insert into IC_Card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_Areacode,"+T.at(2)+","+T.at(0)+","+T.at(1)+") values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+CLIST::nIDtoString()+"',1,1,0)";
            }
                break;
            default:
                SQL1 = "insert into IC_Card (IC_Card_addr,IC_Card_num,IC_Card_time,IC_Card_Areacode) values ('"+strAddr+"','00:"+strCard+"','"+strTime+"','"+CLIST::nIDtoString()+"')";
                break;
        }
//        if(db1.open())          //
//        {
            QSqlQuery query1(db1);
            if(query1.exec(SQL1))
            {
                QString strSQL = "UPDATE ic_card SET IC_Card_UpStatus = '"+QString::number(0)+"' WHERE IC_Card_num = '00:"+strCard+"' AND IC_Card_addr = '"+strAddr+"'";
                query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    return bSucceed;
}

bool CMYSQL::_AddIcCardA8(QString strCard, QString strAddr, QString strTime)
{
    QString SQL;
    if(strAddr == "栋单元楼房号设备")
        strAddr = "";
    SQL = "insert into ic_card_A8 (IC_Card_addr,IC_Card_num,IC_Card_time) values ('"+strAddr+"','"+strCard+"','"+strTime+"')";
    QSqlDatabase db1 = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query1(db1);
    bool bSucceed1 = query1.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db1);
    return bSucceed1;
}

//清空授权信息
bool CMYSQL::_DeleteIcJurisdictionA8(QString SAddr,QString MAddr)
{
    QSqlDatabase db1 = CCONNECTIONPOOLMYSQL::openConnection();
    QString SQL = "DELETE FROM card_jurisdiction_a8 WHERE Card_Jurisdiction_MAddr = '"+MAddr+"' AND Card_Jurisdiction_SAddr = '"+SAddr+"'";
    QSqlQuery query1(db1);
    bool bSucceed1 = query1.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db1);
    return bSucceed1;
}

//添加授权信息
int CMYSQL::_AddIcJurisdictionA8(QString SAddr, QString MAddr)
{
    QString seleteSQL = "SELECT a.Card_Jurisdiction_SAdd FROM card_jurisdiction_a8 a WHERE a.Card_Jurisdiction_SAddr = '"+SAddr+"' AND a.Card_Jurisdiction_MAddr = '"+MAddr+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery seletequery(db);
    if(seletequery.exec(seleteSQL))
    {
        if(seletequery.next())
        {
            return -1;
        }
    }

    QString insertSQL = "INSERT INTO card_jurisdiction_a8 (Card_Jurisdiction_SAddr,Card_Jurisdiction_MAddr) VALUES ('"+SAddr+"','"+MAddr+"')";
    QSqlQuery insertquery1(db);
    bool bSucceed1 = insertquery1.exec(insertSQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    if(bSucceed1)
        return 1;
    else
        return 0;
}

QString CMYSQL::_FindCard(QString strCardNum)
{
    QString SQL  = "SELECT a.IC_Card_addr from ic_card_a8 a where a.IC_Card_num = '"+strCardNum+"' ";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
        if(query.next())
        {
            return query.value(0).toString();
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return "";
}

//刷卡记录(添加)
int CMYSQL::_InsertCardRecord(QString strType,QString strIntra,QString strCardNum,QString strAddrExplain,QString strCardForIntra)
{
    QString strCard;
    QDateTime dateTime=QDateTime::currentDateTime();
    QString strDtime = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString SQL  = "insert into card_record (Card_Record_addr, Card_Record_device_type, Card_Record_card_num, Card_Record_time, Card_intra_addr, Card_Record_UpStatus, Card_addr) "
            "values ('"+strAddrExplain+"','"+strType+"','"+strCardNum+"','"+strDtime+"','"+strIntra+"','"+QString::number(1)+"','"+strCardForIntra+"')";
    QString SQL1 = "insert into Card_Record (Card_Record_Addr, Card_Record_Device_Type, Card_Record_Card_Num, Card_Record_Time, Card_addr) "
            "values ('"+strAddrExplain+"','"+strType+"','"+strCardNum+"','"+strDtime+"','"+strCardForIntra+"')";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(db1.open())      //
//        {
            QSqlQuery query1(db1);
            bool bSucceed1 = query1.exec(SQL1);
            if(bSucceed1)
            {
                QString strSQL = "update card_record set Card_Record_UpStatus = '"+QString::number(0)+"' where Card_Record_card_num = '"+strCardNum+"'";
                query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    return bSucceed;
}
//更新IC卡信息
int CMYSQL::_ChangeCardPermissions(QString strMiddle,QString strCardNum,int nPermissions)
{
    QString SQL  = "update ic_card set "+strMiddle+" = '"+QString::number(nPermissions)+"',IC_Card_UpStatus = '"+QString::number(2)+"' where IC_Card_num = '"+strCardNum+"'";
    QString SQL1 = "update IC_Card set "+strMiddle+" = '"+QString::number(nPermissions)+"' where IC_Card_num = '"+strCardNum+"' AND IC_Card_Areacode = '"+CLIST::nIDtoString()+"'";

    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(db1.open())      //
//        {
            QSqlQuery query1(db1);
            if(query1.exec(SQL1))
            {
                QString strSQL = "update IC_Card set IC_Card_UpStatus = '"+QString::number(0)+"' where IC_Card_num = '"+strCardNum+"'";
                query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    return bSucceed;
}

int CMYSQL::_Select(QString strCardNum,char *pAddr)
{
    QString SQL = "select  "+QString(pAddr) +" from ic_card where IC_Card_num = '" + strCardNum + "'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    int IsTure = 0;
    if(query.exec(SQL))
    {
        while(query.next())
        {
            IsTure = query.value(0).toInt();
            CCONNECTIONPOOLMYSQL::closeConnection(db);
            return IsTure;
        }
    }
    else
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        return 0;
    }
    return 1;
}

//查询所有有开门权限且有效的IC卡数量
int CMYSQL::_SelectAllCard(QString strMiddle)
{
    QDateTime dateTime=QDateTime::currentDateTime();
    QString strDtime = dateTime.toString("yyyy-MM-dd");
    QString SQL = "select count(*) IC_Card_num from ic_card where IC_Card_time > '" + strDtime + "' and " + strMiddle + "= 1 ";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    int IsTure = 0;
    if(query.exec(SQL))
    {
        while(query.next())
        {
            IsTure = query.value(0).toInt();
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        return IsTure;
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return 0;
}

int CMYSQL::_RecvCard(QString strCardNum,char *pAddr,char *pTime,char *pAddrT)
{
    QString SQL = "select IC_Card_num,IC_Card_time,IC_Card_addr from ic_card where IC_Card_num = '" + strCardNum + "'";
    QSqlDatabase db  = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase dbn = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed;
    QString SQL1,SQLN,strSQL;
    query.exec(SQL);
    if(query.numRowsAffected() > 0)
    {
       query.next();
       memcpy(pTime,query.value(1).toDateTime().toString("yyyy-MM-dd").toUtf8().data(),query.value(1).toDateTime().toString("yyyy-MM-dd").toUtf8().size());
       memcpy(pAddrT,query.value(2).toByteArray().data(),query.value(2).toByteArray().size());
       SQL1 = "update ic_card set "+QString(pAddr)+" = '1',IC_Card_UpStatus = '"+QString::number(2)+"' where IC_Card_num = '"+strCardNum+"'";
       SQLN = "update IC_Card set "+QString(pAddr)+" = '1' where IC_Card_num = '"+strCardNum+"' AND IC_Card_Ardacode = '"+CLIST::nIDtoString()+"'";
    }
    else
    {
        QString strTime = QDateTime::currentDateTime().date().addYears(2).toString("yyyy-MM-dd");
        memcpy(pTime,strTime.toAscii().data(),strTime.toAscii().size());
        SQL1 = "insert into ic_card (IC_Card_num,"+QString(pAddr)+",IC_Card_time,IC_Card_UpStatus)VALUES('"+strCardNum+"',1,'"+strTime +"','"+QString::number(1)+"')";
        SQLN = "insert into IC_Card (IC_Card_num,"+QString(pAddr)+",IC_Card_time,IC_Card_Areacode)VALUES('"+strCardNum+"',1,'"+strTime +"','"+CLIST::nIDtoString()+"')";
    }
    bSucceed = query.exec(SQL1);
    if(bSucceed)
    {
//        if(dbn.open())          //
//        {
            QSqlQuery queryn(dbn);
            if(queryn.exec(SQLN))
            {
                strSQL = "update ic_card  set IC_Card_UpStatus = '"+QString::number(0)+"' where IC_Card_num = '"+strCardNum+"'";
                query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(dbn);
    return 1;
}

//查找是否有IC卡的有效期为今天
int CMYSQL::_IsExpireCard()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString strDtime = dateTime.toString("yyyy-MM-dd");
    QString SQL = "select count(*)IC_Card_num from ic_card where IC_Card_time = '" + strDtime + "'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    int IsTure = 0;
    if(query.exec(SQL))
    {
        while(query.next())
        {
            IsTure = query.value(0).toInt();
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return IsTure;
}
//更改开门权限
bool CMYSQL::_UpdateCardMessage(QString strIcCard,QString strAddr,int nPower)
{
    QString SQL  = "update ic_card set "+strAddr+" = '"+QString::number(nPower)+"',IC_Card_UpStatus = '"+QString::number(2)+"' where IC_Card_num = '"+strIcCard+"'";
    QString SQL1 = "update IC_Card set "+strAddr+" = '"+QString::number(nPower)+"' where IC_Card_num = '"+strIcCard+"' AND IC_Card_Areacode = '"+CLIST::nIDtoString()+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(db1.open())          //
//        {
            QSqlQuery query1(db1);
            if(query1.exec(SQL1))
            {
                QString strSQL = "update ic_card set IC_Card_UpStatus = '"+QString::number(0)+"' where IC_Card_num = '"+strIcCard+"'";
                query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    return bSucceed;
}
//修改IC卡
bool CMYSQL::_UpdateToIcCard(QString strCard,QString strAddr,QString strOldCard,QString strOldAddr,QString strTime)
{
    if(strAddr == "栋单元楼房号设备")
        strAddr = "";

    QString SQL = "update ic_card set IC_Card_addr = '"+strAddr+"',IC_Card_num = '00:"+strCard+"',"
            "IC_Card_time = '"+strTime+"',IC_Card_UpStatus = '"+QString::number(2)+"' where IC_Card_num = '00:"+strOldCard+"'";
//            " //and IC_Card_addr = '"+strOldAddr+"'";
    QString SQL1 = "update IC_Card set IC_Card_addr = '"+strAddr+"',IC_Card_num = '00:"+strCard+"',"
            "IC_Card_time = '"+strTime+"' where IC_Card_num = '00:"+strOldCard+"' AND IC_Card_addr = '"+strOldAddr+"' AND IC_Card_Areacode = '"+CLIST::nIDtoString()+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(db1.open())          //
//        {
            QSqlQuery query1(db1);
            if(query1.exec(SQL1))
            {
                QString strSQL = "update IC_Card set IC_Card_UpStatus = '"+QString::number(0)+"' where IC_Card_num = '00:"+strCard+"' AND IC_Card_addr = '"+strAddr+"'";
                query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    return bSucceed;
}
//删除IC卡
bool CMYSQL::_DeleteIcCard(QString strIcCard)
{
    QString SQL = "delete FROM ic_card WHERE IC_Card_num = '"+strIcCard+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);

    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    if(bSucceed)
    {
//        if(db1.open())          //
//        {
            QSqlQuery queryMsSql(db1);
            QString strMsSql = "delete from IC_Card where  IC_Card_num = '"+strIcCard+"' and IC_Card_Areacode = '"+CLIST::nIDtoString()+"'";
            bool bMsSucceed = queryMsSql.exec(strMsSql);
            if(!bMsSucceed)
            {
                QString strMySql = "insert into UpIcCard (UpIcCard_num) values ('"+strIcCard+"')";
                query.exec(strMySql);
            }
//        }
    }
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//查找IC卡
int CMYSQL::_SelectIcCard(QString strIcCard)
{
    QString SQL = "select * FROM ic_card WHERE IC_Card_num = '00:"+strIcCard+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    if(query.exec(SQL))
    {
        int t = query.numRowsAffected();
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        return t;
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return 0;
}

int CMYSQL::_SelectIcCardA8(QString strIcCard)
{
    QString SQL = "select * FROM ic_card_A8 WHERE IC_Card_num = '"+strIcCard+"'";

    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    if(query.exec(SQL))
    {
        int t = query.numRowsAffected();
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        return t;
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return 0;
}

//修改IC卡
bool CMYSQL::_UpdateIcCard(QString strIcCard,QString strMiddleAddr)
{
    QString strTime = QDateTime::currentDateTime().date().addYears(2).toString("yyyy-MM-dd");
    QString SQL = "SELECT "+strMiddleAddr+" FROM ic_card WHERE IC_Card_num = '"+strIcCard+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    QString SQL1;
    if(query.exec(SQL))
    {
        if(!query.next())
        {
            SQL.clear();
            SQL  = "insert into ic_card (IC_Card_num,"+strMiddleAddr+",IC_Card_time,IC_Card_UpStatus)VALUES('"+strIcCard+"',1,'"+strTime +"','"+QString::number(1)+"')";
            SQL1 = "insert into IC_Card (IC_Card_num,"+strMiddleAddr+",IC_Card_time,IC_Card_Areacode)VALUES('"+strIcCard+"',1,'"+strTime +"','"+CLIST::nIDtoString()+"')";
            bool bSucceed = query.exec(SQL);
            if(bSucceed)
            {
//                if(db1.open())          //
//                {
                    QSqlQuery query1(db1);
                    if(query1.exec(SQL1))
                    {
                        QString strSQL = "update ic_card set IC_Card_UpStatus = '"+QString::number(0)+"' where IC_Card_num = '"+strIcCard+"'";
                        query.exec(strSQL);
                    }
//                }
            }
            CCONNECTIONPOOLMYSQL::closeConnection(db);
            CCONNECTIONPOOLMSSQL::closeConnection(db1);
            return bSucceed;
        }
        else
        {
            if(!query.value(0).toInt())
            {
                SQL.clear();
                SQL  = "update ic_card set "+strMiddleAddr+" = 1,IC_Card_UpStatus = '"+QString::number(2)+"' where IC_Card_num = '"+strIcCard+"' AND IC_Card_Ardacode = '"+CLIST::nIDtoString()+"'";
                SQL1 = "update IC_Card set "+strMiddleAddr+" = 1 where IC_Card_num = '"+strIcCard+"'";
                bool bSucceed = query.exec(SQL);
                if(bSucceed)
                {
//                    if(db1.open())          //
//                    {
                        QSqlQuery query1(db1);
                        if(query1.exec(SQL1))
                        {
                            QString strSQL = "update  ic_card set IC_Card_UpStatus = '"+QString::number(0)+"' where IC_Card_num = '"+strIcCard+"'";
                            query.exec(strSQL);
                        }
//                    }
                }
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                CCONNECTIONPOOLMSSQL::closeConnection(db1);
                return bSucceed;
            }
            CCONNECTIONPOOLMYSQL::closeConnection(db);
            CCONNECTIONPOOLMSSQL::closeConnection(db1);
            return true;
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    return false;
}

bool CMYSQL::_GetAllMiddle(QList<QString> *T)
{
    QString SQL = "SHOW FIELDS FROM ic_card";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    int i = 0;
    if(query.exec(SQL))
    {
         while(query.next())
         {
             i ++;
             if(i > 4)
             {
                 (*T).insert(i,QString(query.value(0).toByteArray().data()));
                 i++;
             }
         }
         CCONNECTIONPOOLMYSQL::closeConnection(db);
         return true;
     }
    else
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
         return false;
    }
}



//解锁密码判断
bool CMYSQL::_Deblock(QString strUser,QString strPass)
{
    QString SQL = "select *from user where User_pass = MD5('"+strPass+"') and User_user = '"+strUser+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    int i = query.exec(SQL);
    if(i)
    {
        if(query.next())
        {
            CCONNECTIONPOOLMYSQL::closeConnection(db);
            return true;
        }
        else
        {
            CCONNECTIONPOOLMYSQL::closeConnection(db);
            return false;
        }
    }
    else
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        return false;
    }
}
//删除住户设备
bool CMYSQL::_DeleteTenement(QString strMac,QString strIp,QString strAddr,QString strType)
{
    QString SQL = "delete from  tenement  where Tenement_addr = '"+strAddr+"' and Tenement_mac_addr = '"+strMac+"' and \
            Tenement_ip_addr = '"+strIp+"' and Tenement_type = '"+strType+"' ";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(db1.open())          //
//        {
            QSqlQuery queryMsSql(db1);
            QString strMsSql = "delete from tenement where Tenement_mac_addr = '"+strMac+"' and Tenement_Areacode = '"+CLIST::nIDtoString()+"'";
            bool bMsSucceed = queryMsSql.exec(strMsSql);
            if(!bMsSucceed)
            {
                QString strMySql = "insert into Updevice (UpDevice_mac_addr) values ('"+strMac+"')";
                query.exec(strMySql);
            }
//        }
    }
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//删除报警记录
bool CMYSQL::_DeleteAlarmRecord(QString strAddr,QString strStimeS,QString strStimeE)
{
    QString SQL;
    SQL.clear();
    if(strAddr != NULL)
    {
        SQL = "delete from alarm where (Alarm_addr like '%"+strAddr+"%' or Alarm_intra_addr like '%"+strAddr+"%') and Alarm_stime > '"+strStimeS+"' and Alarm_stime < '"+strStimeE+"' and  Alarm_etime is NOT NULL OR Alarm_dtime IS NOT NULL";
    }
    else
    {
        SQL = "delete from alarm where Alarm_stime > '"+strStimeS+"' and Alarm_stime < '"+strStimeE+"' and Alarm_etime is NOT NULL OR Alarm_dtime IS NOT NULL";
    }
    qDebug()<<SQL;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//处理报修
bool CMYSQL::_DealRepairs(QString strAddr,QString strStime,QString strDtime,QString strHuman,QString strMessage)
{
    QString SQL = "update repairs_record set Repairs_Record_dtime = '"+strDtime+"' ,Repairs_Record_deal_human = '"+strHuman+"',"
            "Repairs_Record_message = '"+strMessage+"',Repairs_Record_UpStatus = '"+QString::number(2)+"' where Repairs_Record_addr = '"+strAddr+"' and Repairs_Record_stime = '"+strStime+"'";

    QString SQL1 = "update Repairs_Record set Repairs_Record_dtime = '"+strDtime+"' ,Repairs_Record_deal_human = '"+strHuman+"',"
            "Repairs_Record_message = '"+strMessage+"' where Repairs_Record_addr = '"+strAddr+"' and Repairs_Record_stime = '"+strStime+"' and Repairs_Record_Areacode = '"+CLIST::nIDtoString()+"'";

    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(db1.open())          //
//        {
            QSqlQuery query1(db1);
            if(query1.exec(SQL1))
            {
                QString strSQL = "update repairs_record set Repairs_Record_UpStatus = '"+QString::number(0)+"' WHERE Repairs_Record_addr = '"+strAddr+"' and Repairs_Record_stime = '"+strStime+"'";
                query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    return bSucceed;
}
//处理所有
bool CMYSQL::_DealAll(int nFlag)
{
    QString SQL,SQL1;
    SQL.clear();
    QDateTime dateTime = QDateTime::currentDateTime();
    QString strDtime = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    if(nFlag == 1)//报警
    {
        SQL = "update alarm set Alarm_dtime = '"+strDtime+"' ,Alarm_deal_human = '管理员',Alarm_UpStatus = '"+QString::number(2)+"'"
                 " where Alarm_dtime is NULL";
        SQL1 = "update Alarm set Alarm_dtime = '"+strDtime+"' ,Alarm_deal_human = '管理员'"
                " where Alarm_Areacode = '"+CLIST::nIDtoString()+"' AND Alarm_dtime is NULL";
    }
    else
        SQL = "update repairs_record set Repairs_Record_dtime = '"+strDtime+"' ,Repairs_Record_deal_human = '管理员',Repairs_Record_UpStatus = '"+QString::number(2)+"'"
              " where Repairs_Record_dtime is NULL";
        SQL = "update Repairs_Record set Repairs_Record_dtime = '"+strDtime+"' ,Repairs_Record_deal_human = '管理员'"
                " where Repairs_Record_Areacode = '"+CLIST::nIDtoString()+"' AND Repairs_Record_dtime is NULL";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(db1.open())          //
//        {
            QSqlQuery query1(db1);
            if(query1.exec(SQL1))
            {
                QString strSQL;
                if(nFlag == 1)//报警
                    strSQL = "update alarm set Alarm_UpStatus = '"+QString::number(0)+"' where Alarm_dtime = '"+strDtime+"'";
                else
                    strSQL = "update repairs_record set Repairs_Record_UpStatus = '"+QString::number(0)+"' where Repairs_Record_dtime = '"+strDtime+"'";
                query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    return bSucceed;
}
//查找用户
 bool CMYSQL::_FindUser(QString strUser,QString strPass)
 {
     QString SQL = "select *from user where User_pass = MD5('"+strPass+"') and User_user = '"+strUser+"'";
     QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
     QSqlQuery query(db);
     if(query.exec(SQL))
     {
         if(query.next())
         {
             CCONNECTIONPOOLMYSQL::closeConnection(db);
             return true;
         }
         else
         {
             CCONNECTIONPOOLMYSQL::closeConnection(db);
             return false;
         }
     }
     else
     {
         CCONNECTIONPOOLMYSQL::closeConnection(db);
         return false;
     }
 }
//插入设备日志
bool CMYSQL::_OnlineMiddle(SMiddleDevice sMiddleDevice)
{
    QString str = QString(sMiddleDevice.gcAddr);
    QString str1 = QString(sMiddleDevice.gcAddrExplain);
    QString str2 = QString(sMiddleDevice.gcMacAddr);
    QString str3 = QString(sMiddleDevice.gcIpAddr);
    QString str4 = QString(sMiddleDevice.gcType);
    QString str5 = QString(sMiddleDevice.gcIntraAddr);
    QString str6 = "断开";
    QDateTime dateTime=QDateTime::currentDateTime();
    QString str7 = dateTime.toString("yyyy-MM-dd hh:mm:ss");

    QString SQL = "UPDATE middle SET Middle_addr = '"+str+"',Middle_addr_explain = '"+str1+"',Middle_ip_addr = '"
            +str3+"',Middle_type = '"+str4+"',Middle_intra_addr = '"+str5+"',Middle_Online = '"+QString::number(0)+"'"
            " ,Middle_UpStatus = '"+QString::number(2)+"' where Middle_mac_addr = '"+str2+"'";

    QString SQLN = "UPDATE Tenement SET Tenement_addr = '"+str+"',Tenement_addr_explain = '"+str1+"',Tenement_ip_addr = '"
            +str3+"',Tenement_type = '"+str4+"',Tenement_intra_addr = '"+str5+"',Tenement_Online = '"+QString::number(0)+"'"
            " where Tenement_mac_addr = '"+str2+"' AND Tenement_Areacode = '"+CLIST::nIDtoString()+"'";

    QString SQL1 = "insert into device (Device_addr,Device_ip_addr,Device_status,Device_time,Device_intra_addr,Device_type) VALUES ('"+str1+"','"+str3+"','"+str6+"','"+str7+"','"+str5+"','"+str4+"')";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase dbn = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(dbn.open())          //
//        {
            QSqlQuery queryn(dbn);
            if(queryn.exec(SQLN))
            {
                QString strSQL = "UPDATE middle SET Middle_UpStatus = '"+QString::number(0)+"' where Middle_mac_addr = '"+str2+"'";
                query.exec(strSQL);
            }
//        }
    }
    if(bSucceed && query.exec(SQL1))
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return true;
    }
    else
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return false;
    }
}
//处理报警
bool CMYSQL::_UpdateDealAlarm(SAlarmDevice sAlarmDevice,QString strAlarmStatu,QString strDealHuman,QString strMessage)
{
    QString strDtime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString strStime = QString(sAlarmDevice.gcStime);
    QString SQL = "update alarm set Alarm_dtime = '"+strDtime+"',Alarm_status = '"+strAlarmStatu+"',"
            "Alarm_deal_human = '"+strDealHuman+"',Alarm_deal_message = '"+strMessage+"',Alarm_UpStatus = '"+QString::number(2)+"' where Alarm_stime = '"+strStime+"'";

    QString SQL1 = "update Alarm set Alarm_dtime = '"+strDtime+"',Alarm_status = '"+strAlarmStatu+"',"
            "Alarm_deal_human = '"+strDealHuman+"',Alarm_deal_message = '"+strMessage+"' where Alarm_stime = '"+strStime+"' AND Alarm_Areacode = '"+CLIST::nIDtoString()+"'";

    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(db1.open())        //
//        {
            QSqlQuery query1(db1);
            if(query1.exec(SQL1))
            {
                QString strSQL = "update alarm set Alarm_UpStatus = '"+QString::number(0)+"' where Alarm_stime = '"+strStime+"'";
                query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    return bSucceed;
}
//更新报警结束时间
bool CMYSQL::_UpdateAlarm(SAlarmDevice sAlarmDevice)
{
    QString str = QString(sAlarmDevice.gcIpAddr);
    QString str1 = QString(sAlarmDevice.gcEtime);
    QString str2 = QString(sAlarmDevice.gcIntraAddr);
    QString SQL  = "update alarm set Alarm_UpStatus = '"+QString::number(2)+"',Alarm_etime = '"+str1+"' where Alarm_ip_addr = '"+str+"' and Alarm_intra_addr = '"+str2+"' and Alarm_etime is null";
    QString SQL1 = "update Alarm set Alarm_etime = '"+str1+"' where Alarm_Areacode = '"+CLIST::nIDtoString()+"' AND Alarm_ip_addr = '"+str+"' and Alarm_intra_addr = '"+str2+"' and Alarm_etime is null";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(db1.open())          //
//        {
            QSqlQuery query1(db1);
            if(query1.exec(SQL1))
            {
                QString strSQL = "update  alarm set Alarm_UpStatus = '"+QString::number(0)+"' where Alarm_ip_addr = '"+str+"' and Alarm_intra_addr = '"+str2+"' and Alarm_etime = '"+str1+"'";
                query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    return bSucceed;
}
//修改报修记录
bool CMYSQL::_UpdateRepairsRecord(int nFlag,QString strAddr,QString strType,QString strStime,QString strIntraAddr,QString strDeviceType)
{
    QString SQL;
    SQL.clear();
    switch(nFlag)
    {
        case 0:
        SQL = "insert into repairs_record (Repairs_Record_addr,Repairs_Record_type,Repairs_Record_stime,Repairs_Record_intra_addr,Repairs_Record_device_type,Repairs_Record_UpStatus)"
                "VALUES ('"+strAddr+"','"+strType+"','"+strStime+"','"+strIntraAddr+"','"+strDeviceType+"','"+QString::number(1)+"')";
            break;
        case 1:
            SQL = "update repairs_record set Repairs_Record_UpStatus = '"+QString::number(2)+"',Repairs_Record_etime = '"+strStime+"' where Repairs_Record_intra_addr = '"+strIntraAddr+"' and Repairs_Record_type = '"+strType+"' and Repairs_Record_etime is null";
            break;
        default :
            break;
    }

    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
        QString SQL1 ;
        switch(nFlag)
        {
            case 0:
                SQL1 = "insert into Repairs_Record (Repairs_Record_Addr,Repairs_Record_Type,Repairs_Record_Stime,Repairs_Record_Intra_Addr,Repairs_Record_Device_Type,Repairs_Record_Areacode)"
                    "VALUES ('"+strAddr+"','"+strType+"','"+strStime+"','"+strIntraAddr+"','"+strDeviceType+"','"+CLIST::nIDtoString()+"')";
                break;
            case 1:
                SQL1 = "update Repairs_Record set Repairs_Record_Etime = '"+strStime+"' where Repairs_Record_Intra_Addr = '"+strIntraAddr+"' and Repairs_Record_Areacode = '"+CLIST::nIDtoString()+"' and Repairs_Record_Type = '"+strType+"' and Repairs_Record_Etime is null";
                break;
            default :
                break;
        }
//        if(db1.open())          //
//        {
            QSqlQuery query1(db1);
            if(query1.exec(SQL1))
            {
                 QString strSQL = SQL = "update repairs_record set Repairs_Record_UpStatus = '"+QString::number(0)+"' where Repairs_Record_intra_addr = '"+strIntraAddr+"' and Repairs_Record_type = '"+strType+"'";
                 query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db);
    return bSucceed;
}
/*========================================================================
        Name:		操作类型数据库存储。
    ----------------------------------------------------------
        returns:	返回保存是否成功。
    ----------------------------------------------------------
        Remarks:        将当期用户的操作类型存入数据库
    ----------------------------------------------------------
        Params:         nType 操作类型
                        strUser 用户名
                        strName 用户姓名
==========================================================================*/
bool CMYSQL::_InsertSystem(int nType,QString strUser,QString strName)
{
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    QString strType,strTime;
    strType.clear();
    strTime.clear();
    switch(nType)
    {
    case 1:
        strType = "登陆";
        break;
    case 2:
        strType = "锁定";
        break;
    case 3:
        strType = "退出";
        break;
    case 4:
        strType = "解锁";
        break;
    case 5:
        strType = "注销";
        break;
    default :
        break;
    }
    QDateTime dateTime=QDateTime::currentDateTime();
    strTime = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString SQL = "insert into System (System_name,System_human,System_type,System_time)values ('"+strUser+"','"+strName+"','"+strType+"','"+strTime+"')";

    bool bSucceed = query.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    return bSucceed;
}
//插入报警
bool CMYSQL::_InsertAlarm(SAlarmDevice sAlarmDevice)
{
    QString str = QString(sAlarmDevice.gcMacAddr);
    QString str1 = QString(sAlarmDevice.gcIpAddr);
    QString str2 = "防区" + QString::number(sAlarmDevice.nFenceId);
    QString str3;
    switch(sAlarmDevice.nAlarmType)
    {
        str3.clear();
        case 0:
            str3 = "火警";
            break;
        case 2:
            str3 = "红外";
            break;
        case 3:
            str3 = "门铃";
            break;
        case 4:
            str3 = "烟感";
            break;
        case 6:
            str3 = "门磁1";
            break;
        case 7:
            str3 = "遥控SOS";
            break;
        case 8:
            str3 = "门磁2";
            break;
        case 10:
            str3 = "水浸";
            break;
        case 12:
            str3 = "煤气";
            break;
        case 13:
            str3 = "门磁3";
            break;
        case 14:
            str3 = "胁迫开门";
            break;
        case 15:
            str3 = "设备强拆";
            break;
        case 16:
            str3 = "开门超时";
        default :
            str3 = "错误";
            break;
    }

    QString str4 = QString(sAlarmDevice.gcStime);
    QString str5 = QString(sAlarmDevice.gcIntraAddr);
    QString str6 = QString(sAlarmDevice.gcName);
    QString str7 = QString(sAlarmDevice.gcPhone1);
    QString str8 = QString(sAlarmDevice.gcAddr);
    QString str9 = QString(sAlarmDevice.gcType);
    QString SQL = "INSERT INTO alarm (alarm.Alarm_mac_addr,alarm.Alarm_ip_addr,alarm.Alarm_fence,alarm.Alarm_type,alarm.Alarm_stime,alarm.Alarm_intra_addr,alarm.Alarm_name,alarm.Alarm_phone,alarm.Alarm_addr,alarm.Alarm_device_type,alarm.Alarm_UpStatus)"
            "VALUES ('"+str+"','"+str1+"','"+str2+"','"+str3+"','"+str4+"','"+str5+"','"+str6+"','"+str7+"','"+str8+"','"+str9+"','"+QString::number(1)+"')";
    QString SQL1 = "INSERT INTO Alarm (Alarm.Alarm_mac_addr,Alarm.Alarm_ip_addr,Alarm.Alarm_fence,Alarm.Alarm_type,Alarm.Alarm_stime,Alarm.Alarm_intra_addr,Alarm.Alarm_name,Alarm.Alarm_phone,Alarm.Alarm_addr,Alarm.Alarm_device_type,Alarm.Alarm_Areacode)"
            "VALUES ('"+str+"','"+str1+"','"+str2+"','"+str3+"','"+str4+"','"+str5+"','"+str6+"','"+str7+"','"+str8+"','"+str9+"','"+CLIST::nIDtoString()+"')";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(db1.open())          //
//        {
            QSqlQuery query1(db1);
            if(query1.exec(SQL1))
            {
                QString strSQL = "update alarm set Alarm_UpStatus = '"+QString::number(0)+"' WHERE Alarm_mac_addr = '"+str+"' AND Alarm_stime = '"+str4+"' AND Alarm_intra_addr = '"+str5+"'";
                query.exec(strSQL);
            }
//        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    CCONNECTIONPOOLMSSQL::closeConnection(db1);
    qDebug()<<"报警插入："<<bSucceed;
    return bSucceed;
}
//插入住户设备
bool CMYSQL::_InsertTenement(STenementDevice sTenementDevice)
{
    QString str = QString(sTenementDevice.gcAddr);
    QString str1 = QString(sTenementDevice.gcAddrExplain);
    QString str2 = QString(sTenementDevice.gcMacAddr);
    QString str3 = QString(sTenementDevice.gcIpAddr);
    QString str4 = QString(sTenementDevice.gcType);
    QString str5 = QString(sTenementDevice.gcIntraAddr);
    QString str6 = QString(sTenementDevice.gcName);
    QString str7 = QString(sTenementDevice.gcPhone1);
    QString str8 = QString(sTenementDevice.gcPhone2);
    QString str9 = "连接";
    QString str10 = QString::number(sTenementDevice.nFenceState);
    QDateTime dateTime=QDateTime::currentDateTime();
    QString str11 = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString SQL = "insert into tenement (Tenement_addr,Tenement_addr_explain,Tenement_ip_addr,Tenement_mac_addr,Tenement_type,Tenement_intra_addr,Tenement_name,Tenement_phone1,Tenement_phone2,Tenement_fence_statu,Tenement_Online,Tenement_UpStatus)VALUES ('"+
            str+"','"+str1+"','"+str3+"','"+str2+"','"+str4+"','"+str5+"','"+str6+"','"+str7+"','"+str8+"','"+str10+"','"+QString::number(1)+"','"+QString::number(1)+"')";
    QString SQL1 = "insert into device (Device_addr,Device_ip_addr,Device_status,Device_time,Device_intra_addr,Device_type) VALUES ('"+str1+"','"+str3+"','"+str9+"','"+str11+"','"+str5+"','"+str4+"')";

    QString str12;
    str12.clear();
    switch(sTenementDevice.nFenceState)
    {
        case 1:
            str12 = "外出布防";
            break;
        case 2:
            str12 = "在家布防";
            break;
        case 0:
            str12 = "撤防";
            break;
        default :
            break;
    }
    QString SQL2 = "insert into device (Device_addr,Device_ip_addr,Device_status,Device_time,Device_intra_addr,Device_type) VALUES ('"+str1+"','"+str3+"','"+str12+"','"+str11+"','"+str5+"','"+str4+"')";

    QString SQLN = "insert into Tenement (Tenement_addr,Tenement_addr_explain,Tenement_ip_addr,Tenement_mac_addr,Tenement_type,Tenement_intra_addr,Tenement_name,Tenement_phone1,Tenement_phone2,Tenement_fence_statu,Tenement_Online,Tenement_Areacode)"
            " VALUES ('"+str+"','"+str1+"','"+str3+"','"+str2+"','"+str4+"','"+str5+"','"+str6+"','"+str7+"','"+str8+"','"+str10+"','"+QString::number(1)+"','"+CLIST::nIDtoString()+"')";

    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase dbn = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed1 = query.exec(SQL);
    bool bSucceed2 = query.exec(SQL1);
    bool bSucceed3 = query.exec(SQL2);
    if(bSucceed1)
    {
//        if(dbn.open())          //
//        {
            QSqlQuery queryn(dbn);
            bool b = queryn.exec(SQLN);
            if(b)
            {
                QString strSQL = "UPDATE tenement SET Tenement_UpStatus = '"+QString::number(0)+"' WHERE Tenement_mac_addr = '"+str2+"'";
                query.exec(strSQL);
            }
//        }
    }
    if(bSucceed1 && bSucceed2 && bSucceed3)
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return true;
    }
    else
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return false;
    }
}


//掉线存入设备日志
bool CMYSQL::_OnlineTenement(STenementDevice sTenementDevice)
{
    QString str  = QString(sTenementDevice.gcAddr);
    QString str1 = QString(sTenementDevice.gcAddrExplain);
    QString str2 = QString(sTenementDevice.gcMacAddr);
    QString str3 = QString(sTenementDevice.gcIpAddr);
    QString str4 = QString(sTenementDevice.gcType);
    QString str5 = QString(sTenementDevice.gcIntraAddr);
    QString str6 = QString(sTenementDevice.gcName);
    QString str7 = QString(sTenementDevice.gcPhone1);
    QString str8 = QString(sTenementDevice.gcPhone2);
    QString str9 = "断开";
    QString str10 = QString::number(sTenementDevice.nFenceState);
    QDateTime dateTime=QDateTime::currentDateTime();
    QString str11 = dateTime.toString("yyyy-MM-dd hh:mm:ss");

    QString SQL = "UPDATE tenement SET Tenement_addr = '"+str+"',Tenement_addr_explain = '"+str1+"',Tenement_ip_addr = '"
            +str3+"',Tenement_type = '"+str4+"',Tenement_intra_addr = '"+str5+"',Tenement_name = '"
            +str6+"',Tenement_phone1 = '"+str7+"',Tenement_phone2 = '"+str8+"',Tenement_fence_statu = '"+str10+"'"
            ",Tenement_Online = '"+QString::number(0)+"',Tenement_UpStatus = '"+QString::number(2)+"' where Tenement_mac_addr = '"+str2+"'";

    QString SQLN = "UPDATE Tenement SET Tenement_addr = '"+str+"',Tenement_addr_explain = '"+str1+"',Tenement_ip_addr = '"
            +str3+"',Tenement_type = '"+str4+"',Tenement_intra_addr = '"+str5+"',Tenement_name = '"
            +str6+"',Tenement_phone1 = '"+str7+"',Tenement_phone2 = '"+str8+"',Tenement_fence_statu = '"+str10+"'"
            ",Tenement_Online = '"+QString::number(0)+"' where Tenement_mac_addr = '"+str2+"' AND Tenement_Areacode = '"+CLIST::nIDtoString()+"'";


    QString SQL1 = "insert into device (Device_addr,Device_ip_addr,Device_status,Device_time,Device_intra_addr,Device_type) VALUES ('"+str1+"','"+str3+"','"+str9+"','"+str11+"','"+str5+"','"+str4+"')";

    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase dbn = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(dbn.open())          //
//        {
            QSqlQuery queryn(dbn);
            if(queryn.exec(SQLN))
            {
                QString strSQL = "UPDATE tenement SET Tenement_UpStatus = '"+QString::number(0)+"' WHERE Tenement_mac_addr = '"+str2+"'";
                query.exec(strSQL);
            }
//        }
    }
    if(query.exec(SQL1))
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return true;
    }
    else
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return false;
    }
}
//更新住户设备
bool CMYSQL::_UpdateTenement(STenementDevice sTenementDevice)
{
    QString str = QString(sTenementDevice.gcAddr);
    QString str1 = QString(sTenementDevice.gcAddrExplain);
    QString str2 = QString(sTenementDevice.gcMacAddr);
    QString str3 = QString(sTenementDevice.gcIpAddr);
    QString str4 = QString(sTenementDevice.gcType);
    QString str5 = QString(sTenementDevice.gcIntraAddr);
    QString str6 = QString(sTenementDevice.gcName);
    QString str7 = QString(sTenementDevice.gcPhone1);
    QString str8 = QString(sTenementDevice.gcPhone2);
    QString str9 = "连接";
    QString str10 = QString::number(sTenementDevice.nFenceState);
    QDateTime dateTime=QDateTime::currentDateTime();
    QString str11 = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString SQL = "UPDATE tenement SET Tenement_addr = '"+str+"',Tenement_addr_explain = '"+str1+"',Tenement_ip_addr = '"
            +str3+"',Tenement_type = '"+str4+"',Tenement_intra_addr = '"+str5+"',Tenement_name = '"
            +str6+"',Tenement_phone1 = '"+str7+"',Tenement_phone2 = '"+str8+"',Tenement_fence_statu = '"+str10+"' "
            ",Tenement_Online = '"+QString::number(1)+"',Tenement_UpStatus = '"+QString::number(2)+"'  where Tenement_mac_addr = '"+str2+"'";
    QString SQLN = "UPDATE Tenement SET Tenement_addr = '"+str+"',Tenement_addr_explain = '"+str1+"',Tenement_ip_addr = '"
            +str3+"',Tenement_type = '"+str4+"',Tenement_intra_addr = '"+str5+"',Tenement_name = '"
            +str6+"',Tenement_phone1 = '"+str7+"',Tenement_phone2 = '"+str8+"',Tenement_fence_statu = '"+str10+"' "
            ",Tenement_Online = '"+QString::number(1)+"'  where Tenement_mac_addr = '"+str2+"' AND Tenement_Areacode = '"+CLIST::nIDtoString()+"'";

    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase dbn = CCONNECTIONPOOLMSSQL::openConnection();
    QSqlQuery query(db);
    bool bSucceed = query.exec(SQL);
    if(bSucceed)
    {
//        if(dbn.open())          //
//        {
            QSqlQuery queryn(dbn);
            if(queryn.exec(SQLN))
            {
                QString strSQL = "UPDATE tenement SET Tenement_UpStatus = '"+QString::number(0)+"' WHERE Tenement_mac_addr = '"+str2+"'";
                query.exec(strSQL);
            }
//        }
    }

    QString SQL1;
    int j = 0;
    int m = 0;
    if(sTenementDevice.nState > 0)
    {
        j = 1;
    }
    else
    {
        SQL1.clear();
        SQL1 = "insert into device (Device_addr,Device_ip_addr,Device_status,Device_time,Device_intra_addr,Device_type) VALUES ('"+str1+"','"+str3+"','"+str9+"','"+str11+"','"+str5+"','"+str4+"')";
        j = query.exec(SQL1);
    }

    if(sTenementDevice.nOldFenceState == sTenementDevice.nFenceState)
    {
        m = 1;
    }
    else
    {
        QString str12;
        str12.clear();
        switch(sTenementDevice.nFenceState)
        {
            case 1:
                str12 = "外出布防";
                break;
            case 2:
                str12 = "在家布防";
                break;
            case 0:
                str12 = "撤防";
                break;
            default :
                break;
        }
        SQL1.clear();
        SQL1 = "insert into device (Device_addr,Device_ip_addr,Device_status,Device_time,Device_intra_addr,Device_type) VALUES ('"+str1+"','"+str3+"','"+str12+"','"+str11+"','"+str5+"','"+str4+"')";
        m = query.exec(SQL1);
    }
//    qDebug()<<"j:"<<j<<"m:"<<m;
    if(query.exec(SQL) && j && m)
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return true;
    }
    else
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        CCONNECTIONPOOLMSSQL::closeConnection(dbn);
        return false;
    }
}

//插入呼叫记录
bool CMYSQL::_InsertCallRecord(QString strDialing,QString strCalled,QDateTime DateTime,QString strCallName)
{
    QString strDateTime = DateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString SQL  = "INSERT INTO call_photo_a8 (Call_Photo_Time,Call_Photo_DialingAddr,Call_Photo_CalledAddr,Call_Photo_Path) VALUES ('"+strDateTime+"','"+strDialing+"','"+strCalled+"','"+strCallName+"')";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);

    if(query.exec(SQL))
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        return true;
    }
    else
    {
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        return false;
    }
}
//删除呼叫记录
bool CMYSQL::_DeleteCallRecord(QString strAddr, QString strStimeS, QString strStimeE)
{
    QString SQL;
    SQL.clear();
    if(strAddr != NULL)
    {
        SQL = "DELETE FROM call_photo_a8 WHERE call_photo_dialingaddr like '%"+strAddr+"%' AND call_photo_time between '"+strStimeS+"' AND '"+strStimeE+"'";
    }
    else
    {
        SQL = "DELETE FROM call_photo_a8 WHERE call_photo_time between '"+strStimeS+"' AND '"+strStimeE+"'";
    }
    QSqlDatabase MyDB = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery MyQuery(MyDB);
    bool MybSucceed = MyQuery.exec(SQL);
    CCONNECTIONPOOLMYSQL::closeConnection(MyDB);
    return MybSucceed;
}
