#ifndef STRUCT_H
#define STRUCT_H
#include <QtGui>

typedef unsigned long long	U64;
typedef signed long long	I64;
typedef unsigned long		U32;
typedef signed long		I32;
typedef unsigned short 		U16;
typedef signed short 		I16;
typedef          char		I8;
typedef unsigned char		U8;
#define RECV_SIZE 8100
#define ONLINE_TIME 20
#define ASK_MESSAGE_TIME 15
#define CARD_MAXLEN 5
#define NID 100
#define NTIME 10
#define PAGE_MAXLEN 400

struct SUser{
    QString strName;//姓名
    QString strUser;//用户名
    QString strPass;//密码
    QString strCity;//城市
    QString strCityNum;//城市编码
    U16 nGrade;//权限
    int protocol;   //协议
};

struct SWeather
{
    QString strName;
    QString strWindDirection;//风向
    U16 nWeather;//天气,1晴2多云3雨4雪
    U16 nWind;//风力
    I16 nMinTemp;//
    I16 nMaxTemp;//
    I16 nNowTemp;//
    U16 nHumidity;//湿度
};

struct SMiddleDevice    //中间设备
{
    U16 nCheckedState:1;//选中状态
    U16 nAskTime:3;
    U16 nCardState:4;//ic卡下发状态  0为初始值 1为需要下发 2为已经下发成功
    U16 nState;//连接状态
    U32 nId;
    I8 gcIntraAddr[20];//内部地址
    I8 gcOldAddr[20];//用来判断是否添加新字段到开门权限
    I8 gcAddr[20];//地址
    I8 gcMacAddr[20];//网卡地址
    I8 gcIpAddr[20];//IP地址
    I8 gcType[20];//终端类型
    I8 gcAddrExplain[64];//地址说明
};

Q_DECLARE_METATYPE(SMiddleDevice)

struct SAlarmDevice
{
    U16 nFenceId:4;//报警防区
    U16 nAlarmType;//报警类型
    U32 nId;//编号
    U32 nTenementId;//对应在线表ID
    I8 gcIntraAddr[20];//内部地址
    I8 gcPhone1[20];//电话
    I8 gcMacAddr[20];//网卡地址
    I8 gcIpAddr[20];//IP地址
    I8 gcStime[20];
    I8 gcEtime[20];
    I8 gcType[20];
    I8 gcAddr[64];//地址
    I8 gcName[64];//名字
    U16 FenceNum;   //防区模块个数
};

struct STenementDevice
{
    U16 nAlarmState:1;//报警状态
    U16 nCheckedState:1;//选中状态
    U16 nFenceState:2;//布防状态
    U16 nOldFenceState:2;
    U16 nAskTime:4;
    U16 nState:5;//连接状态
    U32 nId;//编号
    I8 gcIntraAddr[20];//内部地址
    I8 gcAddr[20];//地址
    I8 gcPhone1[20];//电话
    I8 gcPhone2[20];
    I8 gcMacAddr[20];//网卡地址
    I8 gcIpAddr[20];//IP地址
    I8 gcType[20];//终端类型
    I8 gcAddrExplain[40];//地址说明
    I8 gcName[64];//名字
};
Q_DECLARE_METATYPE(STenementDevice)

typedef struct NodeAlarm
{
    SAlarmDevice data;              //单链表中的数据域
    struct NodeAlarm *next;          //单链表的指针域
}NodeAlarm,*ItemAlarm;

typedef struct NodeMiddle
{
    SMiddleDevice data;              //单链表中的数据域
    struct NodeMiddle *next;          //单链表的指针域
}NodeMiddle,*ItemMiddle;

typedef struct NodeTenement
{
    STenementDevice data;              //单链表中的数据域
    struct NodeTenement *next;          //单链表的指针域
}NodeTenement,*ItemTenement;

struct SCard
{
    I8 gcAddr[20];
    int nOldCheck;
    int nNewCheck;
};

typedef struct NodeCard
{
    SCard data;              //单链表中的数据域
    struct NodeCard *next;          //单链表的指针域
}NodeCard,*ItemCard;

#endif // STRUCT_H
