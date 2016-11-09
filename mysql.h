#ifndef MYSQL_H
#define MYSQL_H
#include <QtGui/QApplication>
#include <QtSql>
#include <QSqlDatabase>
#include "STRUCT.h"
#include "connectionpoolmysql.h"
#include "connectionpoolmssql.h"
class CMYSQL
{
private:
     QString m_strDBName,m_strDBUser,m_strDBPass;

public:
    CMYSQL();

    bool _OpenODBC();
    bool _OpenMySql();
    bool _IsOpen();
    bool _GetAllUser(QList<QString> *T);
    QString _GetCityNum(QString strCity);
    bool _GetUser(SUser &sUser);

    bool _InsertTenement(STenementDevice sTenementDevice);
    bool _UpdateTenement(STenementDevice sTenementDevice);
    bool _DeleteTenement(QString strMac,QString strIp,QString strAddr,QString strType);
    bool _InsertMiddle(SMiddleDevice sMiddleDevice);
    bool _UpdateMiddle(int nMark,SMiddleDevice sMiddleDevice);
    bool _DeleteMiddle(QString strMac,QString strIp,QString strAddr,QString strType);
    bool _InsertAlarm(SAlarmDevice sAlarmDevice);
    bool _UpdateAlarm(SAlarmDevice sAlarmDevice);
    bool _UpdateDealAlarm(SAlarmDevice sAlarmDevice,QString strAlarmStatu,QString strDealHuman,QString strMessage);
    bool _DeleteAlarmRecord(QString strAddr,QString strStimeS,QString strStimeE);

    //设备日志
    bool _OnlineTenement(STenementDevice sTenementDevice);
    bool _OnlineMiddle(SMiddleDevice sMiddleDevice);
    bool _DeleteDeviceRecord(QString strAddr,QString strStimeS,QString strStimeE);
    //常用电话 报修类型
    bool _UpdatePhone(QString strNewDepartment,QString strNewHuman,QString strNewPhone,QString strOldDepartment,QString strOldHuman,QString strOldPhone);
    bool _InsertPhone(QString strDepartment,QString strHuman,QString strPhone);
    bool _DeletePhone(QString strDepartment,QString strHuman,QString strPhone);

    bool _UpdateRepairs(QString strNewType,QString strOldType);
    bool _InsertRepairs(QString strType);
    bool _DeleteRepairs(QString strType);
    //用户
    bool _Deblock(QString strUser,QString strPass);
    bool _InsertUser(QString strName,QString strUser,QString strPass,int nGrade);
    bool _UpdateUser(QString strOldUser,QString strName,QString strUser,QString strPass);
    bool _DeleteUser(QString strName,QString strUser);
    bool _FindUser(QString strUser,QString strPass);
    bool _DeleteCardRecord(QString strAddr,QString strStimeS,QString strStimeE);
    //报修记录
    bool _DeleteRepairsRecord(QString strAddr,QString strStimeS,QString strStimeE);
    bool _DealRepairs(QString strAddr,QString strStime,QString strDtime,QString strHuman,QString strMessage);
    bool _DealAll(int nFlag);
    bool _UpdateRepairsRecord(int nFlag,QString strAddr,QString strType,QString strStime,QString strIntraAddr,QString strDeviceType);
    //系统日志
    bool _InsertSystem(int nType,QString strUser,QString strName);
    bool _DeleteSystemRecord(QString strAddr,QString strStimeS,QString strStimeE);
    //刷卡记录
    int  _InsertCardRecord(QString strType,QString strIntra,QString strCardNum,QString strAddrExplain,QString strCardForIntra);
    QString _FindCard(QString strCardNum);
    //IC卡
    int  _SelectIcCard(QString strIcCard);//是否存在
    int  _SelectIcCardA8(QString strIcCard);    //A8
    bool _UpdateIcCard(QString strIcCard,QString strMiddleAddr);
    bool _DeleteIcCard(QString strIcCard);
    bool _AddIcCard(int m_nLimits,QList<QString> T,QString strCard,QString strAddr,QString strTime);
    bool _AddIcCardA8(QString strCard,QString strAddr,QString strTime); //A8
    bool _DeleteIcJurisdictionA8(QString SAddr,QString MAddr);      //A8
    int  _AddIcJurisdictionA8(QString SAddr, QString MAddr);        //A8
    bool _UpdateToIcCard(QString strCard,QString strAddr,QString strOldCard,QString strOldAddr,QString strTime);
    bool _UpdateCardMessage(QString strIcCard,QString strAddr,int nPower);
    int  _IsExpireCard();//有效期检测
    int  _SelectAllCard(QString strMiddle);
    int  _ChangeCardPermissions(QString strMiddle,QString strCardNum,int nPermissions);
    int  _RecvCard(QString strCardNum,char *pAddr,char *pTime,char *pAddrT);
    bool _GetAllMiddle(QList<QString> *T);
    int  _Select(QString strCardNum,char *pAddr);
    QString _AddrExplainToAddr(QString AddrExplain);
    QString _AddrToAddrExplain(QString Addr);

    //插入呼叫记录
    bool _InsertCallRecord(QString strDialing,QString strCalled,QDateTime DateTime,QString strCallName);
    bool _DeleteCallRecord(QString strAddr, QString strStimeS, QString strStimeE);
};

#endif // MYSQL_H
