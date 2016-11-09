#ifndef SERVERCENTER_H
#define SERVERCENTER_H

#include <QMainWindow>
#include <QtGui\QMainWindow>
#include <QList>
#include <phonon>
#include <QtSql>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QCloseEvent>
#include <Windows.h>
#include "udpsend.h"
#include "udprecv.h"
#include "syncmssql.h"
#include "STRUCT.h"
//#include "udpserver.h"
#include "update.h"
#include "phone.h"
#include "repairs.h"
#include "dealalarm.h"
#include "user.h"
#include "delete.h"
#include "dealrepairs.h"
#include "find.h"
#include "finddevice.h"
#include "lock.h"
#include "about.h"
#include "card.h"
#include "findcard.h"
#include "weather.h"
#include "smartcontrol.h"
#include "setodbc.h"
#include "connectionpoolmysql.h"
#include "connectionpoolmssql.h"
#include "pic.h"

namespace Ui {
    class CSERVERCENTER;
}

class CSERVERCENTER : public QMainWindow
{
    Q_OBJECT

public:
    explicit CSERVERCENTER(SUser sUser,QWidget *parent = 0);
    ~CSERVERCENTER();
    void _Test();       //数据库事务操作
    void _UpdateLoading(SUser sUser);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void closeEvent( QCloseEvent * event );
    void paintEvent(QPaintEvent *event);
    void resizeEvent( QResizeEvent* event);

private slots:
    /*********************定时器***************************/
    void _SendCard();//自动检测下发IC卡
    void _CheckClash();//检查冲突
    void _ColorChange();//报警红色显示
    void _FocusChange();//焦点
    void timerEvent( QTimerEvent *event );//时间

    void SlotDateTime(int nFlag,int nDay);//注册时间监测
    void SlotUdpOpenFail();//端口冲突

    /**********************室内设备**************************/
    void SlotReset();//重启设备
    void SlotRecvReset();//收到回复重启
    void SlotInitTenement();//初始化
    void SlotUpdateTenement(QVariant data);//其他更改
    void SlotOnlineTenement(int Id,int Alarm);//掉线

    void SlotUpdate();//修改地址#
    void SlotRecvInitPass(QString strAddr,int nFlag);//接收到初始化密码的回复
    void SlotPingIp();//网络连接测试
    void SlotSmartControl();//智能家居
    void SlotSendLamp(char *buf,QString strIp);//发送灯控制请求信号
    void SlotDeleteDevice();//删除设备
    void SlotFindMiddle();//查找设备界面
    void SlotMiddleFind(int nIndex,QString strFind);//查找设备
    void SlotInitSystemPass();//初始化系统密码
    void SlotInitSystemPassA8();//初始化系统密码
    void SlotRead(char *pAddr,QString strIp);//读地址#
    void SlotWrite(char *pBuf,QString strIp);//写地址#
    void SlotRecvRead(unsigned char *pBuf,QString strIp);//读回复#
    void SlotRecvRead_A8(unsigned char *pBuf,QString strIp);//读回复#
    void SlotRecvWrite(unsigned char *buf,QString strIp);//写回复#
    void SlotUpdateMac();//写地址mac地址改变更新#
    void SlotUpdateAddr();//写地址成功信号#

    /**********************中间设备**************************/
    void SlotInitMiddle();//初始化
    void SlotUpdateMiddle(QVariant data);//更新
    void SlotOnlineMiddle(int Id);//掉线
    void SlotInitLockPass();//初始化开门密码
    void SlotInitLockPassA8();//初始化开门密码

    /**********************普通信息***************************/

    /**********************物业信息**************************/
    void SlotUpdateRepairs();//修改报修类型
    void SlotDeleteRepairs();//删除报修类型
    void SlotAddRepairs();//添加报修
    void SlotRepairs();//增删改成功后信号
    void SlotDownRepairs();//下发报修类型
    void SlotDownRepairsA8();//下发报修类型A8
    void SlotUpdatePhone();//更新电话
    void SlotDeletePhone();//删除电话
    void SlotAddPhone();//新增电话
    void SlotDownPhone();//下发常用电话
    void SlotPhone();//增删改成功后信号

