#ifndef UDPSERVER_H
#define UDPSERVER_H

#include"Item.h"

#include <QThread>
#include <windows.h>
#include "mysql.h"
#include <Winsock2.h>
#include <QDebug>
#include <WS2TCPIP.h>
#include <QtNetwork>
#include <QtScript/QtScript>
#include <QtScript/QScriptEngine>
#include <QVariant>
#include <QTimer>
#include "connectionpoolmysql.h"
class CUDPSERVER : public QThread
{
    Q_OBJECT
public:
    explicit CUDPSERVER(QString strCityNum,QObject *parent = 0);
    CMYSQL m_Mysql;
    NodeTenement *m_TenementItem;
    NodeMiddle *m_MiddleItem;
    NodeAlarm *m_AlarmItem;
    U16 m_nPage;
    U32 m_nAllTenement;
    U32 m_nAllTenementOnline;
    U32 m_nAllMiddle;
    U32 m_nAllMiddleOnline;
    U16 m_nAlarm;
    SWeather sWeather;
    QString m_strCity;
    QStringList Files;
    QFile File;
    int nSendId,nAallfile,nFinishfiles,nPacklen;
    int nCheckId,nSendPack,nLength,nFailFile,nAskTime;
    QString strPath;
public:
    void run();
    //普通信息下发
    int _SendMessage(QString strMessage);
    //新闻下发
    void _Path();
    void _LoadDown(QString strIp);
    //读写地址
    void _SendRead(char *pAddr,QString strIp);
    void _SendWrite(char *pBuf,QString strIp);
    //下发报修类型 电话 灯光控制
    void _SendRepairs(int nLen,char *pMessage);
    void _SendPhone(int nLen,char *pMessage);
    void _SendLamp(char *pBuf,QString strIp);//控制灯
    //下发初始化密码
    void _SendPass(QString strAddr,QString strMac,QString strIp,QString strType,int nFlag);
    //获取报警设备信息
    int _GetAlarmFromItem(SAlarmDevice *sAlarmDevice,NodeAlarm *ItemAlarm);
    //删除报警 （报警链表删除）
    SAlarmDevice _DeleteItemAlarm(SAlarmDevice sAlarmDevice);
    void _DealAlarm(char *pBuf,int nAlarmType,QString strIp);

    //更新城市信息
    void _UpdateLoading(QString strCityNum);
    //上传 注册 全部下载 注销 卡命令下发
    int _UpCard(QByteArray byte);
    int _DownCard(QByteArray byte,char *pCard,int nAllCard,int nSendCard,int nPack);
    int _DownAllCard(QByteArray byte,char *pCard,int nAllCard,int nSendCard,int nPack);
    int _LogOutCard(QByteArray byte,char *pCard,int nAllCard,int nSendCard,int nPack);
    int _DownAloneCard(QByteArray byte,char *pCard,int nAllCard);//未使用
    //清空报警链表
    bool _ClearAlarm();
    //定时下发卡（有效期处理）上传可以处理不要
    void _SendUpCard();
    void _SendDownCard();
    //重启设备
    void _Reset(QString strIp,QString strAddr,QString strType);
signals:
    void SigUdpOpenFail();
    void SigInitMiddle();
    void SigUpdateMiddle(QVariant dataVar);
    void SigOnlineMiddle(int Id);//新写
    void SigReset();


    void SigInitTenement();//住户地址改变
    void SigUpdateTenement(QVariant dataVar);//改变
    void SigOnlineTenement(int Id,int Alarm);//掉线
    void SigSendMessage(int Id,int Result);//发送结果

    void Sigbtn(int flag);
    void SigNewsDonw(int Id,int All,int Finfish);

    void SigInsertAlarm();//新写
    void SigRepairsRecord();//新写

    void SigRecvRead(unsigned char *buf,QString strIp);
    void SigRecvWrite(unsigned char *buf,QString strIp);
    void SigRecvInitPass(QString strAddr,int nFlag);

    void SigUpdateMac();
    void SigUpdateAddr();