    /**********************重要新闻**************************/
    void Slotbtn(int Flag);//新闻发送过程中按键
    void SlotSendMessage(int Id,int Result);//发送结果
    void SlotNewsDonw(int Id,int All,int Finfish);//新闻下载结果

    /**********************系统日志**************************/
    void SlotSystemFind();      //查找
    void SlotSystemDelete();    //删除

    /**********************设备日志**************************/
    void SlotDeviceFind();      //查找
    void SlotDeviceDelete();    //删除

    /**********************刷卡日志**************************/
    void SlotCardRecordFind();      //查找
    void SlotCardRecordDelete();    //刷卡记录右键响应函数

    /**********************报警记录***************************/
    void SlotAlarmRecordFind();     //查找
    void SlotAlarmRecordDelete();   //删除

    void SlotAlarmDeal();       //处理报警
    void SlotDealAll();     //管理员一键全部处理报警或报修

    /**********************报修记录***************************/
    void SlotRepairsRecord();       //报修记录插入
    void SlotRepairsRecordFind();   //查找
    void SlotRepairsRecordDelete(); //删除
    void SlotRepairsRecordDeal();   //处理
    void SlotDealRepairs();         //处理结果

    /**********************通话记录***************************/
    void SlotCallRecordDelete();    //通话记录删除

    /**********************IC卡信息**************************/
    void SlotRefurbish(QString strCardNum);//刷新界面
    void SlotUpdateTime(QString strCardNum);//修改有效期
    void SlotUpCard();      //上传卡
    void SlotDownCard();    //注册卡
    void SlotDownAllCard(); //下载所有卡

    void SlotLogOutCard();//注销卡
    void SlotRecvDownAloneCard(int nAllCard);//未使用
    void SlotCardMessageSave();//修改权限
    void SlotRecvLogOutCard(unsigned char *buf,QString strIp);//注销卡回复
    void SlotRecvDownCard(unsigned char *buf,QString strIp);//注册卡回复

    void SlotRecvDownAllCard(unsigned char *buf,QString strIp);//全部下载回复

    void SlotRecvDownCard1(unsigned char *buf,QString strIp);   //未使用
    void SlotRecvUpCard(unsigned char *buf);    //上传卡回应
    void SlotIcCarUpdate();     //更新
    void SlotIcCarDelete();     //删除
    void SlotIcCarAdd();        //新增
    void SlotIcCarFind();       //查找界面

    /**********************A8IC卡**************************/
    void SlotUpCard_A8();     //上传卡A8
    void SlotRecvUpCard_A8(unsigned char *buf);   //上传卡回应A8
    void SlotDownCard_A8();   //下载卡A8
    void SlotRecvDownCard_A8(unsigned char *buf, QString strIp);    //写IC卡A8
    //void SlotRecvDownAllCard_A8(unsigned char *buf, QString strIp);

    void SlotSaveUpCard();
    void SlotSaveAllUpCard();
    void SlotClearAllCard();
    //查找卡响应
    void SlotFindCard(int nFlag,QString strCard,QString strTAddr,QString strMAddr,QString strTime);

    /**********************操作员管理**************************/
    void SlotUserUpdate();      //更新
    void SlotUserDelete();      //删除
    void SlotUserAdd();         //新增
    void SlotUpdateUser();      //增删改成功后信号

    /**********************设备报警**************************/
    void SlotAboutToFinish();
    void SlotInsertAlarm();     //报警记录插入
    void SlotDealAlarm(int nAlarmType,QString strStime,QString strAlarmStatu,QString strDealHuman,QString strMessage);//处理结果
    void SlotDeal();        //处理实时报警
    void SlotStop();

    void SlotUpdateCallRecord();
    //记录和日志删除和查找
    void SlotDelete();
    void SlotFind(int nIndex,int nTime,QString strFind,QString strStime,QString strEtime);//记录和日志的查找

    void SlotUpdateCity(QString strCityNum,QString strCity);
    void SlotUpdateWeatherNew();

    /**********************翻页***************************/
    void SlotPreviousPage();
    void SlotNextPage();
    void _UpdatePage(int nType);

    //菜单栏
    void SlotUpdateTreeIC();
    void SlotUpdateTreeTenement();
    void SlotUpdateTreeMiddle();
    void SlotUpdateWeather();       //设置天气
    void SlotUpdateWeather(int MaxTemp,int MinTemp,int Weather);    //设置天气
    void SlotUpdateDBSet();
    void SlotUpdateTreeQuit();
    void SlotUpdateTreeMessage();
    void SlotUpdateTreeRepairsmessage();
    void SlotUpdateTreeNews();
    void SlotUpdateTreeRepairs();
    void SlotUpdateTreeAlarm();
    void SlotUpdateTreeSystem();
    void SlotUpdateTreeDevice();
    void SlotUpdateTreeCard();
    void SlotUpdateTreeCallRecord();    //呼叫记录
    void SlotUpdateTreeManage();
    void SlotUpdateTreeAbout();
    void SlotUpdateTreeHelp();
    void SlotUpdateTreeLock();
    void SlotUpdateTreeLogout();

    void SlotUnLock();
    void SlotDeleteCard();

    void SlotCheck(int row,int col);//选择项发生变化——（普通信息 新闻信息 开门权限）
    void _test(unsigned char* buf);     //？？？

    void SlotDisConnect();

private slots:
    void on_QTree_clicked(const QModelIndex &index);

    void on_QchkTenement_activated(int index);
    void on_QchkMiddle_activated(int index);

    void on_QpbtnCheckAllT_clicked();
    void on_QpbtnCheckOnlineT_clicked();
    void on_QpbtnCheckUnselectT_clicked();
    void on_QpbtnCheckInvertT_clicked();
    void on_QpbtnSendT_clicked();

    void on_QpbtnBrowse_clicked();
    void on_QpbtnSendM_clicked();
    void on_QpbtnCheckAllM_clicked();
    void on_QpbtnCheckOnlineM_clicked();
    void on_QpbtnCheckUnselectM_clicked();
    void on_QpbtnCheckInverM_clicked();

    void on_QTenement_customContextMenuRequested(const QPoint &pos);
    void on_QMiddle_customContextMenuRequested(const QPoint &pos);
    void on_QRepairs_customContextMenuRequested(const QPoint &pos);
    void on_QPhone_customContextMenuRequested(const QPoint &pos);
    void on_QIcCard_customContextMenuRequested(const QPoint &pos);
    void on_QCardMessage_customContextMenuRequested(const QPoint &pos);
    void on_QUser_customContextMenuRequested(const QPoint &pos);
    void on_QRepairsRecord_customContextMenuRequested(const QPoint &pos);
    void on_QAlarmRecord_customContextMenuRequested(const QPoint &pos);
    void on_QCardRecord_customContextMenuRequested(const QPoint &pos);
    void on_QDevice_customContextMenuRequested(const QPoint &pos);
    void on_QSystem_customContextMenuRequested(const QPoint &pos);
    void on_QAlarm_customContextMenuRequested(const QPoint &pos);
    void on_QCallRecord_customContextMenuRequested(const QPoint &pos);

    void on_QtextMessage_textChanged();
    void IconActivated(QSystemTrayIcon::ActivationReason reason);
    void SlotShowManager();
    void on_QIcCard_clicked(const QModelIndex &index);//IC卡选中换行

    void on_QpbtnExplain_clicked();

/**********************A8IC卡**************************/
    void on_Qpbtn_IC_Register_clicked();//注册
    void on_QpbtnAdd_clicked();
    void on_QpbtnMode_clicked();
    void on_Empower_Indoor_clicked(const QModelIndex &index);

    void on_Qpbtn_Empower_clicked();//授权
    void on_QpbtnAuthorization_clicked();
    void on_Register_Indoor_clicked(const QModelIndex &index);

    void on_Qpbtn_lssued_clicked();//下发
    void on_Issued_Machine_clicked(const QModelIndex &index);
    void on_Empower_IC_customContextMenuRequested(const QPoint &pos);   //删除此卡
    void on_Upload_IC_customContextMenuRequested(const QPoint &pos);    //保存此卡
    void on_QpbtnDown_clicked();
    void on_QpbtnUp_clicked();

/**********************呼叫记录**************************/
    void on_QrbtnTime_clicked();
    void on_QpbtnFind_clicked();