    void SigRecvLogOutCard(unsigned char *buf,QString strIp);
    void SigRecvDownCard(unsigned char *buf,QString strIp);
    void SigRecvDownAllCard(unsigned char *buf,QString strIp);
    void SigRecvUpCard(unsigned char *buf);
    void SigDateTime(int nFlag,int nDay);
    void SigUpdateWeatherNew();

    void SigRecvDownAloneCard(int nAllCard);

    void _test(unsigned char *buf);
public slots:
    void SlotReadyRead();
    void SlotReadyRead1();
    void _UpdateWeather();
    void _TimeSendCard();
private :
    char m_gHead[6];
    char m_gMark[20];
    char m_gName[20];
    QNetworkRequest m_NetworkRequest;
    QNetworkReply *m_NetworkReply;
    QNetworkAccessManager *m_NetworkManager;
    QNetworkRequest m_NetworkRequest1;
    QNetworkReply *m_NetworkReply1;
    QNetworkAccessManager *m_NetworkManager1;

    SOCKET Fsocket;
    WSADATA wsd;
    struct sockaddr_in To;
    QString m_strLocalIP,m_strMulticastIP;
private :
    void _Init();//初始化成员
    void _QueryNowWeather();//天气获取
    void _QueryDayWeather();
    void _ReadConfig();//有效期
    void _InitTenement();//初始化设备链表
    void _InitMiddle();

    int  _Read();//接收数据
    void _Recv(unsigned char *buf,char *pFromIP,int nRecvLen);
    int _Sendto(QString strIp,unsigned char *send,int nSendlen);//发送
    //设备在线
    void _DeviceOnline(unsigned char *buf,char *pFromIP);
    void _AskDeviceOnline(unsigned char *buf,char *pFromIP);

    void _RecvDownAloneCard(unsigned char *buf,char *pFromIP);//未使用
    void _RecvUpCard(unsigned char *buf,char *pFromIp);
    void _RecvDownCard(unsigned char *buf,char *t_IP);
    void _RecvDownAllCard(unsigned char *buf,char *t_IP);
    void _RecvLogOutCard(unsigned char *buf,char *pFromIP);
    void _AskRecvUpCard(unsigned char *buf,char *pFromIP);
    //发送信息回复
    void _RecvSendMessage(unsigned char *buf,char *pFromIP);
    //初始化密码回复
    void _RecvInitPass(unsigned char *buf,char *pFromIP);

    //设备报警
    void _DeviceAlarm(unsigned char *buf,char *pFromIP);
    void _AskAlarm(unsigned char *buf,char *pFromIP);
    //门口机报警
    void _MiddleAlarm(unsigned char *buf,char *pFromIP);
    void _AskMiddleAlarm(unsigned char *buf,char *pFromIP);

    //取消报警
    void _CancelAlarm(unsigned char *buf,char *pFromIP);
    void _AskCancelAlarm(unsigned char *buf,char *pFromIP);
    //刷卡
    void _SwipeCard(unsigned char *buf,char *pFromIP);
    //报修
    void _Repairs(unsigned char *buf,char *pFromIP);
    void _AskRepairs(unsigned char *buf,char *pFromIP);
    //取消报修
    void _CancelRepairs(unsigned char *buf,char *pFromIP);
    void _AskCancelRepairs(unsigned char *buf,char *pFromIP);
    //地址转换
    void _IntraToAddr(char *pIntra,char *pAddr,char *pType);
    void _IntraToAddrExplain(QString strIntra,char *pAddrExplain);
    //读写地址
    void _ReadAddr(unsigned char *buf,char *pFromIP);
    void _WrtieAddr(unsigned char *buf,char *pFromIP);
    //新闻视频下发
    int _Begin(QString strIp);
    int _FileMsg(unsigned char *pSend);
    void _Fail(QString strIp);
    void _SendNext();
    int _SendData(unsigned char *buf,char *pFromIP);
    int _End(unsigned char *buf,char *pFromIP);
    int _EndAll(unsigned char *buf,char *pFromIP);

    void _UpCallPic(unsigned char *buf,char *pFromIP);

};

#endif // UDPSERVER_H