    void on_QchkType_currentIndexChanged(int index);

    void on_Qpbtn_CallPhoto_clicked();    //暂时不知道功能用法，所以暂不实现

    void on_QCallRecord_clicked(const QModelIndex &index);
    void on_QCallNow_clicked(const QModelIndex &index);

private:
    Ui::CSERVERCENTER *ui;
    CUDPSEND *CUdpSend;
    CUDPRECV *CUdpRecv;
    CSYNCMSSQLTENEMENT *CSynMsSqlTenement;
    CSYNCMSSQLMIDDLE *CSynMsSqlMiddle;
    CSYNCMSSQLALARMRECORD *CSynMsSqlAlarmRecord;
    CSYNCMSSQLREPAIRSRECORD *CSynMsSqlRepairsRecord;
    CSYNCMSSQLCARDRECORD *CSynMsSqlCardRecord;
    CSYNCMSSQLICCARD *CSynMsSqlIcCard;
    CSYNCMSSQLDELETEDEVICE *CSynMsSqlDeleteDevice;
    CSYNCMSSQLDELETECARD *CSynMsSqlDeleteCard;

    CUPDATE *CUpdate;
    CPHONE *CPhone;
    CREPAIRS *CRepairs;
    CDEALALARM *CDealAlarm;
    CUSER *CUser;
    CDELETE *CDelete;
    CDEALREPAIRS *CDealRepairs;
    CFIND *CFind;
    CFINDDEVICE *CFindDevice;
    CLOCK *CLock;
    CABOUT *CAbout;
    CCARD *CCard;
    CFINDCARD *CFindCard;
    CWEATHER *CWeather;
    CSETODBC *CSetOdbc;
    CSMARTCONTROL *CSmartControl;
    CPIC *CPic;

    Phonon::MediaObject *m_MediaObject;//播放对象
    Phonon::AudioOutput *m_AudioOutput;//音频输出
    QList<Phonon::MediaSource> m_ListSources;//播放列表
    QList<int> m_ListSelected;
    SUser m_sUser;
    NodeCard *m_ItemCard;
    QTimer *m_Timer;
    QByteArray m_Byte;
    QString m_strIcCard, m_strFindIcCard;
    int m_nAllUpCard;
    int m_nMusicId;
    int m_nTimerId;
    int m_nTenementIndex;
    int m_nIndex;
    int m_nColor_state;
    int m_nUdp;
    int m_nFind;
    int m_nCount;
    int m_nMode;
    int m_nPage;
    int m_nFindDevice;
    QString m_strFindSql;
    QList<QString> m_List;
    QMap <QString,QString> m_AddrofCardMap;
    QMap <QString,QString>::iterator tt;
    bool isHeadItem;
    QString MsDis;
    QString MyDis;

    QMenu *systemMenu;
    QMenu *noteMenu;
    QMenu *recordMenu;
    QMenu *daminMenu;
    QMenu *helpMenu;

    QMenu *TenementMenu;
    QMenu *MiddleMenu;
    QMenu *RepairsMenu;
    QMenu *PhoneMenu;
    QMenu *IcCardMenu;
    QMenu *CardMessageMenu;
    QMenu *UserMenu;
    QMenu *RepairsRecordMenu;
    QMenu *AlarmRecordMenu;
    QMenu *CardRecordMenu;
    QMenu *DeviceMenu;
    QMenu *SystemMenu;
    QMenu *AlarmMenu;
    QMenu *CallRecordMenu;

//托盘
    QSystemTrayIcon *TrayIcon;
    QMenu *TrayIconMenu;

    QAction *MinimizeAction;
    QAction *MaximizeAction;
    QAction *RestoreAction;
    QAction *TrayQuitAction;

    QAction *PingIpAction;
    QAction *SmartControlAction;//家居
    QAction *UpdateAction;
    QAction *DeleteAction;
    QAction *InitSystemPassAction;
    QAction *InitLockPassAction;
    QAction *FindAction;
    QAction *ResetAction;

    QAction *DealAllAction;
    QAction *RepairsUpdateAction;
    QAction *RepairsDeleteAction;
    QAction *RepairsAddAction;
    QAction *RepairsDownAction;
    QAction *PhoneUpdateAction;
    QAction *PhoneDeleteAction;
    QAction *PhoneAddAction;
    QAction *PhoneDownAction;
    QAction *IcCarUpdateAction;
    QAction *IcCarDeleteAction;
    QAction *IcCarAddAction;
    QAction *IcCarFindAction;
    QAction *CardMessageUpAction;
    QAction *CardMessageSaveAction;
    QAction *CardMessageDownAction;
    QAction *CardMessageDownAllAction;
    QAction *CardLogOutAction;
    QAction *UserUpdateAction;
    QAction *UserDeleteAction;
    QAction *UserAddAction;

    QAction *RepairsRecordFindAction;
    QAction *RepairsRecordDeleteAction;
    QAction *RepairsRecordDealAction;
    QAction *AlarmRecordFindAction;
    QAction *AlarmRecordDeleteAction;
    QAction *AlarmDealAction;
    QAction *CardRecordFindAction;
    QAction *CardRecordDeleteAction;
    QAction *CallRecordDeleteAction;    //通话记录删除
    QAction *DeviceFindAction;
    QAction *DeviceDeleteAction;
    QAction *SystemFindAction;
    QAction *SystemDeleteAction;
    QAction *DealAction;
    QAction *StopAction;

    QAction *NextPageAction;    //下一页
    QAction *PreviousPageAction;//上一页
    QAction *ICAction;          //IC卡
    QAction *TenementAction;    //住户
    QAction *MiddleAction;      //中间
    QAction *WeatherAction;     //天气设置
    QAction *DBSetAction;       //数据源设置
    QAction *QuitAction;        //退出
    QAction *MessageAction;     //普通信息
    QAction *RepairsmessageAction;//物业信息
    QAction *NewsAction;        //新闻
    QAction *RepairsAction;     //报修
    QAction *AlarmAction;       //报警
    QAction *SystemAction;      //系统
    QAction *DeviceAction;      //设备
    QAction *CardAction;        //刷卡记录
    QAction *ManageAction;      //操作
    QAction *CallRecordAction;  //呼叫记录
    QAction *AboutAction;       //关于
    QAction *HelpAction;        //帮助
    QAction *LockAction;        //锁定
    QAction *LogoutAction;      //注销

    QAction *DeleteCardAction;
    QMenu   *DeleteCardMenu;

    QAction *SaveCardAction;
    QAction *SaveAllCardAction;
    QAction *ClearAllCardAction;
    QMenu   *AddCardMenu;

    QStatusBar *statusbar;
    QLabel *statulabe6;
    QLabel *statulabe4;
    QLabel *statulabe2;
    QLabel *statulabe8;

    QDir dir;
    QLabel *MSSQLIcon;
    QLabel *MySQLIcon;

private:
    /************************初始化*********************/
    void _Init();
    void _InitUI();
    void _InitTray();
    void _Response();
    void _InitClass();
    void _InitMusic();
    void _InitTimer();
    void _ClearItem();
    void _CreateMenus();
    void _SetStyleSheet();
    void _CreateActions();
    void _InsertItem(SCard data);
    void _StatusBar(SUser sUser);

/***********************目录*************************/
    void _InitTree();

/**********************室内设备**************************/
    void _InitTenement();
    void _UpdateFindTenement(NodeTenement *ItemTenement,int nIndex,QString strFind);
    void _UpdateTenement(NodeTenement *ItemTenement);
    void _TenementOnline(NodeTenement *ItemTenement);
    void _TenementNotOnline(NodeTenement *ItemTenement);
    void _TenementIndoor(NodeTenement *ItemTenement);
    void _TenementVillaIndoor(NodeTenement *ItemTenement);
    void _OneUpdateTenement(QVariant dataVar);

/**********************中间设备**************************/
    void _InitMiddle();
    void _UpdateFindMiddle(NodeMiddle *ItemMiddle,int nIndex,QString strFind);
    void _UpdateMiddle(NodeMiddle *ItemMiddle);
    void _MiddleOnline(NodeMiddle *ItemMiddle);
    void _MiddleNotOnline(NodeMiddle *ItemMiddle);
    void _MiddleM(NodeMiddle *ItemMiddle);
    void _MiddleH(NodeMiddle *ItemMiddle);
    void _MiddleP(NodeMiddle *ItemMiddle);
    void _MiddleW(NodeMiddle *ItemMiddle);
    void _MiddleZ(NodeMiddle *ItemMiddle);
    void _OneUpdateMiddle(QVariant dataVar);

/**********************普通信息***************************/
    void _InitMessage();
    void _UpdateMessage(NodeTenement *ItemTenement);
    void _UpdateMessageResult();//结果清空

/**********************物业信息**************************/
    void _InitPhone();
    void _InitRepairs();
    void _UpdatePhone();
    void _UpdateRepairs();

/**********************重要新闻**************************/
    void _InitNews();
    void _UpdateNews(NodeMiddle *ItemMiddle);
    void _ClearNewsResult();

/**********************系统日志**************************/
    void _InitSystem();
    void _UpdateSystem();
    void _UpdateFindSystem(QString  SQL);

/**********************设备日志**************************/
    void _InitDevice();
    void _UpdateDevice();
    void _UpdateFindDevice(QString  SQL);

/**********************刷卡日志**************************/
    void _InitCardRecord();
    void _UpdateCardRecord();
    void _UpdateFindCardRecord(QString  SQL);

/**********************报警记录***************************/
    void _InitAlarmRecord();
    void _UpdateAlarmRecord();
    void _UpdateFindAlarmRecord(QString  SQL);

/**********************报修记录***************************/
    void _InitRepairsRecord();
    void _UpdateRepairsRecord();
    void _UpdateFindRepairsRecord(QString  SQL);

/**********************IC卡信息**************************/
    void _InitIcCard();
    void _InitCardMessage();
    void _UpdateIcCard();
    void _UpdateCardMessage();
    void _CheckCardMessage();
    bool _GetSelectedRow(QList<int> *set_row,int nfalg);

/**********************操作员管理**************************/
    void _InitUser();
    void _UpdateUser();

/**********************报警信息**************************/
    void _InitAlarm();
    void _UpdateTenementToAlarm();
    void _UpdateAlarm(NodeAlarm *ItemAlarm);

    void _UpdateChange(int row,int check);//更改选中状态值

/**********************添加卡信息**************************/
    void _InitEmpowerIC();
    void _UpdateEmpowerIndoor(NodeTenement *ItemTenement);
    void _UpdateEmpowerIC(QString strAddr);

/**********************授权卡信息**************************/
    void _InitRegisterMachine();
    void _InitRegisterIC();
    void _UpdateRegisterIndoor(NodeTenement *ItemTenement);
    void _UpdateRegisterMachine(NodeMiddle *ItemMiddle);
    void _UpdateRegisterIC(QString strAddrExplain);

/********************下发卡信息************************/
    void _InitIssuedMachine();
    void _UpdateIssuedMachine(NodeMiddle *ItemMiddle);

/**********************通话记录**************************/
    void _InitCallNow();
    void _InitCallRecord();
    void _UpdateCallNow();
    void _UpdateCallRecord(QString strSQL = "",int t = 0);

    QString _AddrToAddrExplain(QString Addr,QString strType);
    QString _AddrToAddrExplainA8(QString Addr,QString strType);
    QString _AddrExplainToAddr(QString AddrExplain,QString strType,int n);
    QString _AddrExplainToAddr(QString AddrExplain,QString strType);
    QString _AddrExplainToAddrA8(QString AddrExplain,QString strType,int n);

//    QString _AddrToAddrExplain(QString Addr,QString strType);
//    QString _AddrToAddrExplainA8(QString Addr,QString strType);
//    QString _AddrExplainToAddr(QString AddrExplain,QString strType,int n);
//    QString _AddrExplainToAddrA8(QString AddrExplain,QString strType,int n);

    void _SetTableWidgetSatus(int IcOperateType);

//    void changeEvent(QEvent* event);

signals :
    void SigLoading();
    void ShowManager();
    void Sigquit();
    void SigRefurbish(QString strCard);
};

#endif // SERVERCENTER_H
