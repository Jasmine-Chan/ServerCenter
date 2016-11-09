#include "servercenter.h"
#include "ui_servercenter.h"

CSERVERCENTER::CSERVERCENTER(SUser sUser,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CSERVERCENTER)
{
    ui->setupUi(this);

    m_nIndex = 0;
    m_nUdp = 0;
    m_nMode = 0;
    m_nFind = 0;
    isHeadItem = true;
    m_nFindDevice = 0;  //???
    m_sUser = sUser;
    MyDis = "red1.png";
    MsDis = "red1.png";

    _Init();        //初始化
    ui->stackedWidget->setCurrentIndex(0);

    ui->QlineICCard->installEventFilter(this);
    ui->QlineICCard->setInputMask("hh:hh:hh:hh");
    ui->Qpbtn_CallPhoto->setFlat(true);
}

CSERVERCENTER::~CSERVERCENTER()
{
    delete ui;
}

bool CSERVERCENTER::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->QlineICCard)
    {
        if((ui->QlineICCard->text().length() == 11) && (m_nMode == 0) && ((ui->Empower_Indoor->currentColumn() >= 0)))
        {
            QString strICCard = ui->QlineICCard->text();
            if(ui->Empower_Indoor->currentItem()->text(0).right(2)== "设备" && ui->Empower_Indoor->currentItem()->parent()->text(0).right(2) == "单元" )
            {
                QString strAddExplain;
                strAddExplain = ui->Empower_Indoor->currentItem()->parent()->text(0) + ui->Empower_Indoor->currentItem()->text(0);
                CMYSQL CMysql;
                if(CMysql._SelectIcCardA8(ui->QlineICCard->text()) == 0)
                {
                    if(CMysql._AddIcCardA8(ui->QlineICCard->text(),_AddrExplainToAddrA8(strAddExplain,"室内机",1),QDate(QDate::currentDate().addYears(2)).toString("yyyy-MM-dd")))
                    {
                        _UpdateEmpowerIC(strAddExplain);
                        ui->QlineICCard->clear();
                        return true;
                    }
                    else
                    {
                        ui->QlineICCard->clear();
                        QMessageBox::information(this,"错误","添加失败，请检查数据库");
                        return false;
                    }
                }
                else
                {
                    ui->QlineICCard->clear();
                    QMessageBox::information(this,tr("提示"),tr("卡号已存在"));
                    return false;
                }
            }
            else
                ui->QlineICCard->clear();
                QMessageBox::information(this,"错误","请选择到室内机的设备号");
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return eventFilter(obj, event);
    }
}

//数据库事务操作
void CSERVERCENTER::_Test()
{
    QSqlDatabase db2 = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlDatabase db1 = CCONNECTIONPOOLMSSQL::openConnection();
    if(QSqlDatabase::database().driver()->hasFeature(QSqlDriver::Transactions))
    {
        //先判断该数据库是否支持事务操作
        if(QSqlDatabase::database().transaction()) //启动事务操作
        {
            QSqlQuery queryMY(db2);
            QString SQL1 = "insert into test values (15,12,2)";
            int i1 = queryMY.exec(SQL1);

            QSqlQuery queryMS(db1);

            QString SQL = "delete from test where id = 12";
            int i = queryMS.exec(SQL);

            CCONNECTIONPOOLMYSQL::closeConnection(db2);
            CCONNECTIONPOOLMSSQL::closeConnection(db1);
            qDebug()<<i<<i1<<"commit";

            if(i&&i1)
            {
                QSqlDatabase::database().commit();
            }
            else
            {
                QSqlDatabase::database().rollback();
            }
        }
    }
}

//总初始化
void CSERVERCENTER::_Init()
{
    _InitUI();          //界面初始化
    _InitTray();        //系统托盘初始化
    _InitMusic();       //音乐初始化
    _InitClass();       //类初始化
    _InitTimer();       //定时器初始化
    _SetStyleSheet();   //设置样式表
    _CreateActions();   //动作
    _CreateMenus();     //右键菜单
    _Response();        //信号槽连接
    _StatusBar(m_sUser);//设置状态栏
    CUdpSend->start();  //启动线程
    CUdpRecv->start();  //启动线程
    CUdpSend->_SendIdToServer();    //发送数据到19服务器
    m_ItemCard = (NodeCard *)malloc(sizeof(NodeCard));
    m_ItemCard ->next = NULL;
}

//定时器初始化
void CSERVERCENTER::_InitTimer()
{
    m_nTimerId = startTimer(1000);//刷新系统时间

    QTimer *Timer = new QTimer();
    connect( Timer,SIGNAL(timeout()), this, SLOT(_ColorChange()) );
    Timer->start(1000);

    QTimer *Timer1 = new QTimer();
    connect( Timer1,SIGNAL(timeout()), this, SLOT(_CheckClash()) );
    Timer1->start(1000);

    QTimer *Timer2 = new QTimer();
    connect( Timer2,SIGNAL(timeout()), this, SLOT(_FocusChange()) );

    m_Timer = new QTimer();
    connect( m_Timer,SIGNAL(timeout()), this, SLOT(_SendCard()) );
//    Timer2->start(100);

    QTimer *MS_Timer = new QTimer();
    MS_Timer->start(3000);
    connect(MS_Timer, SIGNAL(timeout()), this, SLOT(SlotDisConnect()));
}

//系统托盘初始化
void CSERVERCENTER::_InitTray()
{
    QIcon icon = QIcon(":/tu.png");
    TrayIcon = new QSystemTrayIcon(this);
    TrayIcon->setIcon(icon);
    TrayIcon->setToolTip("中心管理系统");
    TrayIcon->show();
}

//UI界面初始化
void CSERVERCENTER::_InitUI()
{
    QIcon icon = QIcon(":/tu.png");
    setWindowIcon(icon);
    setWindowTitle(tr("中心管理系统"));

    _InitTree();
    _InitTenement();
    _InitMiddle();
    _InitAlarm();
    _InitAlarmRecord();
    _InitRepairsRecord();
    _InitIcCard();
    _InitCardRecord();
    _InitUser();
    _InitSystem();
    _InitDevice();
    _InitMessage();
    _InitNews();
    _InitRepairs();
    _InitPhone();
    _InitCardMessage();
    _InitCallRecord();
    _InitCallNow();
    _InitEmpowerIC();
    _InitRegisterIC();
    _InitRegisterMachine();
    _InitIssuedMachine();

    ui->QlblAllName->hide();
    ui->QlblAll->clear();
    ui->QlblClashName->hide();

    if(m_sUser.protocol != 0)
    {
        ui->QCallNow->hide();
        ui->QlblPic->hide();
    }
}

//音乐初始化
void CSERVERCENTER::_InitMusic()
{
    m_MediaObject = new Phonon::MediaObject(this);
    m_AudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    Phonon::createPath(m_MediaObject, m_AudioOutput);

    QString path;
    QDir dir;
    path = dir.currentPath();
    Phonon::MediaSource tSource1(path + "/music/alarm.mp3");
    m_ListSources.append(tSource1);
    Phonon::MediaSource tSource2(path + "/music/repairs.mp3");
    m_ListSources.append(tSource2);
    m_nMusicId = 0;
}

//类初始化
void CSERVERCENTER::_InitClass()
{
    CUdpSend = new CUDPSEND(m_sUser);
    CUdpRecv = new CUDPRECV(m_sUser);
    CUpdate = new CUPDATE();
    CPhone = new CPHONE();
    CRepairs = new CREPAIRS();
    CDealAlarm = new CDEALALARM();
    CUser = new CUSER();
    CDelete = new CDELETE();
    CDealRepairs = new CDEALREPAIRS();
    CFind = new CFIND();
    CFindDevice = new CFINDDEVICE();
    CLock = new CLOCK();
    CAbout = new CABOUT();
    CCard = new CCARD();
    CFindCard = new CFINDCARD();
    CWeather = new CWEATHER();
    CSmartControl = new CSMARTCONTROL();
    CSetOdbc = new CSETODBC();
    CPic = new CPIC();

    CSynMsSqlTenement = new CSYNCMSSQLTENEMENT();
    CSynMsSqlMiddle = new CSYNCMSSQLMIDDLE();
    CSynMsSqlAlarmRecord = new CSYNCMSSQLALARMRECORD();
    CSynMsSqlRepairsRecord = new CSYNCMSSQLREPAIRSRECORD();
    CSynMsSqlCardRecord = new CSYNCMSSQLCARDRECORD();
    CSynMsSqlIcCard = new CSYNCMSSQLICCARD();
    CSynMsSqlDeleteDevice = new CSYNCMSSQLDELETEDEVICE();
    CSynMsSqlDeleteCard = new CSYNCMSSQLDELETECARD();

    CSynMsSqlTenement->start();
    CSynMsSqlMiddle->start();
    CSynMsSqlAlarmRecord->start();
    CSynMsSqlRepairsRecord->start();
    CSynMsSqlCardRecord->start();
    CSynMsSqlIcCard->start();
    CSynMsSqlDeleteDevice->start();
    CSynMsSqlDeleteCard->start();
}

//设置样式表
void CSERVERCENTER::_SetStyleSheet()
{
    ui->QAlarm->setStyleSheet("QTableWidget {"
                "background-color:transparent;border:1px solid  #9D9B90;"
                "}");
}

//动作
void CSERVERCENTER::_CreateActions()
{
//    MinimizeAction = new QAction(QIcon(":/tu.png"),tr("最小化"),this);
    MinimizeAction = new QAction(tr("最小化"),this);
    MaximizeAction = new QAction(tr("最大化"),this);
    RestoreAction = new QAction(tr("还原"),this);
    TrayQuitAction = new QAction(tr("退出"),this);

    CardRecordFindAction = new QAction(tr("查找"),this);
    CardRecordDeleteAction = new QAction(tr("删除"),this);
    CallRecordDeleteAction = new QAction(tr("删除"),this);
    DeviceFindAction = new QAction(tr("查找"),this);
    DeviceDeleteAction = new QAction(tr("删除"),this);
    SystemFindAction = new QAction(tr("查找"),this);
    SystemDeleteAction = new QAction(tr("删除"),this);

    AlarmRecordFindAction = new QAction(tr("查找"),this);
    AlarmRecordDeleteAction = new QAction(tr("删除"),this);
    AlarmDealAction = new QAction(tr("处理"),this);

    DealAction = new QAction(tr("处理"),this);
    StopAction = new QAction(tr("停止播放音乐"),this);

    RepairsRecordFindAction = new QAction(tr("查找"),this);
    RepairsRecordDeleteAction = new QAction(tr("删除"),this);
    RepairsRecordDealAction = new QAction(tr("处理"),this);
    DealAllAction = new QAction(tr("全部处理"),this);

    UserUpdateAction = new QAction(tr("修改"),this);
    UserDeleteAction = new QAction(tr("删除"),this);
    UserAddAction = new QAction(tr("添加"),this);

    IcCarUpdateAction = new QAction(tr("修改"),this);
    IcCarDeleteAction = new QAction(tr("删除"),this);
    IcCarAddAction = new QAction(tr("添加"),this);
    IcCarFindAction = new QAction(tr("查找"),this);

    CardMessageUpAction = new QAction(tr("上传"),this);
    CardMessageSaveAction = new QAction(tr("保存"),this);
    CardMessageDownAction = new QAction(tr("注册"),this);
    CardMessageDownAllAction = new QAction(tr("覆盖"),this);
    CardLogOutAction = new QAction(tr("注销"),this);

    RepairsUpdateAction = new QAction(tr("修改"),this);
    RepairsDeleteAction = new QAction(tr("删除"),this);
    RepairsAddAction = new QAction(tr("添加"),this);
    RepairsDownAction = new QAction(tr("下发"),this);
    PhoneUpdateAction = new QAction(tr("修改"),this);
    PhoneDeleteAction = new QAction(tr("删除"),this);
    PhoneAddAction = new QAction(tr("添加"),this);
    PhoneDownAction = new QAction(tr("下发"),this);

    PingIpAction = new QAction(tr("网络连接测试"),this);
    SmartControlAction = new QAction(tr("智能家居"),this);
    UpdateAction = new QAction(tr("修改"),this);
    DeleteAction = new QAction(tr("删除"),this);
    InitSystemPassAction = new QAction(tr("初始化系统密码"),this);
    InitLockPassAction = new QAction(tr("初始化开锁密码"),this);
    FindAction = new QAction(tr("查找"),this);
    ResetAction = new QAction(tr("重启"),this);

    NextPageAction = new QAction(tr("下一页"), this);
    PreviousPageAction = new QAction(tr("上一页"),this);
    ICAction = new QAction(tr("IC卡信息"),this);
    MiddleAction = new QAction(tr("中间设备"),this);
    TenementAction = new QAction(tr("住户设备"),this);
    DBSetAction = new QAction(tr("数据库设置"),this);
    WeatherAction  = new QAction(tr("天气设置"),this);
    QuitAction = new QAction(tr("退出"),this);
    MessageAction = new QAction(tr("普通信息"),this);
    RepairsmessageAction = new QAction(tr("物业信息"),this);
    NewsAction = new QAction(tr("重要新闻"),this);
    RepairsAction = new QAction(tr("报修记录"),this);
    AlarmAction = new QAction(tr("报警记录"),this);
    SystemAction = new QAction(tr("系统日志"),this);
    DeviceAction = new QAction(tr("设备日志"),this);
    CardAction = new QAction(tr("刷卡日志"),this);
    ManageAction = new QAction(tr("操作员管理"),this);
    CallRecordAction = new QAction(tr("呼叫记录"),this);
    AboutAction = new QAction(tr("关于"),this);
    HelpAction = new QAction(tr("帮助"),this);
    LockAction = new QAction(tr("锁定"),this);
    LogoutAction = new QAction(tr("注销"),this);

    DeleteCardAction = new QAction(tr("删除此卡"),this);

    SaveCardAction = new QAction(tr("保存此卡号"),this);
    SaveAllCardAction = new QAction(tr("保存全部卡号"),this);
    ClearAllCardAction = new QAction(tr("清空"),this);
}

//菜单创建
void CSERVERCENTER::_CreateMenus()
{
    ui->QlinePath->setEnabled(false);
    TrayIconMenu = new QMenu(QApplication::desktop());
    TrayIconMenu->addAction(MinimizeAction);
    TrayIconMenu->addAction(MaximizeAction);
    TrayIconMenu->addAction(RestoreAction);
    TrayIconMenu->addSeparator();
    TrayIconMenu->addAction(TrayQuitAction);
    TrayIcon->setContextMenu(TrayIconMenu);

    systemMenu = ui->menubar->addMenu(tr("系统"));
    if(m_sUser.protocol == 0)
    {
        systemMenu->addAction(ICAction);
    }
    systemMenu->addAction(MiddleAction);
    systemMenu->addAction(TenementAction);
    systemMenu->addAction(LockAction);
    systemMenu->addAction(LogoutAction);
    systemMenu->addAction(QuitAction);

    noteMenu = ui->menubar->addMenu(tr("小区短信"));
    noteMenu->addAction(MessageAction);
    noteMenu->addAction(RepairsmessageAction);
    if(m_sUser.protocol != 0)
    {
        noteMenu->addAction(NewsAction);
    }

    recordMenu = ui->menubar->addMenu(tr("日志&记录"));
    recordMenu->addAction(RepairsAction);
    recordMenu->addAction(AlarmAction);
    recordMenu->addAction(SystemAction);
    recordMenu->addAction(DeviceAction);
    if(m_sUser.protocol == 0)
    {
        recordMenu->addAction(CallRecordAction);
        recordMenu->addAction(CardAction);
    }

    daminMenu = ui->menubar->addMenu(tr("操作员管理"));
    daminMenu->addAction(WeatherAction);
    daminMenu->addAction(ManageAction);
//    daminMenu->addAction(DBSetAction);

    helpMenu = ui->menubar->addMenu(tr("帮助"));
    helpMenu->addAction(AboutAction);
    helpMenu->addAction(HelpAction);

    MiddleMenu = new QMenu(ui->QMiddle);
    ui->QMiddle->setContextMenuPolicy(Qt::CustomContextMenu);
    TenementMenu = new QMenu(ui->QTenement);
    ui->QTenement->setContextMenuPolicy(Qt::CustomContextMenu);
    PhoneMenu = new QMenu(ui->QPhone);
    ui->QPhone->setContextMenuPolicy(Qt::CustomContextMenu);
    RepairsMenu = new QMenu(ui->QRepairs);
    ui->QRepairs->setContextMenuPolicy(Qt::CustomContextMenu);
    IcCardMenu = new QMenu(ui->QIcCard);
    ui->QIcCard->setContextMenuPolicy(Qt::CustomContextMenu);
    CardMessageMenu = new QMenu(ui->QCardMessage);
    ui->QCardMessage->setContextMenuPolicy(Qt::CustomContextMenu);

    UserMenu = new QMenu(ui->QUser);
    ui->QUser->setContextMenuPolicy(Qt::CustomContextMenu);

    RepairsRecordMenu = new QMenu(ui->QRepairsRecord);
    ui->QRepairsRecord->setContextMenuPolicy(Qt::CustomContextMenu);
    AlarmRecordMenu = new QMenu(ui->QAlarmRecord);
    ui->QAlarmRecord->setContextMenuPolicy(Qt::CustomContextMenu);
    CardRecordMenu = new QMenu(ui->QCardRecord);
    ui->QCardRecord->setContextMenuPolicy(Qt::CustomContextMenu);
    DeviceMenu = new QMenu(ui->QDevice);
    ui->QDevice->setContextMenuPolicy(Qt::CustomContextMenu);
    SystemMenu = new QMenu(ui->QSystem);
    ui->QSystem->setContextMenuPolicy(Qt::CustomContextMenu);
    AlarmMenu = new QMenu(ui->QAlarm);
    ui->QAlarm->setContextMenuPolicy(Qt::CustomContextMenu);
    CallRecordMenu = new QMenu(ui->QCallRecord);
    ui->QCallRecord->setContextMenuPolicy(Qt::CustomContextMenu);

    DeleteCardMenu = new QMenu(ui->Empower_IC);
    ui->Empower_IC->setContextMenuPolicy(Qt::CustomContextMenu);

    AddCardMenu = new QMenu(ui->Upload_IC);
    ui->Upload_IC->setContextMenuPolicy(Qt::CustomContextMenu);
}

void CSERVERCENTER::resizeEvent( QResizeEvent* event)
{
    QSize oldSize,size;
    oldSize = event->oldSize(); //获取开始的size
    size = event->size(); //获取现在的size
}

// 注销后登陆，更新相关数据
void CSERVERCENTER::_UpdateLoading(SUser sUser)
{
    m_sUser = sUser;
    QString grade;
    grade.clear();
    switch(m_sUser.nGrade)
    {
        case 1:
            grade.clear();
            grade = "超级管理员";
            break;
        case 2:
            grade.clear();
            grade = "管理员";
            break;
        case 3:
            grade.clear();
            grade = "操作员";
            break;
        default :
            grade.clear();
            grade = "错误";
            break;
    }

    statulabe2->setText(m_sUser.strName);
    statulabe4->setText(grade);
    CUdpRecv->_UpdateLoading(m_sUser.strCityNum);
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("设备信息",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 0;      //page_1
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
    ui->QlblAllName->hide();
    ui->QlblAll->clear();
    ui->QlblClashName->hide();
    ui->QlblClash->clear();

    QDateTime dateTime=QDateTime::currentDateTime();
    QString time = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    statulabe8->setText(time);
    statulabe8->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    TrayIcon->show();
}


//设备地址 IP地址冲突检测
void CSERVERCENTER::_CheckClash()
{
    QString str;
    QFont ft;
    ft.setPointSize(12);
    QPalette pa;
    pa.setColor(QPalette::WindowText,Qt::red);
    str.clear();
    QDateTime dateTime = QDateTime::currentDateTime();

    QString SQL1,SQL2,SQL3,SQL4,SQL5,SQL6,SQL7,SQL8,SQL9,SQL10;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    SQL1 = "select Tenement_intra_addr from tenement group by Tenement_intra_addr having count(*)> 1 ";
    if(query.exec(SQL1))
    {
        while(query.next())
        {
            str = str + QString(query.value(0).toByteArray().data()) + ",";
        }
    }
    SQL2 = "select Middle_intra_addr from middle group by Middle_intra_addr having count(*)> 1 ";
    query.clear();
    if(query.exec(SQL2))
    {
        while(query.next())
        {
            str = str + QString(query.value(0).toByteArray().data())+ ",";
        }
    }
    SQL3 = "select Middle_ip_addr from middle group by Middle_ip_addr having count(*)> 1 ";
    query.clear();
    if(query.exec(SQL3))
    {
        while(query.next())
        {
            str = str + QString(query.value(0).toByteArray().data())+ ",";
        }
    }
    SQL4 = "select Tenement_ip_addr from tenement group by Tenement_ip_addr having count(*)> 1 ";
    query.clear();
    if(query.exec(SQL4))
    {
        while(query.next())
        {
            str = str + QString(query.value(0).toByteArray().data())+ ",";
        }
    }
    SQL5 = "SELECT  a.Tenement_ip_addr from tenement a,middle b WHERE a.Tenement_ip_addr = b.Middle_ip_addr";
    query.clear();
    if(query.exec(SQL5))
    {
        while(query.next())
        {
            str = str + QString(query.value(0).toByteArray().data())+ ",";
        }
    }
    SQL6 = "select Middle_mac_addr from middle group by Middle_mac_addr having count(*)> 1 ";
    query.clear();
    if(query.exec(SQL6))
    {
        while(query.next())
        {
            str = str + QString(query.value(0).toByteArray().data())+ ",";
        }
    }
    SQL7 = "select Tenement_mac_addr from tenement group by Tenement_mac_addr having count(*)> 1 ";
    query.clear();
    if(query.exec(SQL7))
    {
        while(query.next())
        {
            str = str + QString(query.value(0).toByteArray().data())+ ",";
        }
    }
    SQL8 = "SELECT  a.Tenement_mac_addr from tenement a,middle b WHERE a.Tenement_mac_addr = b.Middle_mac_addr";
    query.clear();
    if(query.exec(SQL8))
    {
        while(query.next())
        {
            str = str + QString(query.value(0).toByteArray().data())+ ",";
        }
    }
    if(str!=NULL)
    {
        ui->QlblClashName->show();
        ui->QlblClash->setFont(ft);
        ui->QlblClash->setPalette(pa);
        ui->QlblClash->setText(str.left(str.length() - 1));
    }else{
        ui->QlblClash->clear();
        ui->QlblClashName->hide();
    }

    int nAlarm = 0;
    int nRepairs = 0;
    query.clear();
    SQL9 = "SELECT count(*) FROM alarm WHERE Alarm_dtime is NULL";
    if(query.exec(SQL9))
    {
        while(query.next())
        {
            nAlarm  = query.value(0).toInt();
        }
    }

    query.clear();
    SQL9 = "SELECT count(*) FROM  repairs_record WHERE Repairs_Record_dtime is NULL";
    if(query.exec(SQL9))
    {
        while(query.next())
        {
            nRepairs  = query.value(0).toInt();
        }
    }

    if(nAlarm != 0)
    {
        ui->QlblNotDealAlarm->setFont(ft);
        ui->QlblNotDealAlarm->setPalette(pa);
        ui->QlblNotDealAlarm->setText(QString::number(nAlarm));
    }
    else
    {
//        ui->QlblNotDealAlarm->clear();
        ui->QlblNotDealAlarm->setText(QString::number(0));
    }

    if(nRepairs != 0)
    {
        ui->QlblNotDealRepairs->setFont(ft);
        ui->QlblNotDealRepairs->setPalette(pa);
        ui->QlblNotDealRepairs->setText(QString::number(nRepairs));
    }
    else
    {
//        ui->QlblNotDealRepairs->clear();
        ui->QlblNotDealRepairs->setText(QString::number(0));
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

//列表初始化  begin
void CSERVERCENTER::_InitTenement()
{
    ui->QTenement->setColumnCount(12);
    ui->QTenement->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"序号"<<"地址"<<"地址说明"<<"姓名"<<"电话1"<<"电话2"<<"网卡地址"<<"IP地址"<<"连接状态"<<"布防状态"<<"报警状态"<<"设备类型";
    ui->QTenement->verticalHeader()->hide();//行头不显示
    ui->QTenement->horizontalHeader()->setClickable(false);//行头不可选
    ui->QTenement->setHorizontalHeaderLabels(headers);
    ui->QTenement->horizontalHeader()->setHighlightSections(false);//表头坍塌
    ui->QTenement->resizeColumnsToContents();
    ui->QTenement->setFont(QFont("Helvetica"));//字体
    ui->QTenement->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QTenement->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QTenement->setColumnWidth(0,30);
    ui->QTenement->setColumnWidth(1,140);
    ui->QTenement->setColumnWidth(2,190);
    ui->QTenement->setColumnWidth(3,30);
    ui->QTenement->setColumnWidth(4,10);
    ui->QTenement->setColumnWidth(5,10);
    ui->QTenement->setColumnWidth(6,120);
    ui->QTenement->setColumnWidth(7,100);
    ui->QTenement->setColumnWidth(8,80);
    ui->QTenement->setColumnWidth(9,80);
    ui->QTenement->setColumnWidth(10,80);
    ui->QTenement->setColumnWidth(11,80);
}

void CSERVERCENTER::_InitMiddle()
{
    ui->QMiddle->setColumnCount(7);
    ui->QMiddle->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"序号"<<"地址"<<"地址说明"<<"网卡地址"<<"IP地址"<<"连接状态"<<"设备类型";
    ui->QMiddle->verticalHeader()->hide();//行头不显示
    ui->QMiddle->horizontalHeader()->setClickable(false);//行头不可选
    ui->QMiddle->setHorizontalHeaderLabels(headers);
    ui->QMiddle->horizontalHeader()->setHighlightSections(false);
    ui->QMiddle->resizeColumnsToContents();
    ui->QMiddle->setFont(QFont("Helvetica"));

    ui->QMiddle->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QMiddle->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QMiddle->setColumnWidth(0,40);
    ui->QMiddle->setColumnWidth(1,150);
    ui->QMiddle->setColumnWidth(2,200);
    ui->QMiddle->setColumnWidth(3,120);
    ui->QMiddle->setColumnWidth(4,100);
    ui->QMiddle->setColumnWidth(5,80);
    ui->QMiddle->setColumnWidth(6,80);
}

void CSERVERCENTER::_InitAlarm()
{
    ui->QAlarm->setColumnCount(10);
    ui->QAlarm->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"序号"<<"IP地址"<<"地址"<<"姓名"<<"电话1"<<"报警防区"<<"报警类型"<<"报警时间"<<"取消时间"<<"设备类型";
    ui->QAlarm->verticalHeader()->hide();//行头不显示
    ui->QAlarm->horizontalHeader()->setClickable(false);//行头不可选
    ui->QAlarm->setHorizontalHeaderLabels(headers);
    ui->QAlarm->horizontalHeader()->setHighlightSections(false);
    ui->QAlarm->resizeColumnsToContents();
    ui->QAlarm->setFont(QFont("Helvetica"));
    ui->QAlarm->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QAlarm->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QAlarm->setColumnWidth(0,40);
    ui->QAlarm->setColumnWidth(1,100);
    ui->QAlarm->setColumnWidth(2,200);
    ui->QAlarm->setColumnWidth(3,100);
    ui->QAlarm->setColumnWidth(4,100);
    ui->QAlarm->setColumnWidth(5,80);
    ui->QAlarm->setColumnWidth(6,80);
    ui->QAlarm->setColumnWidth(7,130);
    ui->QAlarm->setColumnWidth(8,130);
    ui->QAlarm->setColumnWidth(9,80);
}

void CSERVERCENTER::_InitAlarmRecord()
{
    ui->QAlarmRecord->setColumnCount(14);
    ui->QAlarmRecord->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"序号"<<"IP地址"<<"地址"<<"设备类型"<<"姓名"<<"电话1"<<"报警防区"<<"报警类型"<<"报警时间"<<"确认时间"<<"取消时间"<<"是否误报"<<"处理人"<<"详情";
    ui->QAlarmRecord->verticalHeader()->hide();//行头不显示
    ui->QAlarmRecord->horizontalHeader()->setClickable(false);//行头不可选
    ui->QAlarmRecord->setHorizontalHeaderLabels(headers);
    ui->QAlarmRecord->horizontalHeader()->setHighlightSections(false);
    ui->QAlarmRecord->resizeColumnsToContents();
    ui->QAlarmRecord->setFont(QFont("Helvetica"));
    ui->QAlarmRecord->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QAlarmRecord->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QAlarmRecord->setColumnWidth(0,50);
    ui->QAlarmRecord->setColumnWidth(1,100);
    ui->QAlarmRecord->setColumnWidth(2,200);
    ui->QAlarmRecord->setColumnWidth(3,80);
    ui->QAlarmRecord->setColumnWidth(4,100);
    ui->QAlarmRecord->setColumnWidth(5,100);
    ui->QAlarmRecord->setColumnWidth(6,80);
    ui->QAlarmRecord->setColumnWidth(7,80);
    ui->QAlarmRecord->setColumnWidth(8,130);
    ui->QAlarmRecord->setColumnWidth(9,130);
    ui->QAlarmRecord->setColumnWidth(10,130);
    ui->QAlarmRecord->setColumnWidth(11,80);
    ui->QAlarmRecord->setColumnWidth(12,80);
    ui->QAlarmRecord->setColumnWidth(13,130);
}

void CSERVERCENTER::_InitRepairsRecord()
{
    ui->QRepairsRecord->setColumnCount(9);
    ui->QRepairsRecord->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"序号"<<"地址"<<"设备类型"<<"报修类型"<<"报修时间"<<"取消时间"<<"处理时间"<<"处理人"<<"详情";
    ui->QRepairsRecord->verticalHeader()->hide();//行头不显示
    ui->QRepairsRecord->horizontalHeader()->setClickable(false);//行头不可选
    ui->QRepairsRecord->setHorizontalHeaderLabels(headers);
    ui->QRepairsRecord->horizontalHeader()->setHighlightSections(false);
    ui->QRepairsRecord->resizeColumnsToContents();
    ui->QRepairsRecord->setFont(QFont("Helvetica"));
    ui->QRepairsRecord->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QRepairsRecord->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QRepairsRecord->setColumnWidth(0,50);
    ui->QRepairsRecord->setColumnWidth(1,200);
    ui->QRepairsRecord->setColumnWidth(2,80);
    ui->QRepairsRecord->setColumnWidth(3,120);
    ui->QRepairsRecord->setColumnWidth(4,130);
    ui->QRepairsRecord->setColumnWidth(5,130);
    ui->QRepairsRecord->setColumnWidth(6,130);
    ui->QRepairsRecord->setColumnWidth(7,100);
    ui->QRepairsRecord->setColumnWidth(8,120);
}

void CSERVERCENTER::_InitIcCard()
{
    ui->QIcCard->setColumnCount(4);
    ui->QIcCard->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"序号"<<"卡号"<<"所属地址"<<"有效时间";
    ui->QIcCard->verticalHeader()->hide();//行头不显示
    ui->QIcCard->horizontalHeader()->setClickable(false);//行头不可选
    ui->QIcCard->setHorizontalHeaderLabels(headers);
    ui->QIcCard->horizontalHeader()->setHighlightSections(false);
    ui->QIcCard->resizeColumnsToContents();
    ui->QIcCard->setFont(QFont("Helvetica"));
    ui->QIcCard->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QIcCard->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QIcCard->setColumnWidth(0,50);
    ui->QIcCard->setColumnWidth(1,110);
    ui->QIcCard->setColumnWidth(2,220);
    ui->QIcCard->setColumnWidth(3,90);
}

void CSERVERCENTER::_InitCardMessage()
{
    ui->QCardMessage->setColumnCount(4);
    ui->QCardMessage->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"权限"<<"设备地址"<<"设备类型"<<"发送结果";
    ui->QCardMessage->verticalHeader()->hide();//行头不显示
    ui->QCardMessage->horizontalHeader()->setClickable(false);//行头不可选
    ui->QCardMessage->setHorizontalHeaderLabels(headers);
    ui->QCardMessage->horizontalHeader()->setHighlightSections(false);
    ui->QCardMessage->resizeColumnsToContents();
    ui->QCardMessage->setFont(QFont("Helvetica"));
    ui->QCardMessage->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QCardMessage->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QCardMessage->setColumnWidth(0,50);
    ui->QCardMessage->setColumnWidth(1,200);
    ui->QCardMessage->setColumnWidth(2,120);
    ui->QCardMessage->setColumnWidth(3,120);
}

void CSERVERCENTER::_InitCardRecord()
{
    ui->QCardRecord->setColumnCount(6);
    ui->QCardRecord->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"序号"<<"设备地址"<<"设备类型"<<"卡号"<<"所属地址"<<"刷卡时间";
    ui->QCardRecord->verticalHeader()->hide();//行头不显示
    ui->QCardRecord->horizontalHeader()->setClickable(false);//行头不可选
    ui->QCardRecord->setHorizontalHeaderLabels(headers);
    ui->QCardRecord->horizontalHeader()->setHighlightSections(false);
    ui->QCardRecord->resizeColumnsToContents();
    ui->QCardRecord->setFont(QFont("Helvetica"));
    ui->QCardRecord->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QCardRecord->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QCardRecord->setColumnWidth(0,50);
    ui->QCardRecord->setColumnWidth(1,200);
    ui->QCardRecord->setColumnWidth(2,120);
    ui->QCardRecord->setColumnWidth(3,120);
    ui->QCardRecord->setColumnWidth(4,105);
    ui->QCardRecord->setColumnWidth(5,120);
}

void CSERVERCENTER::_InitUser()
{
    ui->QUser->setColumnCount(4);
    ui->QUser->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"序号"<<"用户名"<<"姓名"<<"级别";
    ui->QUser->verticalHeader()->hide();//行头不显示
    ui->QUser->setHorizontalHeaderLabels(headers);
    ui->QUser->horizontalHeader()->setClickable(false);//行头不可选
    ui->QUser->horizontalHeader()->setHighlightSections(false);
    ui->QUser->resizeColumnsToContents();
    ui->QUser->setFont(QFont("Helvetica"));
    ui->QUser->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QUser->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QUser->setColumnWidth(0,50);
    ui->QUser->setColumnWidth(1,150);
    ui->QUser->setColumnWidth(2,150);
    ui->QUser->setColumnWidth(3,150);
}

void CSERVERCENTER::_InitSystem()
{
    ui->QSystem->setColumnCount(5);
    ui->QSystem->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"序号"<<"用户名"<<"姓名"<<"操作类型"<<"时间";
    ui->QSystem->verticalHeader()->hide();//行头不显示
    ui->QSystem->horizontalHeader()->setClickable(false);//行头不可选
    ui->QSystem->setHorizontalHeaderLabels(headers);
    ui->QSystem->horizontalHeader()->setHighlightSections(false);
    ui->QSystem->resizeColumnsToContents();
    ui->QSystem->setFont(QFont("Helvetica"));
    ui->QSystem->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QSystem->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QSystem->setColumnWidth(0,50);
    ui->QSystem->setColumnWidth(1,120);
    ui->QSystem->setColumnWidth(2,120);
    ui->QSystem->setColumnWidth(3,120);
    ui->QSystem->setColumnWidth(4,130);
}

void CSERVERCENTER::_InitDevice()
{
    ui->QDevice->setColumnCount(6);
    ui->QDevice->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"序号"<<"地址"<<"IP地址"<<"状态"<<"时间"<<"设备类型";
    ui->QDevice->verticalHeader()->hide();//行头不显示
    ui->QDevice->horizontalHeader()->setClickable(false);//行头不可选
    ui->QDevice->setHorizontalHeaderLabels(headers);
    ui->QDevice->horizontalHeader()->setHighlightSections(false);
    ui->QDevice->resizeColumnsToContents();
    ui->QDevice->setFont(QFont("Helvetica"));
    ui->QDevice->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QDevice->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QDevice->setColumnWidth(0,50);
    ui->QDevice->setColumnWidth(1,200);
    ui->QDevice->setColumnWidth(2,120);
    ui->QDevice->setColumnWidth(3,120);
    ui->QDevice->setColumnWidth(4,130);
    ui->QDevice->setColumnWidth(5,130);
}

void CSERVERCENTER::_InitMessage()
{
    ui->QMessage->setColumnCount(8);
    ui->QMessage->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"选择"<<"地址"<<"地址说明"<<"姓名"<<"IP地址"<<"连接状态"<<"设备类型"<<"发送结果";
    ui->QMessage->verticalHeader()->hide();//行头不显示
    ui->QMessage->horizontalHeader()->setClickable(false);//行头不可选
    ui->QMessage->setHorizontalHeaderLabels(headers);
    ui->QMessage->horizontalHeader()->setHighlightSections(false);
    ui->QMessage->resizeColumnsToContents();
    ui->QMessage->setFont(QFont("Helvetica"));
    ui->QMessage->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QMessage->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QMessage->setColumnWidth(0,50);
    ui->QMessage->setColumnWidth(1,25);
    ui->QMessage->setColumnWidth(2,200);
    ui->QMessage->setColumnWidth(3,100);
    ui->QMessage->setColumnWidth(4,100);
    ui->QMessage->setColumnWidth(5,80);
    ui->QMessage->setColumnWidth(6,80);
    ui->QMessage->setColumnWidth(7,80);
}

void CSERVERCENTER::_InitNews()
{
    ui->QNews->setColumnCount(7);
    ui->QNews->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"选择"<<"地址"<<"地址说明"<<"IP地址"<<"连接状态"<<"设备类型"<<"发送结果";
    ui->QNews->verticalHeader()->hide();//行头不显示
    ui->QNews->horizontalHeader()->setClickable(false);//行头不可选
    ui->QNews->setHorizontalHeaderLabels(headers);
    ui->QNews->horizontalHeader()->setHighlightSections(false);
    ui->QNews->resizeColumnsToContents();
    ui->QNews->setFont(QFont("Helvetica"));
    ui->QNews->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QNews->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QNews->setColumnWidth(0,50);
    ui->QNews->setColumnWidth(1,25);
    ui->QNews->setColumnWidth(2,200);
    ui->QNews->setColumnWidth(3,100);
    ui->QNews->setColumnWidth(4,80);
    ui->QNews->setColumnWidth(5,80);
    ui->QNews->setColumnWidth(6,140);
}

void CSERVERCENTER::_InitPhone()
{
    ui->QPhone->setColumnCount(3);
    ui->QPhone->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"部门"<<"联系人"<<"电话";
    ui->QPhone->verticalHeader()->hide();//行头不显示
    ui->QPhone->horizontalHeader()->setClickable(false);//行头不可选
    ui->QPhone->setHorizontalHeaderLabels(headers);
    ui->QPhone->horizontalHeader()->setHighlightSections(false);
    ui->QPhone->resizeColumnsToContents();
    ui->QPhone->setFont(QFont("Helvetica"));
    ui->QPhone->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QPhone->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QPhone->setColumnWidth(1,100);
    ui->QPhone->setColumnWidth(2,120);
    ui->QPhone->setColumnWidth(0,100);
}

void CSERVERCENTER::_InitRepairs()
{
    ui->QRepairs->setColumnCount(1);
    ui->QRepairs->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"报修类型";
    ui->QRepairs->verticalHeader()->hide();//行头不显示
    ui->QRepairs->horizontalHeader()->setClickable(false);//行头不可选
    ui->QRepairs->setHorizontalHeaderLabels(headers);
    ui->QRepairs->horizontalHeader()->setHighlightSections(false);
    ui->QRepairs->resizeColumnsToContents();
    ui->QRepairs->setFont(QFont("Helvetica"));
    ui->QRepairs->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->QRepairs->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->QRepairs->setColumnWidth(0,150);
}

void CSERVERCENTER::_InitCallRecord()
{
    ui->QCallRecord->setColumnCount(5);
    ui->QCallRecord->verticalHeader()->hide();
    ui->QCallRecord->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->QCallRecord->setSelectionBehavior(QAbstractItemView::SelectRows);
    QStringList header;
    header<<"序号"<<"呼叫时间"<<"主叫设备地址"<<"被叫设备地址"<<"图片路径";
    ui->QCallRecord->horizontalHeader()->setHighlightSections(false);//表头坍塌
    ui->QCallRecord->setHorizontalHeaderLabels(header);
    ui->QCallRecord->setColumnWidth(0,60);
    ui->QCallRecord->setColumnWidth(1,150);
    ui->QCallRecord->setColumnWidth(2,150);
    ui->QCallRecord->setColumnWidth(3,200);
    ui->QCallRecord->setColumnWidth(4,30);
}

void CSERVERCENTER::_InitCallNow()
{
    ui->QCallNow->setColumnCount(5);
    ui->QCallNow->verticalHeader()->hide();
    ui->QCallNow->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->QCallNow->setSelectionBehavior(QAbstractItemView::SelectRows);
    QStringList header;
    header<<"序号"<<"呼叫时间"<<"主叫设备地址"<<"被叫设备地址"<<"图片路径";
    ui->QCallNow->horizontalHeader()->setHighlightSections(false);//表头坍塌
    ui->QCallNow->setHorizontalHeaderLabels(header);
    ui->QCallNow->setColumnWidth(0,60);
    ui->QCallNow->setColumnWidth(1,150);
    ui->QCallNow->setColumnWidth(2,150);
    ui->QCallNow->setColumnWidth(3,200);
    ui->QCallNow->setColumnWidth(4,30);
}

void CSERVERCENTER::_InitEmpowerIC()
{
    ui->Empower_IC->setColumnCount(1);
    ui->Empower_IC->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"IC卡";
    ui->Empower_IC->verticalHeader()->hide();//行头不显示
    ui->Empower_IC->horizontalHeader()->setClickable(false);//行头不可选
    ui->Empower_IC->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->Empower_IC->setColumnWidth(0,150);
}

void CSERVERCENTER::_UpdateEmpowerIC(QString strAddr)
{
    QString SQL = "SELECT * FROM ic_card_A8 WHERE IC_Card_addr = '"+_AddrExplainToAddrA8(strAddr,"室内机",1)+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    int ti = 0;
    if(query.exec(SQL))
    {
        ui->Empower_IC->setRowCount(ti);
        while(query.next())
        {
            ui->Empower_IC->setRowCount(ti+1);
            QTableWidgetItem *tableItem = new QTableWidgetItem(QString(query.value(2).toString()));
            tableItem->setTextAlignment(Qt::AlignCenter);
            ui->Empower_IC->setItem(ti,0,tableItem);
            ti++;
        }
    }
    ui->Empower_IC->setHorizontalHeaderLabels(QStringList()<<"IC卡张数：" + QString::number(ti));
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

void CSERVERCENTER::_UpdateRegisterIndoor(NodeTenement *ItemTenement)
{
    m_List.clear();
    ui->Register_Indoor->clear();
    int ti = 0;
    QTreeWidgetItem *head[100];
    QString HeaderstrAddr;
    while(ItemTenement->next != NULL)
    {
        HeaderstrAddr = QString(ItemTenement->data.gcAddrExplain).left(10);

        for(int i = 0; i<m_List.length(); i++)
        {
            if(HeaderstrAddr == QString(m_List.at(i).data()))
            {
                QTreeWidgetItem *treeItem = new QTreeWidgetItem(QStringList()<<QString(ItemTenement->data.gcAddrExplain).right(10));
                head[i]->addChild(treeItem);
                isHeadItem = false;
            }
        }
        if(isHeadItem)
        {
            head[ti] = new QTreeWidgetItem(QStringList()<<QString(HeaderstrAddr));
            ui->Register_Indoor->addTopLevelItem(head[ti]);
            QTreeWidgetItem *treeItem = new QTreeWidgetItem(QStringList()<<QString(ItemTenement->data.gcAddrExplain).right(10));
            head[ti]->addChild(treeItem);
            m_List.append(HeaderstrAddr);
            ti++;
        }
        isHeadItem = true;
        ItemTenement = ItemTenement->next;
    }
}

void CSERVERCENTER::_InitRegisterMachine()
{
    ui->Register_Machine->setColumnCount(2);
    ui->Register_Machine->setShowGrid(true);//显示表格线
    QStringList headers;
    headers<<"权限"<<"中间设备";
    ui->Register_Machine->verticalHeader()->hide();//行头不显示
    ui->Register_Machine->horizontalHeader()->setClickable(false);//行头不可选
    ui->Register_Machine->setHorizontalHeaderLabels(headers);
    ui->Register_Machine->horizontalHeader()->setHighlightSections(false);
    ui->Register_Machine->resizeColumnsToContents();
    ui->Register_Machine->setFont(QFont("Helvetica"));
    ui->Register_Machine->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->Register_Machine->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->Register_Machine->setColumnWidth(0,50);
    ui->Register_Machine->setColumnWidth(1,200);
}

void CSERVERCENTER::_UpdateRegisterMachine(NodeMiddle *ItemMiddle)
{
    int ti = 0;
    while(ItemMiddle->next!=NULL)
    {
        if(ItemMiddle->data.gcIntraAddr[0] == 'M' || ItemMiddle->data.gcIntraAddr[0] == 'H')
        {
            ui->Register_Machine->setRowCount(ti+1);
            QTableWidgetItem *checkBox1 = new QTableWidgetItem();
            checkBox1->setCheckState(Qt::Unchecked);
            checkBox1->setTextAlignment(Qt::AlignCenter);
            ui->Register_Machine->setItem(ti,0,checkBox1);

            QTableWidgetItem *tableItem1 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddrExplain));
            tableItem1->setTextAlignment(Qt::AlignCenter);
            ui->Register_Machine->setItem(ti,1,tableItem1);
            ti++;
        }
        ItemMiddle = ItemMiddle->next;
    }
}

void CSERVERCENTER::_InitRegisterIC()
{
    ui->Register_IC->setColumnCount(1);
    ui->Register_IC->setShowGrid(true);
    ui->Register_IC->verticalHeader()->hide();//行头不显示
    ui->Register_IC->horizontalHeader()->setClickable(false);//行头不可选
    //    ui->Register_IC->setHorizontalHeaderLabels(headers);
    ui->Register_IC->horizontalHeader()->setHighlightSections(false);
    ui->Register_IC->resizeColumnsToContents();
    ui->Register_IC->setFont(QFont("Helvetica"));
    ui->Register_IC->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->Register_IC->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->Register_IC->setColumnWidth(0,100);
}

void CSERVERCENTER::_UpdateIssuedMachine(NodeMiddle *ItemMiddle)
{
    ui->Issued_Machine->clear();
    ui->Issued_Machine->setHeaderHidden(true);
    QTreeWidgetItem *head = new QTreeWidgetItem(QStringList()<<QString("中间设备"));
    ui->Issued_Machine->addTopLevelItem(head);

    while(ItemMiddle->next != NULL)
    {
        if(ItemMiddle->data.gcIntraAddr[0] == 'M' || ItemMiddle->data.gcIntraAddr[0] == 'H')
        {
            QTreeWidgetItem *treeItem = new QTreeWidgetItem(QStringList()<<QString(ItemMiddle->data.gcAddrExplain));
            head->addChild(treeItem);
        }
        ItemMiddle = ItemMiddle->next;
    }
}

void CSERVERCENTER::_InitIssuedMachine()
{
    ui->Download_IC->setColumnCount(2);
    ui->Download_IC->setShowGrid(true);
    QStringList headers;
    headers<<"序号"<<"IC卡号";
    ui->Download_IC->setHorizontalHeaderLabels(headers);
    ui->Download_IC->verticalHeader()->hide();
    ui->Download_IC->horizontalHeader()->setClickable(false);//行头不可选
    ui->Download_IC->horizontalHeader()->setHighlightSections(false);
    ui->Download_IC->resizeColumnsToContents();
    ui->Download_IC->setFont(QFont("Helvetica"));
    ui->Download_IC->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    ui->Download_IC->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式：一行
    ui->Download_IC->setColumnWidth(0,40);
    ui->Download_IC->setColumnWidth(1,150);

    ui->Upload_IC->setColumnCount(3);
    ui->Upload_IC->setShowGrid(true);
    QStringList headers1;
    headers1<<"序号"<<"IC卡号"<<"所属房间";
    ui->Upload_IC->verticalHeader()->hide();
    //    ui->Upload_IC->horizontalHeader()->hide();
    ui->Upload_IC->horizontalHeader()->setClickable(false);//行头不可选
    ui->Upload_IC->setHorizontalHeaderLabels(headers1);
    ui->Upload_IC->horizontalHeader()->setHighlightSections(false);
    ui->Upload_IC->resizeColumnsToContents();
    ui->Upload_IC->setFont(QFont("Helvetica"));
    ui->Upload_IC->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->Upload_IC->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->Upload_IC->setColumnWidth(0,40);
    ui->Upload_IC->setColumnWidth(1,100);
    ui->Upload_IC->setColumnWidth(2,200);
    //    ui->Upload_IC->setColumnWidth(3,10);
}
//列表初始化  end

void CSERVERCENTER::on_Issued_Machine_clicked(const QModelIndex &index)
{
    ui->QpbtnUp->setEnabled(true);
    ui->QpbtnDown->setEnabled(true);
    m_AddrofCardMap.clear();
    QString strAddExplain;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    if(ui->Issued_Machine->currentItem()->text(0)!="中间设备")
    {
        strAddExplain = ui->Issued_Machine->currentItem()->text(0);
        QString SQL = "select  IC_Card_num, IC_Card_addr,IC_Card_id from ic_card_a8 b where  EXISTS (select 1 from "
                               "(select Card_Jurisdiction_SAddr from card_jurisdiction_a8 where  Card_Jurisdiction_MAddr = '"+_AddrExplainToAddrA8(strAddExplain,"门口机",1)+"') a where a.Card_Jurisdiction_SAddr=b.IC_Card_addr)";
        QSqlQuery query(db);
        if(query.exec(SQL))
        {
            int ti = 0;
            while(query.next())
            {
                ui->Download_IC->setRowCount(ti+1);

                QTableWidgetItem *tableItem = new QTableWidgetItem(QString::number(ti+1));
                tableItem->setTextAlignment(Qt::AlignCenter);
                ui->Download_IC->setItem(ti,0,tableItem);

                QTableWidgetItem *tableItem1 = new QTableWidgetItem(QString(query.value(0).toString()));
                tableItem1->setTextAlignment(Qt::AlignCenter);
                ui->Download_IC->setItem(ti,1,tableItem1);
                m_AddrofCardMap.insert(query.value(0).toString(),query.value(1).toString());
                ti++;
            }
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

//按设备地址模糊查找中间设备，并更新列表
void CSERVERCENTER::_UpdateFindMiddle(NodeMiddle *ItemMiddle,int nIndex,QString strFind)
{
    int ti = 0;
    ui->QMiddle->setRowCount(ti);
    m_nFindDevice = 1;
    while(ItemMiddle->next != NULL)
    {
        int nFind = -1;
        switch(nIndex)
        {
            case 0:
                nFind = QString(ItemMiddle->data.gcIntraAddr).indexOf(strFind);
                break;
            case 1:
                nFind = QString(ItemMiddle->data.gcIpAddr).indexOf(strFind);
                break;
            case 2:
                nFind = QString(ItemMiddle->data.gcMacAddr).indexOf(strFind);
                break;
            default :
                break;
        }
        if(nFind >= 0)
        {
            ui->QMiddle->setRowCount(ti + 1);
            ItemMiddle->data.nId = ti;
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 0, tabledmItem0);
            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddr));
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 1, tabledmItem1);
            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddrExplain));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 2, tabledmItem2);
            QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemMiddle->data.gcMacAddr));
            tabledmItem3->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 3, tabledmItem3);
            QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemMiddle->data.gcIpAddr));
            tabledmItem4->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 4, tabledmItem4);

            if(ItemMiddle->data.nState > 0)
            {
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QMiddle->setItem(ti, 5, tabledmItem5);
            }
            else
            {
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QMiddle->setItem(ti, 5, tabledmItem5);
            }

            QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemMiddle->data.gcType));
            tabledmItem6->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 6, tabledmItem6);

            if(ItemMiddle->data.nState <= 0)
            {
                for(int j = 0;j < 7;j++)
                {
                    QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                    item->setTextColor(QColor(0,0,0));
                }
            }
            else
            {
                for(int j = 0;j < 7;j++)
                {
                    QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                    item->setTextColor(Qt::blue);
                }
            }
            ti++;
        }
        ItemMiddle = ItemMiddle->next;
    }
}

//根据链表 更新中间设备列表
void CSERVERCENTER::_UpdateMiddle(NodeMiddle *ItemMiddle)
{
    int ti = 0;
    int tOnline = 0;
    ui->QMiddle->setRowCount(ti);

    while(ItemMiddle->next != NULL)
    {
        ui->QMiddle->setRowCount(ti + 1);
        ItemMiddle->data.nId = ti;
        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 0, tabledmItem0);
        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 1, tabledmItem1);
        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 2, tabledmItem2);
        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemMiddle->data.gcMacAddr));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 3, tabledmItem3);
        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemMiddle->data.gcIpAddr));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 4, tabledmItem4);
        if(ItemMiddle->data.nState > 0)
        {
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }else{
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }
        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemMiddle->data.gcType));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 6, tabledmItem6);

        if(ItemMiddle->data.nState <= 0)
        {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(QColor(0,0,0));
            }
        }else {
            tOnline ++;
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(Qt::blue);
            }
        }
        ti++;
        ItemMiddle = ItemMiddle->next;
    }
    CUdpRecv->m_nAllMiddleOnline = tOnline;
    ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));
}

//显示在线的中间设备在列表
void CSERVERCENTER::_MiddleOnline(NodeMiddle *ItemMiddle)
{
    int ti = 0;
    ui->QMiddle->setRowCount(ti);
    while(ItemMiddle->next != NULL)
    {
        if(ItemMiddle->data.nState > 0 )
        {
        ui->QMiddle->setRowCount(ti + 1);
        ItemMiddle->data.nId = ti;
        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 0, tabledmItem0);
        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 1, tabledmItem1);
        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 2, tabledmItem2);
        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemMiddle->data.gcMacAddr));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 3, tabledmItem3);
        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemMiddle->data.gcIpAddr));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 4, tabledmItem4);
        if(ItemMiddle->data.nState > 0)
        {
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }else{
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }
        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemMiddle->data.gcType));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 6, tabledmItem6);

        if(ItemMiddle->data.nState <= 0)
        {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(QColor(0,0,0));
            }
        }else {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(Qt::blue);
            }
        }
        ti++;
        }
        ItemMiddle = ItemMiddle->next;
    }
    CUdpRecv->m_nAllMiddleOnline = ti;
    ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));
}
//显示不在线的中间设备在列表
void CSERVERCENTER::_MiddleNotOnline(NodeMiddle *ItemMiddle)
{
    int ti = 0;
    int tOnline = 0;
    ui->QMiddle->setRowCount(ti);
    while(ItemMiddle->next != NULL)
    {
        if(ItemMiddle->data.nState <= 0 )
        {
            ui->QMiddle->setRowCount(ti + 1);
            ItemMiddle->data.nId = ti;
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 0, tabledmItem0);
            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddr));
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 1, tabledmItem1);
            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddrExplain));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 2, tabledmItem2);
            QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemMiddle->data.gcMacAddr));
            tabledmItem3->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 3, tabledmItem3);
            QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemMiddle->data.gcIpAddr));
            tabledmItem4->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 4, tabledmItem4);
            if(ItemMiddle->data.nState > 0)
            {
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QMiddle->setItem(ti, 5, tabledmItem5);
            }
            else
            {
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QMiddle->setItem(ti, 5, tabledmItem5);
            }
            QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemMiddle->data.gcType));
            tabledmItem6->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 6, tabledmItem6);

            if(ItemMiddle->data.nState <= 0)
            {
                for(int j = 0;j < 7;j++)
                {
                    QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                    item->setTextColor(QColor(0,0,0));
                }
            }
            else
            {
                for(int j = 0;j < 7;j++)
                {
                    QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                    item->setTextColor(Qt::blue);
                }
            }
            ti++;
        }
        else
        {
            tOnline ++;
        }
        ItemMiddle = ItemMiddle->next;
    }
    CUdpRecv->m_nAllMiddleOnline = tOnline;
    ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));
}

//显示门口机在列表
void CSERVERCENTER::_MiddleM(NodeMiddle *ItemMiddle)
{
    int ti = 0;
    int tOnline = 0;
    ui->QMiddle->setRowCount(ti);
    while(ItemMiddle->next != NULL)
    {
        if(ItemMiddle->data.gcIntraAddr[0] == 'M')
        {
        ui->QMiddle->setRowCount(ti + 1);
        ItemMiddle->data.nId = ti;
        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 0, tabledmItem0);
        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 1, tabledmItem1);
        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 2, tabledmItem2);
        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemMiddle->data.gcMacAddr));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 3, tabledmItem3);
        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemMiddle->data.gcIpAddr));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 4, tabledmItem4);
        if(ItemMiddle->data.nState > 0)
        {
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }else{
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }
        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemMiddle->data.gcType));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 6, tabledmItem6);

        if(ItemMiddle->data.nState <= 0)
        {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(QColor(0,0,0));
            }
        }else {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(Qt::blue);
            }
        }
        ti++;
        }
        if(ItemMiddle->data.nState > 0)
        {
            tOnline++;
        }
        ItemMiddle = ItemMiddle->next;
    }
    CUdpRecv->m_nAllMiddleOnline = tOnline;
    ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));

}
//显示二次门口机在列表
void CSERVERCENTER::_MiddleH(NodeMiddle *ItemMiddle)
{
    int ti = 0;
    int tOnline = 0;
    ui->QMiddle->setRowCount(ti);
    while(ItemMiddle->next != NULL)
    {
        if(ItemMiddle->data.gcIntraAddr[0] == 'H')
        {
        ui->QMiddle->setRowCount(ti + 1);
        ItemMiddle->data.nId = ti;
        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 0, tabledmItem0);
        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddr));

        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 1, tabledmItem1);
        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 2, tabledmItem2);
        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemMiddle->data.gcMacAddr));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 3, tabledmItem3);
        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemMiddle->data.gcIpAddr));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 4, tabledmItem4);
        if(ItemMiddle->data.nState > 0)
        {
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }else{
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }
        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemMiddle->data.gcType));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 6, tabledmItem6);

        if(ItemMiddle->data.nState <= 0)
        {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(QColor(0,0,0));
            }
        }else {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(Qt::blue);
            }
        }
        ti++;
        }
        if(ItemMiddle->data.nState > 0)
        {
            tOnline++;
        }
        ItemMiddle = ItemMiddle->next;
    }
    CUdpRecv->m_nAllMiddleOnline = tOnline;
    ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));

}
//显示公共监视机在列表
void CSERVERCENTER::_MiddleP(NodeMiddle *ItemMiddle)
{
    int ti = 0;
    int tOnline = 0;
    ui->QMiddle->setRowCount(ti);
    while(ItemMiddle->next != NULL)
    {
        if(ItemMiddle->data.gcIntraAddr[0] == 'P')
        {
        ui->QMiddle->setRowCount(ti + 1);
        ItemMiddle->data.nId = ti;
        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 0, tabledmItem0);
        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 1, tabledmItem1);
        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 2, tabledmItem2);
        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemMiddle->data.gcMacAddr));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 3, tabledmItem3);
        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemMiddle->data.gcIpAddr));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 4, tabledmItem4);
        if(ItemMiddle->data.nState > 0)
        {
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }else{
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }
        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemMiddle->data.gcType));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 6, tabledmItem6);

        if(ItemMiddle->data.nState <= 0)
        {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(QColor(0,0,0));
            }
        }else {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(Qt::blue);
            }
        }
        ti++;
        }
        if(ItemMiddle->data.nState > 0)
        {
            tOnline++;
        }
        ItemMiddle = ItemMiddle->next;
    }
    CUdpRecv->m_nAllMiddleOnline = tOnline;
    ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));

}
//显示围墙机在列表
void CSERVERCENTER::_MiddleW(NodeMiddle *ItemMiddle)
{
    int ti = 0;
    int tOnline = 0;
    ui->QMiddle->setRowCount(ti);
    while(ItemMiddle->next != NULL)
    {
        if(ItemMiddle->data.gcIntraAddr[0] == 'W')
        {
        ui->QMiddle->setRowCount(ti + 1);
        ItemMiddle->data.nId = ti;
        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 0, tabledmItem0);
        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 1, tabledmItem1);
        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 2, tabledmItem2);
        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemMiddle->data.gcMacAddr));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 3, tabledmItem3);
        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemMiddle->data.gcIpAddr));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 4, tabledmItem4);
        if(ItemMiddle->data.nState > 0)
        {
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }else{
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }
        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemMiddle->data.gcType));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 6, tabledmItem6);

        if(ItemMiddle->data.nState <= 0)
        {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(QColor(0,0,0));
            }
        }else {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(Qt::blue);
            }
        }
        ti++;
        }
        if(ItemMiddle->data.nState > 0)
        {
            tOnline++;
        }
        ItemMiddle = ItemMiddle->next;
    }
    CUdpRecv->m_nAllMiddleOnline = tOnline;
    ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));

}
//显示中心机在列表
void CSERVERCENTER::_MiddleZ(NodeMiddle *ItemMiddle)
{
    int ti = 0;
    int tOnline = 0;
    ui->QMiddle->setRowCount(ti);
    while(ItemMiddle->next != NULL)
    {
        if(ItemMiddle->data.gcIntraAddr[0] == 'Z')
        {
        ui->QMiddle->setRowCount(ti + 1);
        ItemMiddle->data.nId = ti;
        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 0, tabledmItem0);
        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 1, tabledmItem1);
        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 2, tabledmItem2);
        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemMiddle->data.gcMacAddr));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 3, tabledmItem3);
        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemMiddle->data.gcIpAddr));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 4, tabledmItem4);
        if(ItemMiddle->data.nState > 0)
        {
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }else{
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMiddle->setItem(ti, 5, tabledmItem5);
        }
        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemMiddle->data.gcType));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 6, tabledmItem6);

        if(ItemMiddle->data.nState <= 0)
        {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(QColor(0,0,0));
            }
        }else {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(Qt::blue);
            }
        }
        ti++;
        }
        if(ItemMiddle->data.nState > 0)
        {
            tOnline++;
        }
        ItemMiddle = ItemMiddle->next;
    }
    CUdpRecv->m_nAllMiddleOnline = tOnline;
    ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));

}
//中间设备某个设备信息修改时，更新列表数据
void CSERVERCENTER::_OneUpdateMiddle(QVariant dataVar)
{
    SMiddleDevice data;
    data = dataVar.value<SMiddleDevice>();

    int ti = data.nId ;

        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 0, tabledmItem0);
        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(data.gcAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 1, tabledmItem1);
        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 2, tabledmItem2);
        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(data.gcMacAddr));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 3, tabledmItem3);
        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(data.gcIpAddr));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 4, tabledmItem4);
        if(data.nState <= 0)
        {
            CUdpRecv->m_nAllMiddleOnline ++;
        }
        QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
        tabledmItem5->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 5, tabledmItem5);
        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(data.gcType));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QMiddle->setItem(ti, 6, tabledmItem6);


            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMiddle->item(ti, j);
                item->setTextColor(Qt::blue);
            }
            ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));

}
//按条件查找系统列表里的数据并显示，查找条件有参数SQL确定
void CSERVERCENTER::_UpdateFindSystem(QString SQL)
{
    m_nCount = 0;
    QString strSQL = "SELECT count(*) FROM system WHERE " + SQL;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db),query1(db);

    if(query1.exec(strSQL))
    {
        while(query1.next())
        {
            m_nCount = query1.value(0).toInt();
        }
    }
    ui->QlblAll->setText(QString::number(m_nCount));

    if(m_nCount == 0)
    {
        ui->QSystem->setRowCount(m_nCount);
    }
    else
    {
        SQL = "SELECT *FROM system WHERE " + SQL;
        SQL = SQL + " limit "  + QString::number(m_nPage*PAGE_MAXLEN)+","+QString::number(PAGE_MAXLEN);
        if(query.exec(SQL))
        {
            int ti = 0;
            ui->QSystem->setRowCount(ti);
            while(query.next())
            {
                ui->QSystem->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(m_nPage*PAGE_MAXLEN + ti+1));
                tabledmItem0->setTextAlignment(Qt::AlignCenter);
                ui->QSystem->setItem(ti, 0, tabledmItem0);
                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QSystem->setItem(ti, 1, tabledmItem1);
                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(2).toByteArray().data()));
                tabledmItem2->setTextAlignment(Qt::AlignCenter);
                ui->QSystem->setItem(ti, 2, tabledmItem2);
                QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(3).toByteArray().data()));
                tabledmItem3->setTextAlignment(Qt::AlignCenter);
                ui->QSystem->setItem(ti, 3, tabledmItem3);
                QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem4->setTextAlignment(Qt::AlignCenter);
                ui->QSystem->setItem(ti, 4, tabledmItem4);
                ti++;
            }
                ui->QlblAll->setText(QString::number(ti));
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//初始化刷新最近15天系统日志显示在列表
void CSERVERCENTER::_UpdateSystem()
{
    m_nCount = 0;
    QDateTime dateTime = QDateTime::currentDateTime();
    dateTime = dateTime.addDays(-15);
    QString stime = dateTime.toString("yyyy-MM-dd");

    QString strSQL = "SELECT count(*) FROM system WHERE System_time > '"+stime+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db),query1(db);

    if(query1.exec(strSQL))
    {
        while(query1.next())
        {
            m_nCount = query1.value(0).toInt();//value是指select语句查询出来的结果中的第一个字段
        }
    }
    ui->QlblAll->setText(QString::number(m_nCount));
    if(m_nCount == 0)
    {
        ui->QSystem->setRowCount(m_nCount);
        QMessageBox::information(this, "提示", "没有最近15天的系统记录！");
    }
    else
    {
        int nForm = m_nPage*PAGE_MAXLEN;
        QString SQL = "SELECT *FROM system WHERE  System_time > '"+stime+"' ORDER BY System_time DESC";
        SQL = SQL + " limit " + QString::number(m_nPage*PAGE_MAXLEN)+","+QString::number(PAGE_MAXLEN);
        if(query.exec(SQL))
        {
            int ti = 0;
            ui->QSystem->setRowCount(ti);
            while(query.next())
            {
                ui->QSystem->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(nForm + ti + 1));
                tabledmItem0->setTextAlignment(Qt::AlignCenter);
                ui->QSystem->setItem(ti, 0, tabledmItem0);
                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QSystem->setItem(ti, 1, tabledmItem1);
                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(2).toByteArray().data()));
                tabledmItem2->setTextAlignment(Qt::AlignCenter);
                ui->QSystem->setItem(ti, 2, tabledmItem2);
                QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(3).toByteArray().data()));
                tabledmItem3->setTextAlignment(Qt::AlignCenter);
                ui->QSystem->setItem(ti, 3, tabledmItem3);
                QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem4->setTextAlignment(Qt::AlignCenter);
                ui->QSystem->setItem(ti, 4, tabledmItem4);
                ti++;
            }
                    ui->QlblAll->setText(QString::number(ti));
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

//按条件查找设备日志列表里的数据并显示，查找条件由参数SQL确定
void CSERVERCENTER::_UpdateFindDevice(QString SQL)
{
    m_nCount = 0;
    QString strSQL = "SELECT count(*) FROM device WHERE " + SQL;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db),query1(db);

    if(query1.exec(strSQL))
    {
        while(query1.next())
        {
            m_nCount = query1.value(0).toInt();
        }
    }
    ui->QlblAll->setText(QString::number(m_nCount));

    if(m_nCount == 0)
    {
        ui->QDevice->setRowCount(m_nCount);
    }
    else
    {
        SQL = "SELECT *FROM device WHERE  " + SQL;
        SQL = SQL + " limit "  + QString::number(m_nPage*PAGE_MAXLEN)+","+QString::number(PAGE_MAXLEN);
        if(query.exec(SQL))
        {
            int ti = 0;
            ui->QDevice->setRowCount(ti);
            while(query.next())
            {
                ui->QDevice->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(m_nPage*PAGE_MAXLEN + ti+1));
                tabledmItem0->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 0, tabledmItem0);
                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 1, tabledmItem1);
                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(2).toByteArray().data()));
                tabledmItem2->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 2, tabledmItem2);
                QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(3).toByteArray().data()));
                tabledmItem3->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 3, tabledmItem3);
                QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem4->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 4, tabledmItem4);
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(query.value(6).toByteArray().data()));
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 5, tabledmItem5);
                ti++;
            }
            ui->QlblAll->setText(QString::number(ti));
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

//初始化刷新最近15天设备日志显示在列表
void CSERVERCENTER::_UpdateDevice()
{
    m_nCount = 0;
    QDateTime dateTime = QDateTime::currentDateTime();
    dateTime = dateTime.addDays(-15);
    QString stime = dateTime.toString("yyyy-MM-dd");

    QString strSQL = "SELECT count(*) FROM device WHERE  Device_time > '"+stime+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db),query1(db);

    if(query1.exec(strSQL))
    {
        while(query1.next())
        {
            m_nCount = query1.value(0).toInt();
        }
    }
    ui->QlblAll->setText(QString::number(m_nCount));
    if(m_nCount == 0)
    {
        ui->QDevice->setRowCount(m_nCount);
        QMessageBox::information(this, "提示", "没有最近15天的设备记录！");
    }
    else
    {
        int nForm = m_nPage*PAGE_MAXLEN;
        QString SQL = "SELECT *FROM device WHERE  Device_time > '"+stime+"' ORDER BY Device_time DESC";
        SQL = SQL + " limit " + QString::number(m_nPage*PAGE_MAXLEN)+","+QString::number(PAGE_MAXLEN);
        if(query.exec(SQL))
        {
            int ti = 0;
            ui->QDevice->setRowCount(ti);
            while(query.next())
            {
                ui->QDevice->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(nForm + ti+1));
                tabledmItem0->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 0, tabledmItem0);
                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 1, tabledmItem1);
                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(2).toByteArray().data()));
                tabledmItem2->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 2, tabledmItem2);
                QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(3).toByteArray().data()));
                tabledmItem3->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 3, tabledmItem3);
                QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem4->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 4, tabledmItem4);
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(query.value(6).toByteArray().data()));
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QDevice->setItem(ti, 5, tabledmItem5);
                ti++;
            }
            ui->QlblAll->setText(QString::number(ti));
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//按条件查找刷卡日志列表里的数据并显示，查找条件由参数SQL确定
void CSERVERCENTER::_UpdateFindCardRecord(QString SQL)
{
    m_nCount = 0;
    QString strSQL = "SELECT count(*) FROM card_record WHERE " + SQL;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db), query1(db);

    if(query1.exec(strSQL))
    {
        while(query1.next())
        {
            m_nCount = query1.value(0).toInt();
        }
    }
    ui->QlblAll->setText(QString::number(m_nCount));

    if(m_nCount == 0)
    {
        ui->QCardRecord->setRowCount(m_nCount);
    }
    else
    {
        SQL = "SELECT a.Card_Record_addr,a.Card_Record_device_type,a.Card_Record_card_num,\
                a.Card_addr,a.Card_Record_time FROM card_record  WHERE " + SQL;
        SQL = SQL + " limit "  + QString::number(m_nPage*PAGE_MAXLEN)+","+QString::number(PAGE_MAXLEN);
        if(query.exec(SQL))
        {
            int ti = 0;
            ui->QCardRecord->setRowCount(ti);
            while(query.next())
            {
                ui->QCardRecord->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(m_nPage*PAGE_MAXLEN + ti+1));
                tabledmItem0->setTextAlignment(Qt::AlignCenter);
                ui->QCardRecord->setItem(ti, 0, tabledmItem0);
                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(0).toByteArray().data()));
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QCardRecord->setItem(ti, 1, tabledmItem1);
                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
                tabledmItem2->setTextAlignment(Qt::AlignCenter);
                ui->QCardRecord->setItem(ti, 2, tabledmItem2);
                QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(2).toByteArray().data()));
                tabledmItem3->setTextAlignment(Qt::AlignCenter);
                ui->QCardRecord->setItem(ti, 3, tabledmItem3);
                QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(query.value(3).toByteArray().data()));
                tabledmItem4->setTextAlignment(Qt::AlignCenter);
                ui->QCardRecord->setItem(ti, 4, tabledmItem4);
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QCardRecord->setItem(ti, 5, tabledmItem5);
                ti++;
            }
            ui->QlblAll->setText(QString::number(ti));
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//初始化刷新最近15天刷卡日志显示在列表
void CSERVERCENTER::_UpdateCardRecord()
{
    m_nCount = 0;
    QDateTime dateTime=QDateTime::currentDateTime();
    dateTime = dateTime.addDays(-15);
    QString stime = dateTime.toString("yyyy-MM-dd");
    QString strSQL = "SELECT count(*) FROM card_record WHERE  Card_Record_time > '"+stime+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db), query1(db);

    if(query1.exec(strSQL))
    {
        while(query1.next())
        {
            m_nCount = query1.value(0).toInt();
        }
    }
    ui->QlblAll->setText(QString::number(m_nCount));
    if(m_nCount == 0)
    {
        ui->QCardRecord->setRowCount(m_nCount);
        QMessageBox::information(this, "提示", "没有最近15天的刷卡记录！");
    }
    else
    {
        int nForm = m_nPage*PAGE_MAXLEN;
        QString SQL = "SELECT a.Card_Record_addr,a.Card_Record_device_type,a.Card_Record_card_num,\
                a.Card_addr,a.Card_Record_time FROM card_record a WHERE \
                a.Card_Record_time > '"+stime+"' ORDER BY a.Card_Record_time DESC";
        SQL = SQL + " limit " + QString::number(m_nPage*PAGE_MAXLEN)+","+QString::number(PAGE_MAXLEN);
        if(query.exec(SQL))
        {
            int ti = 0;
            ui->QCardRecord->setRowCount(ti);
            while(query.next())
            {
                ui->QCardRecord->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(nForm + ti+1));
                tabledmItem0->setTextAlignment(Qt::AlignCenter);

                ui->QCardRecord->setItem(ti, 0, tabledmItem0);
                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(0).toByteArray().data()));
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QCardRecord->setItem(ti, 1, tabledmItem1);
                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
                tabledmItem2->setTextAlignment(Qt::AlignCenter);
                ui->QCardRecord->setItem(ti, 2, tabledmItem2);
                QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(2).toByteArray().data()));
                tabledmItem3->setTextAlignment(Qt::AlignCenter);
                ui->QCardRecord->setItem(ti, 3, tabledmItem3);
                QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(query.value(3).toByteArray().data()));
                tabledmItem4->setTextAlignment(Qt::AlignCenter);
                ui->QCardRecord->setItem(ti, 4, tabledmItem4);
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QCardRecord->setItem(ti, 5, tabledmItem5);
                ti++;
            }
            ui->QlblAll->setText(QString::number(ti));
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//初始化跟新IC卡列表
void CSERVERCENTER::_UpdateIcCard()
{
    QString SQL = "SELECT IC_Card_num,IC_Card_addr,IC_Card_time from ic_card ORDER BY IC_Card_time DESC";
    m_strFindIcCard = SQL;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    if(query.exec(SQL))
    {
        int ti = 0;
        ui->QIcCard->setRowCount(ti);
        while(query.next())
        {
            ui->QIcCard->setRowCount(ti + 1);
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 0, tabledmItem0);
            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(0).toByteArray().data()));
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 1, tabledmItem1);
            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 2, tabledmItem2);
            QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(2).toDate().toString("yyyy-MM-dd")));
            tabledmItem3->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 3, tabledmItem3);
            ti++;
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//2015 03 18
//更新IC卡对应的权限列表
void CSERVERCENTER::_CheckCardMessage()
{
    int nRow = ui->QIcCard->currentRow();
    QString strCardNum = ui->QIcCard->item(nRow, 1)->text();
    QString SQL = "SELECT *FROM ic_card WHERE IC_Card_num = '"+strCardNum+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    NodeCard *p = m_ItemCard;
    if(query.exec(SQL))
    {
        if(query.next())
        {
            for(int i = 0;i < ui->QCardMessage->rowCount();i++)
            {
                if(p->next != NULL)
                {
                    p->data.nNewCheck = query.value(i+4).toInt();
                    p->data.nOldCheck = query.value(i+4).toInt();
                    p = p->next;
                }
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
                if(query.value(i+4).toInt())
                {
                    tabledmItem0->setCheckState(Qt::Checked);
                }
                else
                {
                    tabledmItem0->setCheckState(Qt::Unchecked);
                }
                tabledmItem0->setText(QString::number(i+1));
                ui->QCardMessage->setItem(i, 0, tabledmItem0);
            }
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//IC卡链表更新插入新卡
void CSERVERCENTER::_InsertItem(SCard data)
{
    NodeCard *p = m_ItemCard;
    if(p->next == NULL)
    {
        NodeCard *q;
        q = (NodeCard *)malloc(sizeof(NodeCard));
        q->data = data;
        q->next = p;
        p = q;
        m_ItemCard = p;
        return;
    }
    while(p->next)
    {
        p = p->next;
    }
    NodeCard *q;
    q = (NodeCard *)malloc(sizeof(NodeCard));
    q->data = p->data;
    q->next = p->next;
    p->data = data;
    p->next = q;
    return;
}
//情况IC卡链表
void CSERVERCENTER::_ClearItem()
{
    NodeCard *q;
    q = (NodeCard *)malloc(sizeof(NodeCard));
    if(m_ItemCard->next == NULL) return;
    while(m_ItemCard->next)
    {
      q=m_ItemCard->next;
      delete(m_ItemCard);
      m_ItemCard=q;
    }
    return;
}

//地址转换->中文
QString CSERVERCENTER::_AddrToAddrExplain(QString Addr,QString strType)
{
    QString str;
     if(strType == "二次门口机")
     {
         QString str1 = Addr;
         str = str1.left(4) + "栋";
         str1 = str1.right(str1.length() - 4);

         str = str + str1.left(3) + "单元";
         str1 = str1.right(str1.length() - 3);

         str = str + str1.left(3) + "楼";
         str1 = str1.right(str1.length() - 3);

         str = str + str1.left(3) + "房";
         str1 = str1.right(str1.length() - 3);

         str = str + str1.left(3) + "号设备";
     }
     else  if((strType == "围墙机") || (strType == "门口机"))
     {
         QString str1 = Addr;
         str = str1.left(4) + "栋";
         str1 = str1.right(str1.length() - 4);

         str = str + str1.left(3) + "单元";
         str1 = str1.right(str1.length() - 3);
         str1 = str1.right(str1.length() - 6);

         str = str + str1.left(3) + "号设备";
     }
    return str;
}

QString CSERVERCENTER::_AddrToAddrExplainA8(QString Addr, QString strType)
{
    QString str;
    if(strType == "二次门口机"||strType == "室内机")
    {
        QString str1 = Addr;
        str = str1.left(5) + "栋";
        str1 = str1.right(str1.length() - 5);

        str = str + str1.left(2) + "单元";
        str1 = str1.right(str1.length() - 2);

        str = str + str1.left(2) + "楼";
        str1 = str1.right(str1.length() - 2);

        str = str + str1.left(2) + "房";
        str1 = str1.right(str1.length() - 2);

        str = str + str1.left(1) + "号设备";
    }
    else if(strType == "门口机")
    {
        QString str1 = Addr;
        str = str1.left(5) + "栋";
        str1 = str1.right(str1.length() - 5);

        str = str + str1.left(2) + "单元";
        str1 = str1.right(str1.length() - 2);

        str = str + str1.left(1) + "号设备";
    }
    else if(strType == "围墙机")
    {
        QString str1 = Addr;
        str = str1.left(5) + "栋";
        str1 = str1.right(str1.length() - 5);

        str = str + str1.left(1) + "号设备";
    }
    else if(strType == "中心机")
    {
        QString str1 = Addr;
        str = str1.left(5) + "栋";
        str1 = str1.right(str1.length() - 5);

        str = str + str1.left(1) + "号设备";

    }
    return str;
}

//->数字
QString CSERVERCENTER::_AddrExplainToAddr(QString AddrExplain,QString strType)
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

    if(strType == "室内机")
    {
        str = "S" + str;
    }
    else if(strType == "门口机")
    {
        str = "M" + str;
    }
    else if(strType == "围墙机")
    {
        str = "W" + str;
    }
    else if(strType == "中心机")
    {
        str = "Z" + str;
    }
    else if(strType == "别墅室内机")
    {
        str = "B" + str;
    }
    else if(strType == "二次门口机")
    {
        str = "H" + str;
    }
    return str;
}
//yes
QString CSERVERCENTER::_AddrExplainToAddr(QString AddrExplain,QString strType,int n)
{
    QString str;
    if(strType == "二次门口机")
    {
        QString str1 = AddrExplain;
        str = str1.left(4);
        str1 = str1.right(str1.length() - 5);

        str = str + str1.left(3);
        str1 = str1.right(str1.length() - 5);

        str = str + str1.left(3);
        str1 = str1.right(str1.length() - 4);

        str = str + str1.left(3);
        str1 = str1.right(str1.length() - 4);

        str = str + str1.left(3);
    }
    else if(strType == "围墙机")
    {
        QString str1 = AddrExplain;
        str = str1.left(4);
        str1 = str1.right(str1.length() - 5);

        str = str + str1.left(3);
        str1 = str1.right(str1.length() - 5);

        str = str +"000000"+ str1.left(3);
    }
    else if(strType == "门口机")
    {
        QString str1 = AddrExplain;
        str = str1.left(4);
        str1 = str1.right(str1.length() - 5);

        str = str + str1.left(3);
        str1 = str1.right(str1.length() - 5);

        str = str +"000000"+ str1.left(3);
    }
     return str;
}

QString CSERVERCENTER::_AddrExplainToAddrA8(QString AddrExplain,QString strType,int n)
{
    QString str;
    if(AddrExplain.left(1)=="H")
        strType = "二次门口机";
    if(strType == "二次门口机"||strType == "室内机")
    {
        QString str1 = AddrExplain;
        str = str1.left(5);
        str1 = str1.right(str1.length() - 6);

        str = str + str1.left(2);
        str1 = str1.right(str1.length() - 4);

        str = str + str1.left(2);
        str1 = str1.right(str1.length() - 3);

        str = str + str1.left(2);
        str1 = str1.right(str1.length() - 3);

        str = str + str1.left(1);
    }
    else if(strType == "围墙机" || strType == "中心机")
    {
        QString str1 = AddrExplain;
        str = str1.left(5);
        str1 = str1.right(str1.length() - 6);
        str = str + str1.left(1);
//        str = str + "0000000";

    }
    else if(strType == "门口机")
    {
        QString str1 = AddrExplain;
        str = str1.left(5);
        str1 = str1.right(str1.length() - 6);

        str = str + str1.left(2);
        str1 = str1.right(str1.length() - 4);

        str = str + str1.left(1);
//        str = str + "0000";
    }
    return str;
}

//20150305
//初始化IC卡权限列表
void CSERVERCENTER::_UpdateCardMessage()
{
    QString SQL = "SHOW FIELDS FROM ic_card";
//    QSqlQuery query;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    if(query.exec(SQL))
    {
        _ClearItem();
        int ti = 0;
        int tj = 0;
        ui->QCardMessage->setRowCount(ti);
        while(query.next())
        {
            tj ++;
            if(tj > 4)
            {
                SCard sCard;
                sCard.nOldCheck = 0;
                sCard.nNewCheck = 0;
                memset(sCard.gcAddr,0,20);
                ui->QCardMessage->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
                tabledmItem0->setCheckState(Qt::Unchecked);
                tabledmItem0->setFlags(Qt::ItemIsEnabled);
                tabledmItem0->setText(QString::number(ti+1));
                ui->QCardMessage->setItem(ti, 0, tabledmItem0);
                QString strIntraAddr = QString(query.value(0).toByteArray().data());
                QString strType;
                if(strIntraAddr.left(1) == "M"){
                    strType = "门口机";
                }else if(strIntraAddr.left(1) == "W"){
                    strType = "围墙机";
                }else if(strIntraAddr.left(1) == "H"){
                    strType = "二次门口机";
                }else if(strIntraAddr.left(1) == "Z"){
                    strType = "中心机";
                }
                memcpy(sCard.gcAddr,query.value(0).toByteArray().data(),query.value(0).toByteArray().size());
                _InsertItem(sCard);
                QString strAddr;

                strAddr.clear();
                strAddr = _AddrToAddrExplain(strIntraAddr,strType);
//                strIntraAddr = strIntraAddr.right(strIntraAddr.length() - 1);
//                strAddr = strIntraAddr.left(3);
//                strIntraAddr = strIntraAddr.right(strIntraAddr.length() - 3);
//                strAddr = strAddr +"-";
//                strAddr = strAddr + strIntraAddr.left(3);
//                strIntraAddr = strIntraAddr.right(strIntraAddr.length() - 3);
//                strAddr = strAddr +"-";
//                strAddr = strAddr + strIntraAddr.left(3);
//                strIntraAddr = strIntraAddr.right(strIntraAddr.length() - 3);
//                strAddr = strAddr +"-";
//                strAddr = strAddr + strIntraAddr.left(3);
//                strIntraAddr = strIntraAddr.right(strIntraAddr.length() - 3);
//                strAddr = strAddr +"-";
//                strAddr = strAddr + strIntraAddr.left(3);


                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(strAddr);
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QCardMessage->setItem(ti, 1, tabledmItem1);
                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(strType);
                tabledmItem2->setTextAlignment(Qt::AlignCenter);
                ui->QCardMessage->setItem(ti, 2, tabledmItem2);
                ti++;
            }
        }

    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//初始化更新报修类型列表
void CSERVERCENTER::_UpdateRepairs()
{
    QString SQL = "SELECT *FROM repairs";
//    QSqlQuery query;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    QSqlQuery query1(db);
    int i = query.exec(SQL);
    SQL.clear();
    SQL = "SELECT count(*) from repairs ";
//    QSqlQuery query1;
    int i1 = query1.exec(SQL);
    if(i&&i1)
    {
        int nRecordLen = 0;
        if(query1.next())
         nRecordLen = query1.value(0).toInt();
        int ti = 0;
        ui->QRepairs->setRowCount(nRecordLen);
        while(query.next())
        {
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QRepairs->setItem(ti, 0, tabledmItem0);
            ti++;
         }
     }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//初始化更新常用电话列表
void CSERVERCENTER::_UpdatePhone()
{
    QString SQL = "SELECT *FROM useful_phone";
//    QSqlQuery query;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    QSqlQuery query1(db);
    int i = query.exec(SQL);
    SQL.clear();
    SQL = "SELECT count(*) from useful_phone ";
//    QSqlQuery query1;
    int i1 = query1.exec(SQL);
    if(i&&i1)
    {
        int nRecordLen = 0;
        if(query1.next())
         nRecordLen = query1.value(0).toInt();
        int ti = 0;
        ui->QPhone->setRowCount(nRecordLen);
        while(query.next())
        {
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QPhone->setItem(ti, 0, tabledmItem0);
            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(2).toByteArray().data()));
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->QPhone->setItem(ti, 1, tabledmItem1);
            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(3).toByteArray().data()));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->QPhone->setItem(ti, 2, tabledmItem2);
            ti++;
         }
     }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//初始化更新操作员管理列表
void CSERVERCENTER::_UpdateUser()
{
    QString SQL = "SELECT *FROM user  ORDER BY User_grade ASC";
//    QSqlQuery query;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);

    if(query.exec(SQL))
    {
        int ti = 0;
        ui->QUser->setRowCount(ti);
        while(query.next())
        {
            int nGrade = query.value(4).toInt();
            if(nGrade >= m_sUser.nGrade)
            {
            ui->QUser->setRowCount(ti + 1);
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QUser->setItem(ti, 0, tabledmItem0);
            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->QUser->setItem(ti, 1, tabledmItem1);
            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(3).toByteArray().data()));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->QUser->setItem(ti, 2, tabledmItem2);
            QString strGrade;
            switch(nGrade)
            {
             case 1:
                strGrade = "超级管理员";
                break;
            case 2:
                strGrade = "管理员";
                break;
            case 3:
                strGrade = "操作员";
                break;
            default :
                break;
            }

            QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(strGrade);
            tabledmItem3->setTextAlignment(Qt::AlignCenter);
            ui->QUser->setItem(ti, 3, tabledmItem3);
            ti++;
            }
        }
        ui->QlblAll->setText(QString::number(ti));
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

//按条件查询报修记录，条件由参数SQL定
void CSERVERCENTER::_UpdateFindRepairsRecord(QString SQL)
{
    m_nCount = 0;
    QString strSQL = "SELECT count(*) FROM repairs_record WHERE " + SQL;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db), query1(db);

    if(query1.exec(strSQL))
    {
        while(query1.next())
        {
            m_nCount = query1.value(0).toInt();
        }
    }
    ui->QlblAll->setText(QString::number(m_nCount));

    if(m_nCount == 0)
    {
        ui->QRepairsRecord->setRowCount(m_nCount);
    }
    else
    {
        SQL = "SELECT b.Repairs_Record_addr,b.Repairs_Record_type,\
                b.Repairs_Record_stime,b.Repairs_Record_etime,b.Repairs_Record_dtime,\
                b.Repairs_Record_deal_human,b.Repairs_Record_message,b.Repairs_Record_device_type from repairs_record b \
                WHERE  " + SQL;
        SQL = SQL + " limit "  + QString::number(m_nPage*PAGE_MAXLEN)+","+QString::number(PAGE_MAXLEN);
        if(query.exec(SQL))
        {
            int ti = 0;
            ui->QRepairsRecord->setRowCount(ti);
            while(query.next())
            {
                ui->QRepairsRecord->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(m_nPage*PAGE_MAXLEN + ti+1));
                tabledmItem0->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 0, tabledmItem0);
                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(0).toByteArray().data()));
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 1, tabledmItem1);
                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
                tabledmItem2->setTextAlignment(Qt::AlignCenter);

                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem(QString(query.value(7).toByteArray().data()));
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 2, tabledmItem8);

                ui->QRepairsRecord->setItem(ti, 3, tabledmItem2);
                QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(2).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem3->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 4, tabledmItem3);
                QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(query.value(3).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem4->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 5, tabledmItem4);
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 6, tabledmItem5);
                QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(query.value(5).toByteArray().data()));
                tabledmItem6->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 7, tabledmItem6);
                QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(query.value(6).toByteArray().data()));
                tabledmItem7->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 8, tabledmItem7);

                ti++;
            }
            ui->QlblAll->setText(QString::number(ti));
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);

}

//初始化显示最近15天报修记录
void CSERVERCENTER::_UpdateRepairsRecord()
{
    m_nCount = 0;
    QDateTime dateTime = QDateTime::currentDateTime();
    dateTime = dateTime.addDays(-15);
    QString stime = dateTime.toString("yyyy-MM-dd");
    QString strSQL = "SELECT count(*) FROM repairs_record WHERE  Repairs_Record_dtime > '"+stime+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db), query1(db);

    if(query1.exec(strSQL))
    {
        while(query1.next())
        {
            m_nCount = query1.value(0).toInt();
        }
    }
    ui->QlblAll->setText(QString::number(m_nCount));
    if(m_nCount == 0)
    {
        ui->QRepairsRecord->setRowCount(m_nCount);
        QMessageBox::information(this, "提示", "没有最近15天的报修记录！");
    }
    else
    {

        int nForm = m_nPage*PAGE_MAXLEN;
        QString SQL = "SELECT b.Repairs_Record_addr,b.Repairs_Record_type,\
                b.Repairs_Record_stime,b.Repairs_Record_etime,b.Repairs_Record_dtime,\
                b.Repairs_Record_deal_human,b.Repairs_Record_message,b.Repairs_Record_device_type from repairs_record b WHERE  b.Repairs_Record_stime > '"+stime+"' ORDER BY  b.Repairs_Record_dtime,b.Repairs_Record_stime DESC";
        SQL = SQL + " limit " + QString::number(m_nPage*PAGE_MAXLEN)+","+QString::number(PAGE_MAXLEN);
        if(query.exec(SQL))
        {
            int ti = 0;
            ui->QRepairsRecord->setRowCount(ti);
            while(query.next())
            {
                ui->QRepairsRecord->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(nForm + ti+1));
                tabledmItem0->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 0, tabledmItem0);
                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(0).toByteArray().data()));
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 1, tabledmItem1);

                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem(QString(query.value(7).toByteArray().data()));
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 2, tabledmItem8);

                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
                tabledmItem2->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 3, tabledmItem2);
                QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(2).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem3->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 4, tabledmItem3);
                QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(query.value(3).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem4->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 5, tabledmItem4);
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 6, tabledmItem5);
                QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(query.value(5).toByteArray().data()));
                tabledmItem6->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 7, tabledmItem6);
                QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(query.value(6).toByteArray().data()));
                tabledmItem7->setTextAlignment(Qt::AlignCenter);
                ui->QRepairsRecord->setItem(ti, 8, tabledmItem7);

                ti++;
            }
            ui->QlblAll->setText(QString::number(ti));
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);

}
//处理报警后更新设备链表报警状态
void CSERVERCENTER::_UpdateTenementToAlarm()
{
    NodeTenement *p;
    p = CUdpRecv->m_TenementItem;
    while(p->next)
    {
        p->data.nAlarmState = 0;
        p = p->next;
    }
}

//初始化显示最近15天报警记录列表
void CSERVERCENTER::_UpdateAlarmRecord()
{
    m_nCount = 0;
    QDateTime dateTime=QDateTime::currentDateTime();
    dateTime = dateTime.addDays(-15);
    QString stime = dateTime.toString("yyyy-MM-dd");
    QString strSQL = "SELECT count(*) FROM alarm a WHERE  a.Alarm_stime > '"+stime+"'";
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db), query1(db);

    if(query1.exec(strSQL))
    {
        while(query1.next())
        {
            m_nCount = query1.value(0).toInt();
        }
    }
    ui->QlblAll->setText(QString::number(m_nCount));
    if(m_nCount == 0)
    {
        ui->QAlarmRecord->setRowCount(m_nCount);
        QMessageBox::information(this, "提示", "没有最近15天的报警记录");
    }
    else
    {
        int nForm = m_nPage*PAGE_MAXLEN;
        QString SQL = "SELECT a.Alarm_ip_addr,a.Alarm_addr,a.Alarm_device_type,a.Alarm_name,a.Alarm_phone,a.Alarm_fence,\
                a.Alarm_type,a.Alarm_stime,a.Alarm_dtime,a.Alarm_etime,a.Alarm_status,a.Alarm_deal_human,a.Alarm_deal_message \
                FROM alarm a WHERE  a.Alarm_stime > '"+stime+"' ORDER BY  a.Alarm_dtime,a.Alarm_stime DESC";
        SQL = SQL + " limit " + QString::number(m_nPage*PAGE_MAXLEN)+","+QString::number(PAGE_MAXLEN);
        if(query.exec(SQL))
        {
            int ti = 0;
            ui->QAlarmRecord->setRowCount(ti);
            while(query.next())
            {
                ui->QAlarmRecord->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(nForm + ti+1));
                tabledmItem0->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 0, tabledmItem0);
                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(0).toByteArray().data()));
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 1, tabledmItem1);
                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
                tabledmItem2->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 2, tabledmItem2);
                QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(2).toByteArray().data()));
                tabledmItem3->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 3, tabledmItem3);
                QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(query.value(3).toByteArray().data()));
                tabledmItem4->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 4, tabledmItem4);
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(query.value(4).toByteArray().data()));
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 5, tabledmItem5);
                QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(query.value(5).toByteArray().data()));
                tabledmItem6->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 6, tabledmItem6);
                QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(query.value(6).toByteArray().data()));
                tabledmItem7->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 7, tabledmItem7);
                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem(query.value(7).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 8, tabledmItem8);
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem(query.value(8).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 9, tabledmItem9);
                QTableWidgetItem *tabledmItem10 = new QTableWidgetItem(query.value(9).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                tabledmItem10->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 10, tabledmItem10);
                QTableWidgetItem *tabledmItem11 = new QTableWidgetItem(QString(query.value(10).toByteArray().data()));
                tabledmItem11->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 11, tabledmItem11);
                QTableWidgetItem *tabledmItem12 = new QTableWidgetItem(QString(query.value(11).toByteArray().data()));
                tabledmItem12->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 12, tabledmItem12);
                QTableWidgetItem *tabledmItem13 = new QTableWidgetItem(QString(query.value(12).toByteArray().data()));
                tabledmItem13->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 13, tabledmItem13);
                ti++;
            }
                ui->QlblAll->setText(QString::number(ti));
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

//按条件查找更新报警记录列表，条件有参数SQL确定
void CSERVERCENTER::_UpdateFindAlarmRecord(QString SQL)
{
    m_nCount = 0;
    QString strSQL = "SELECT count(*) FROM alarm WHERE " + SQL;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db), query1(db);

    if(query1.exec(strSQL))
    {
        while(query1.next())
        {
            m_nCount = query1.value(0).toInt();
        }
    }
    ui->QlblAll->setText(QString::number(m_nCount));

    if(m_nCount == 0)
    {
        ui->QAlarmRecord->setRowCount(m_nCount);
    }
    else
    {
        SQL = "SELECT a.Alarm_ip_addr,a.Alarm_addr,a.Alarm_device_type,a.Alarm_name,a.Alarm_phone,a.Alarm_fence,\
                a.Alarm_type,a.Alarm_stime,a.Alarm_dtime,a.Alarm_etime,a.Alarm_status,a.Alarm_deal_human,a.Alarm_deal_message \
                FROM alarm a WHERE " + SQL;
        SQL = SQL + " limit "  + QString::number(m_nPage*PAGE_MAXLEN)+"," + QString::number(PAGE_MAXLEN);
        if(query.exec(SQL))
        {
            int ti = 0;
            ui->QAlarmRecord->setRowCount(ti);
            while(query.next())
            {
                ui->QAlarmRecord->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(m_nPage*PAGE_MAXLEN + ti+1));
                tabledmItem0->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 0, tabledmItem0);
                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(0).toByteArray().data()));
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 1, tabledmItem1);
                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
                tabledmItem2->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 2, tabledmItem2);
                QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(2).toByteArray().data()));
                tabledmItem3->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 3, tabledmItem3);
                QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(query.value(3).toByteArray().data()));
                tabledmItem4->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 4, tabledmItem4);
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(query.value(4).toByteArray().data()));
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 5, tabledmItem5);
                QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(query.value(5).toByteArray().data()));
                tabledmItem6->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 6, tabledmItem6);
                QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(query.value(6).toByteArray().data()));
                tabledmItem7->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 7, tabledmItem7);
                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem(query.value(7).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 8, tabledmItem8);
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem(query.value(8).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 9, tabledmItem9);
                QTableWidgetItem *tabledmItem10 = new QTableWidgetItem(query.value(9).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                tabledmItem10->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 10, tabledmItem10);
                QTableWidgetItem *tabledmItem11 = new QTableWidgetItem(QString(query.value(10).toByteArray().data()));
                tabledmItem11->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 11, tabledmItem11);
                QTableWidgetItem *tabledmItem12 = new QTableWidgetItem(QString(query.value(11).toByteArray().data()));
                tabledmItem12->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 12, tabledmItem12);
                QTableWidgetItem *tabledmItem13 = new QTableWidgetItem(QString(query.value(12).toByteArray().data()));
                tabledmItem13->setTextAlignment(Qt::AlignCenter);
                ui->QAlarmRecord->setItem(ti, 13, tabledmItem13);
                ti++;
            }
            ui->QlblAll->setText(QString::number(ti));
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

//通过报警链表更新报警列表
void CSERVERCENTER::_UpdateAlarm(NodeAlarm *ItemAlarm)
{
    int ti = 0;
    ui->QAlarm->setRowCount(ti);
    while(ItemAlarm->next != NULL)
    {
        ui->QAlarm->setRowCount(ti + 1);
        ItemAlarm->data.nId = ti;

        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QAlarm->setItem(ti, 0, tabledmItem0);

        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemAlarm->data.gcIpAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QAlarm->setItem(ti, 1, tabledmItem1);

        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemAlarm->data.gcAddr));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QAlarm->setItem(ti, 2, tabledmItem2);

        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemAlarm->data.gcName));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QAlarm->setItem(ti, 3, tabledmItem3);

        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemAlarm->data.gcPhone1));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QAlarm->setItem(ti, 4, tabledmItem4);

        QString str2 = "防区" + QString::number(ItemAlarm->data.nFenceId);
        QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(str2);
        tabledmItem5->setTextAlignment(Qt::AlignCenter);
        ui->QAlarm->setItem(ti, 5, tabledmItem5);

        QString str3;


        switch(ItemAlarm->data.nAlarmType)
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
            break;
        default :
            str3 = "错误";
            break;
        }


        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(str3);
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QAlarm->setItem(ti, 6, tabledmItem6);

        QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(ItemAlarm->data.gcStime));
        tabledmItem7->setTextAlignment(Qt::AlignCenter);
        ui->QAlarm->setItem(ti, 7, tabledmItem7);

        QTableWidgetItem *tabledmItem8 = new QTableWidgetItem(QString(ItemAlarm->data.gcEtime));
        tabledmItem8->setTextAlignment(Qt::AlignCenter);
        ui->QAlarm->setItem(ti, 8, tabledmItem8);

        QTableWidgetItem *tabledmItem9 = new QTableWidgetItem(QString(ItemAlarm->data.gcType));
        tabledmItem9->setTextAlignment(Qt::AlignCenter);
        ui->QAlarm->setItem(ti, 9, tabledmItem9);

        ItemAlarm = ItemAlarm->next;
        ti ++;
    }

}

//按照住户的设备地址模糊查找住户设备并显示在住户列表
void CSERVERCENTER::_UpdateFindTenement(NodeTenement *ItemTenement,int nIndex,QString strFind)
{
    int ti = 0;
    ui->QTenement->setRowCount(ti);
    m_nFindDevice = 1;
    while(ItemTenement->next != NULL)
    {
        int nFind = -1;
        switch(nIndex)
        {
        case 0:
            nFind = QString(ItemTenement->data.gcIntraAddr).indexOf(strFind);
            break;
        case 1:
            nFind = QString(ItemTenement->data.gcIpAddr).indexOf(strFind);
            break;
        case 2:
            nFind = QString(ItemTenement->data.gcMacAddr).indexOf(strFind);
            break;
        case 3:
            nFind = QString(ItemTenement->data.gcName).indexOf(strFind);
            break;
        case 4:
            nFind = QString(ItemTenement->data.gcPhone1).indexOf(strFind);
            break;
        default :
            break;
        }
        if(nFind >= 0)
        {
        ui->QTenement->setRowCount(ti + 1);
        ItemTenement->data.nId = ti;

        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 0, tabledmItem0);

        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemTenement->data.gcAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 1, tabledmItem1);

        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemTenement->data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 2, tabledmItem2);

        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemTenement->data.gcName));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 3, tabledmItem3);

        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone1));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 4, tabledmItem4);

        QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone2));
        tabledmItem5->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 5, tabledmItem5);

        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemTenement->data.gcMacAddr));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 6, tabledmItem6);

        QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(ItemTenement->data.gcIpAddr));
        tabledmItem7->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 7, tabledmItem7);

        if(ItemTenement->data.nState <= 0)
        {
            QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("未连接");
            tabledmItem8->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 8, tabledmItem8);
        }else{
            QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("已连接");
            tabledmItem8->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 8, tabledmItem8);
        }

        if(ItemTenement->data.nFenceState == 0)
        {
            QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("撤防");
            tabledmItem9->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 9, tabledmItem9);
        }else{
            switch(ItemTenement->data.nFenceState)
            {
            case 1:
            {
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("外出布防");
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 9, tabledmItem9);
                break;
            }
            case 2:
            {
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("在家布防");
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 9, tabledmItem9);
                break;
            }
            default :
            {
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("未知布防");
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 9, tabledmItem9);
                break;
            }
            }
        }

        QTableWidgetItem *tabledmItem11 = new QTableWidgetItem(QString(ItemTenement->data.gcType));
        tabledmItem11->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 11, tabledmItem11);

        if(ItemTenement->data.nAlarmState == 0)
        {
            QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("无报警");
            tabledmItem10->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 10, tabledmItem10);
            for(int j = 0;j < 12;j++)
            {
                QTableWidgetItem *item = ui->QTenement->item(ti, j);
                item->setBackgroundColor(QColor(0,125,0,0));
            }
        }else{
            QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("报警");
            tabledmItem10->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 10, tabledmItem10);
            for(int j = 0;j < 12;j++)
            {
                QTableWidgetItem *item = ui->QTenement->item(ti, j);
                item->setBackgroundColor(QColor(255,0,0));
            }
        }

        if(ItemTenement->data.nState <= 0)
        {
            for(int j = 0;j < 12;j++)
            {
                QTableWidgetItem *item = ui->QTenement->item(ti, j);
                item->setTextColor(QColor(0,0,0));
            }
        }else{
            for(int j = 0;j < 12;j++)
            {
                QTableWidgetItem *item = ui->QTenement->item(ti, j);
                item->setTextColor(Qt::blue);
            }
        }
        ti++;
        }
        ItemTenement = ItemTenement->next;
    }
}
//更新住户设备列表通过住户链表
void CSERVERCENTER::_UpdateTenement(NodeTenement *ItemTenement)
{
    int ti = 0;
    int tOnline = 0;
    ui->QTenement->setRowCount(ti);
    while(ItemTenement->next != NULL)
    {
        ui->QTenement->setRowCount(ti + 1);
        ItemTenement->data.nId = ti;

        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 0, tabledmItem0);

        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemTenement->data.gcAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 1, tabledmItem1);

        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemTenement->data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 2, tabledmItem2);

        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemTenement->data.gcName));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 3, tabledmItem3);

        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone1));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 4, tabledmItem4);

        QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone2));
        tabledmItem5->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 5, tabledmItem5);

        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemTenement->data.gcMacAddr));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 6, tabledmItem6);

        QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(ItemTenement->data.gcIpAddr));
        tabledmItem7->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 7, tabledmItem7);

        if(ItemTenement->data.nState <= 0)
        {
            QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("未连接");
            tabledmItem8->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 8, tabledmItem8);
        }else{
            tOnline++;
            QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("已连接");
            tabledmItem8->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 8, tabledmItem8);
        }

        if(ItemTenement->data.nFenceState == 0)
        {
            QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("撤防");
            tabledmItem9->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 9, tabledmItem9);
        }else{
            switch(ItemTenement->data.nFenceState)
            {
            case 1:
            {
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("外出布防");
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 9, tabledmItem9);
                break;
            }
            case 2:
            {
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("在家布防");
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 9, tabledmItem9);
                break;
            }
            default :
            {
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("未知布防");
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 9, tabledmItem9);
                break;
            }
            }
        }

        QTableWidgetItem *tabledmItem11 = new QTableWidgetItem(QString(ItemTenement->data.gcType));
        tabledmItem11->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 11, tabledmItem11);

        if(ItemTenement->data.nAlarmState == 0)
        {
            QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("无报警");
            tabledmItem10->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 10, tabledmItem10);
            for(int j = 0;j < 12;j++)
            {
                QTableWidgetItem *item = ui->QTenement->item(ti, j);
                item->setBackgroundColor(QColor(0,125,0,0));
            }
        }else{
            QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("报警");
            tabledmItem10->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 10, tabledmItem10);
            for(int j = 0;j < 12;j++)
            {
                QTableWidgetItem *item = ui->QTenement->item(ti, j);
                item->setBackgroundColor(QColor(255,0,0));
            }
        }

        if(ItemTenement->data.nState <= 0)
        {
            for(int j = 0;j < 12;j++)
            {
                QTableWidgetItem *item = ui->QTenement->item(ti, j);
                item->setTextColor(QColor(0,0,0));
            }
        }else{
            for(int j = 0;j < 12;j++)
            {
                QTableWidgetItem *item = ui->QTenement->item(ti, j);
                item->setTextColor(Qt::blue);
            }
        }
        ti++;
        ItemTenement = ItemTenement->next;
    }
    CUdpRecv->m_nAllTenementOnline = tOnline;
    ui->QlblOnlineTenement->setText(QString::number(CUdpRecv->m_nAllTenementOnline));
}


void CSERVERCENTER::closeEvent( QCloseEvent * event )
{
    if(this->isVisible())
    {
    hide();
    event->ignore();
    }else
        event->accept();
}
//初始化树形目录
void CSERVERCENTER::_InitTree()
{
    ui->QTree->setHeaderHidden(true);
    QTreeWidgetItem* B1 = new QTreeWidgetItem(QStringList()<<"设备信息");
    QTreeWidgetItem* B2 = new QTreeWidgetItem(QStringList()<<"小区短信");
    QTreeWidgetItem* B3 = new QTreeWidgetItem(QStringList()<<"日志");
    QTreeWidgetItem* B4 = new QTreeWidgetItem(QStringList()<<"记录");
    QTreeWidgetItem* B5 = new QTreeWidgetItem(QStringList()<<"报修记录");
    QTreeWidgetItem* B6 = new QTreeWidgetItem(QStringList()<<"IC卡信息");
    QTreeWidgetItem* B7 = new QTreeWidgetItem(QStringList()<<"管理");

    B1->setIcon(0,QIcon(":/ico/傲游浏览器.ico"));
    B2->setIcon(0,QIcon(":/ico/IMSmallToolbarQRing.ico"));
    B3->setIcon(0,QIcon(":/ico/IMSmallToolbarQRing.ico"));
    B4->setIcon(0,QIcon(":/ico/IMSmallToolbarQRing.ico"));
    B5->setIcon(0,QIcon(":/ico/MailButton.ico"));
    B6->setIcon(0,QIcon(":/ico/Bitcomet.ico"));
    B7->setIcon(0,QIcon(":/ico/ChgNick.ico"));

    ui->QTree->addTopLevelItem(B1);
    ui->QTree->addTopLevelItem(B2);
    ui->QTree->addTopLevelItem(B3);
    ui->QTree->addTopLevelItem(B4);
//    ui->QTree->addTopLevelItem(B5);
//    ui->QTree->addTopLevelItem(B6);
    ui->QTree->addTopLevelItem(B7);

    QTreeWidgetItem *b1 = new QTreeWidgetItem(QStringList()<<"系统日志");
    QTreeWidgetItem *b2 = new QTreeWidgetItem(QStringList()<<"设备日志");
    QTreeWidgetItem *b3 = new QTreeWidgetItem(QStringList()<<"刷卡日志");

    b1->setIcon(0,QIcon(":/ico/傲游浏览器.ico"));
    b2->setIcon(0,QIcon(":/ico/傲游浏览器.ico"));
    b3->setIcon(0,QIcon(":/ico/傲游浏览器.ico"));

    B3->addChild(b1);
    B3->addChild(b2);
    if(m_sUser.protocol == 0)
    {
        B3->addChild(b3);   //刷卡
    }

    QTreeWidgetItem *b4 = new QTreeWidgetItem(QStringList()<<"住户设备");
    QTreeWidgetItem *b5 = new QTreeWidgetItem(QStringList()<<"中间设备");
    b4->setIcon(0,QIcon(":/ico/傲游浏览器.ico"));
    b5->setIcon(0,QIcon(":/ico/傲游浏览器.ico"));
    B1->addChild(b4);
    B1->addChild(b5);

    QTreeWidgetItem *b6 = new QTreeWidgetItem(QStringList()<<"普通信息");
    QTreeWidgetItem *b7 = new QTreeWidgetItem(QStringList()<<"物业信息");
    QTreeWidgetItem *b8 = new QTreeWidgetItem(QStringList()<<"重要新闻");

    B2->addChild(b6);
    B2->addChild(b7);
    if(m_sUser.protocol != 0)
    {
        B2->addChild(b8);
    }

    QTreeWidgetItem *b9  = new QTreeWidgetItem(QStringList()<<"报警记录");
    QTreeWidgetItem *b10 = new QTreeWidgetItem(QStringList()<<"报修记录");
    QTreeWidgetItem *b11 = new QTreeWidgetItem(QStringList()<<"呼叫记录");

    b9->setIcon(0,QIcon(":/ico/傲游浏览器.ico"));
    b10->setIcon(0,QIcon(":/ico/傲游浏览器.ico"));
    b11->setIcon(0,QIcon(":/ico/傲游浏览器.ico"));

    B4->addChild(b9);
    B4->addChild(b10);
    if(m_sUser.protocol == 0)    //A9
    {
        B4->addChild(b11);  //呼叫记录
    }

    QTreeWidgetItem *b12 = new QTreeWidgetItem(QStringList()<<"IC卡管理");
    QTreeWidgetItem *b13 = new QTreeWidgetItem(QStringList()<<"操作员管理");

    b12->setIcon(0,QIcon(":/ico/傲游浏览器.ico"));
    b13->setIcon(0,QIcon(":/ico/傲游浏览器.ico"));
    if(m_sUser.protocol == 0)
    {
        B7->addChild(b12);      //操作员管理
    }
    B7->addChild(b13);
}

void CSERVERCENTER::on_QTree_clicked(const QModelIndex &index)
{
    m_nIndex = 0;
    m_nFind = 0;
    m_nPage = 0;
    QString str = ui->QTree->currentItem()->text(0);
    if(str == "设备信息" || str == "小区短信" || str == "日志" || str == "记录" || str == "管理")
    {
        ui->QlblAllName->hide();
        ui->QlblAll->clear();
        CUdpRecv->m_nPage = 0;  //page_1
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
    }
    else if(str == "住户设备")
    {
        m_nFindDevice == 0;
        ui->QlblAllName->show();
        CUdpRecv->m_nPage = 1;  //page_2
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        ui->QchkTenement->setCurrentIndex(0);
        ui->QlblAll->setText(QString::number(CUdpRecv->m_nAllTenement));
        ui->QlblOnlineTenement->setText(QString::number(CUdpRecv->m_nAllTenementOnline));
        _UpdateTenement(CUdpRecv->m_TenementItem);
        _UpdateCallNow();
    }
    else if(str == "中间设备")
    {
        m_nFindDevice == 0;
        ui->QlblAllName->show();
        CUdpRecv->m_nPage = 2;  //page_3
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        ui->QchkMiddle->setCurrentIndex(0);
        ui->QlblAll->setText(QString::number(CUdpRecv->m_nAllMiddle));
        ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));
        _UpdateMiddle(CUdpRecv->m_MiddleItem);
    }
    else if(str == "报警记录")
    {
        ui->QlblAllName->show();
        CUdpRecv->m_nPage = 3;
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        _UpdateAlarmRecord();
    }
    else if(str == "报修记录")
    {
        ui->QlblAllName->show();
        CUdpRecv->m_nPage = 4;
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        _UpdateRepairsRecord();
    }
    else if(str == "IC卡管理")
    {
        ui->QlblAllName->hide();
        ui->QlblAll->clear();

        if(m_sUser.protocol == 0)
        {
            CUdpRecv->m_nPage = 13;    //page
            ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
            ui->IC_stackedWidget->setCurrentIndex(0);
            _SetTableWidgetSatus(1);
            ui->QpbtnAdd->setEnabled(false);
            _UpdateEmpowerIndoor(CUdpRecv->m_TenementItem);
        }
        else
        {
            CUdpRecv->m_nPage = 5;
            ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
            _ClearItem();
            _UpdateIcCard();
            _UpdateCardMessage();
        }
    }
    else if(str == "操作员管理")
    {
        ui->QlblAllName->show();
        CUdpRecv->m_nPage = 6;
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        _UpdateUser();
    }
    else if(str == "系统日志")
    {
        ui->QlblAllName->show();
        CUdpRecv->m_nPage = 7;
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        _UpdateSystem();
    }
    else if(str == "设备日志")
    {
        ui->QlblAllName->show();
        CUdpRecv->m_nPage = 8;
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        _UpdateDevice();
    }
    else if(str == "刷卡日志")
    {
        ui->QlblAllName->show();
        CUdpRecv->m_nPage = 9;
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        _UpdateCardRecord();
    }
    else if(str == "普通信息")
    {
        ui->QlblAllName->show();
        CUdpRecv->m_nPage = 10;     //page_11
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        ui->QlblAll->setText(QString::number(CUdpRecv->m_nAllTenement));
        ui->QtextMessage->clear();
        _UpdateMessage(CUdpRecv->m_TenementItem);
    }
    else if(str == "物业信息")
    {
        ui->QlblAllName->hide();
        ui->QlblAll->clear();
        CUdpRecv->m_nPage = 11;     //page_12
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        _UpdateRepairs();
        _UpdatePhone();
    }
    else if(str == "重要新闻")
    {
        ui->QlblAllName->show();
        CUdpRecv->m_nPage = 12;
        ui->QlinePath->clear();
        ui->QlblAll->setText(QString::number(CUdpRecv->m_nAllMiddle));
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        _UpdateNews(CUdpRecv->m_MiddleItem);
    }
    else if(str == "呼叫记录")
    {
        QDateTime dateTime = QDateTime::currentDateTime();
        ui->QdateETime->setDateTime(dateTime);//setDate(dateTime.date());
        dateTime = dateTime.addDays(-5);
        ui->QdateSTime->setDateTime(dateTime);//setDate(dateTime.date());
        ui->QrbtnTime->setChecked(false);
        ui->QdateETime->setEnabled(false);
        ui->QdateSTime->setEnabled(false);
        ui->QchkType->setCurrentIndex(0);
        ui->QlblAllName->show();
        CUdpRecv->m_nPage = 14;      //page_14
        ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
        _UpdateCallRecord();
    }
//    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::IconActivated(QSystemTrayIcon::ActivationReason reason)
{
    //触发后台图标执行相应事件
    switch (reason)
    {
        case QSystemTrayIcon::Trigger:
            //showMessage("鼠标单击！");
            break;
        case QSystemTrayIcon::DoubleClick:
            //showMessage("鼠标双击！");
            //qDebug()<<"双击";
            //this->showNormal();
            //this->show();
            emit ShowManager();
            break;
        case QSystemTrayIcon::MiddleClick:
            //showMessage("鼠标中键！");
            break;
        default:
            break;
    }
}


//中间设备状态由在线转为不在线，更新对应列表信息
void CSERVERCENTER::SlotOnlineMiddle(int Id)
{
    CUdpRecv->m_nAllMiddleOnline--;
    ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));
    if(CUdpRecv->m_nPage == 2)
    {
        if(m_nFindDevice == 1)
            return;
        switch(m_nIndex)
        {
            case 0:
            {
                QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
                tabledmItem5->setTextAlignment(Qt::AlignCenter);
                ui->QMiddle->setItem(Id, 5, tabledmItem5);
                for(int j = 0;j < 7;j++)
                {
                    QTableWidgetItem *item = ui->QMiddle->item(Id, j);
                    item->setTextColor(QColor(0,0,0));
                }
            }
                break;
            case 1:
                _MiddleOnline(CUdpRecv->m_MiddleItem);
                break;
            case 2:
                _MiddleNotOnline(CUdpRecv->m_MiddleItem);
                break;
            case 3:
                _MiddleM(CUdpRecv->m_MiddleItem);
                break;
            case 4:
                _MiddleW(CUdpRecv->m_MiddleItem);
                break;
            case 5:
                _MiddleZ(CUdpRecv->m_MiddleItem);
                break;
            case 6:
                _MiddleP(CUdpRecv->m_MiddleItem);
                break;
            case 7:
                _MiddleH(CUdpRecv->m_MiddleItem);
                break;
            default :
                break;
        }
    }
    else if(CUdpRecv->m_nPage == 12)
    {
        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem("未连接");
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QNews->setItem(Id, 4, tabledmItem4);
        for(int j = 0;j < 6;j++)
        {
            QTableWidgetItem *item = ui->QNews->item(Id, j);
            item->setTextColor(QColor(0,0,0));
        }
    }
    else if(CUdpRecv->m_nPage == 8)
    {
        _UpdatePage(CUdpRecv->m_nPage);
    }
}


//收到报警信号，进行相应：播放铃声，刷新对应列表
void CSERVERCENTER::SlotInsertAlarm()
{
    if(m_MediaObject->state() == Phonon::PlayingState)
        m_MediaObject->stop();
    m_nMusicId = 0;
//    qDebug()<<m_ListSources[m_nMusicId].fileName();
    m_MediaObject->setCurrentSource(m_ListSources[m_nMusicId]);
    m_MediaObject->play();
    _UpdateTenement(CUdpRecv->m_TenementItem);
    _UpdateAlarm(CUdpRecv->m_AlarmItem);
    if(CUdpRecv->m_nAlarm == 1)
    {
        QFont ft;
        ft.setPointSize(12);
        ui->QlblAlarm->setFont(ft);
        //设置颜色
        QPalette pa;
        pa.setColor(QPalette::WindowText,Qt::red);
        ui->QlblAlarm->setPalette(pa);
    }
    ui->QlblAlarm->setText(QString::number(CUdpRecv->m_nAlarm));
//    _UpdateAlarmRecord();
    _UpdatePage(CUdpRecv->m_nPage);

}
//报修响应
void CSERVERCENTER::SlotRepairsRecord()
{
    if(m_MediaObject->state() == Phonon::PlayingState)
        m_MediaObject->stop();
    m_nMusicId = 1;
    m_MediaObject->setCurrentSource(m_ListSources[m_nMusicId]);
    m_MediaObject->play();
    if(CUdpRecv->m_nPage == 4)
    {
        _UpdatePage(CUdpRecv->m_nPage);
    }
//        _UpdateRepairsRecord();
}

//显示界面
void CSERVERCENTER::SlotShowManager()
{
    this->activateWindow();
    this->showNormal();
}
//播放结束后响应
void CSERVERCENTER::SlotAboutToFinish()
{
    m_MediaObject->enqueue(m_ListSources.at(m_nMusicId));
}

void CSERVERCENTER::paintEvent(QPaintEvent *event)
{
   QPainter painter(this);
   painter.drawPixmap(0,0,width(),height(),QPixmap(":/backdrop.png"));
}
void CSERVERCENTER::_test(unsigned char* buf)
{
//    qDebug()<<buf[6];
}

//信号槽连接机制
void CSERVERCENTER::_Response()
{
    connect(m_MediaObject, SIGNAL(aboutToFinish()), this, SLOT(SlotAboutToFinish()));
    connect(this,SIGNAL(ShowManager()),this,SLOT(SlotShowManager()));
    connect(TrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(IconActivated(QSystemTrayIcon::ActivationReason)));
    connect( this, SIGNAL(Sigquit()), qApp, SLOT(quit()) );
    connect( this, SIGNAL(SigRefurbish(QString)), this, SLOT(SlotRefurbish(QString)) );

    connect(MinimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
    connect(MaximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));
    connect(RestoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    connect(TrayQuitAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeQuit()));

    connect(CLock,SIGNAL(SigUnLock()),this,SLOT(SlotUnLock()));

    connect(CUdpRecv,SIGNAL(_test(unsigned char*)),this,SLOT(_test(unsigned char*)));
    connect(CUdpRecv,SIGNAL(SigRecvDownAloneCard(int)),this,SLOT(SlotRecvDownAloneCard(int)));
    connect(CUdpRecv,SIGNAL(SigReset()),this,SLOT(SlotRecvReset()));
    connect(CUdpRecv,SIGNAL(SigUpdateWeatherNew()),this,SLOT(SlotUpdateWeatherNew()));
    connect(CUdpRecv,SIGNAL(SigDateTime(int,int)),this,SLOT(SlotDateTime(int,int)));
    connect(CUdpRecv,SIGNAL(SigRecvInitPass(QString,int)),this,SLOT(SlotRecvInitPass(QString,int)));
    connect(CUdpRecv, SIGNAL(SigUdpOpenFail()), this, SLOT(SlotUdpOpenFail()));
    connect(CUdpRecv, SIGNAL(SigOnlineMiddle(int)), this, SLOT(SlotOnlineMiddle(int)));
    connect(CUdpRecv, SIGNAL(SigInsertAlarm()), this, SLOT(SlotInsertAlarm()));
    connect(CUdpRecv, SIGNAL(SigRepairsRecord()), this, SLOT(SlotRepairsRecord()));

    connect(CUdpRecv, SIGNAL(SigInitMiddle()), this, SLOT(SlotInitMiddle()));
    connect(CUdpRecv, SIGNAL(SigUpdateMiddle(QVariant)), this, SLOT(SlotUpdateMiddle(QVariant)));

    connect(CUdpRecv, SIGNAL(SigInitTenement()), this, SLOT(SlotInitTenement()));
    connect(CUdpRecv, SIGNAL(SigUpdateTenement(QVariant)), this, SLOT(SlotUpdateTenement(QVariant)));
    connect(CUdpRecv, SIGNAL(SigOnlineTenement(int,int)), this, SLOT(SlotOnlineTenement(int,int)));
    connect(CUdpRecv, SIGNAL(SigSendMessage(int,int)), this, SLOT(SlotSendMessage(int,int)));

    connect(CUdpRecv, SIGNAL(SigUpdateMac()), this, SLOT(SlotUpdateMac()));
    connect(CUdpRecv, SIGNAL(SigUpdateAddr()), this, SLOT(SlotUpdateAddr()));

    connect(CUdpRecv, SIGNAL(Sigbtn(int)), this, SLOT(Slotbtn(int)));
    connect(CUdpRecv, SIGNAL(SigNewsDonw(int,int,int)), this, SLOT(SlotNewsDonw(int,int,int)));
    connect(CUdpRecv,SIGNAL(SigRecvLogOutCard(unsigned char *,QString)),this,SLOT(SlotRecvLogOutCard(unsigned char *,QString)));

    connect(ui->QCardMessage, SIGNAL(cellChanged(int,int)), this, SLOT(SlotCheck(int, int)));
    connect(ui->QMessage, SIGNAL(cellChanged(int,int)), this, SLOT(SlotCheck(int, int)));
    connect(ui->QNews, SIGNAL(cellChanged(int,int)), this, SLOT(SlotCheck(int, int)));
    connect(NextPageAction, SIGNAL(triggered()), this, SLOT(SlotNextPage()));           //下一页
    connect(PreviousPageAction, SIGNAL(triggered()), this, SLOT(SlotPreviousPage()));   //上一页
    connect(ICAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeIC()));
    connect(WeatherAction, SIGNAL(triggered()), this, SLOT(SlotUpdateWeather()));
    connect(DBSetAction, SIGNAL(triggered()), this, SLOT(SlotUpdateDBSet()));
    connect(TenementAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeTenement()));
    connect(MiddleAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeMiddle()));
    connect(QuitAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeQuit()));
    connect(MessageAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeMessage()));
    connect(RepairsmessageAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeRepairsmessage()));
    connect(NewsAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeNews()));
    connect(RepairsAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeRepairs()));
    connect(AlarmAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeAlarm()));
    connect(SystemAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeSystem()));
    connect(DeviceAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeDevice()));
    connect(CardAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeCard()));
    connect(CallRecordAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeCallRecord()));
    connect(ManageAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeManage()));
    connect(AboutAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeAbout()));
    connect(HelpAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeHelp()));
    connect(LockAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeLock()));
    connect(LogoutAction, SIGNAL(triggered()), this, SLOT(SlotUpdateTreeLogout()));

    connect(CSmartControl,SIGNAL(SigSendLamp(char*,QString)),this,SLOT(SlotSendLamp(char*,QString)));
    connect(CUpdate, SIGNAL(SigRead(char*,QString)), this, SLOT(SlotRead(char*,QString)));
    connect(CUpdate, SIGNAL(SigWrite(char*,QString)), this, SLOT(SlotWrite(char*,QString)));
    connect(CPhone, SIGNAL(SigPhone()), this, SLOT(SlotPhone()));
    connect(CRepairs, SIGNAL(SigRepairs()), this, SLOT(SlotRepairs()));
    connect(CDealAlarm, SIGNAL(SigDealAlarm(int,QString,QString,QString,QString)), this, SLOT(SlotDealAlarm(int,QString,QString,QString,QString)));
    connect(CUser,SIGNAL(SigUpdateUser()),this,SLOT(SlotUpdateUser()));
    connect(CDealRepairs,SIGNAL(SigDealRepairs()),this,SLOT(SlotDealRepairs()));
    connect(CDelete,SIGNAL(SigDelete()),this,SLOT(SlotDelete()));
    connect(CFind,SIGNAL(SigFind(int,int,QString,QString,QString)),this,SLOT(SlotFind(int,int,QString,QString,QString)));
    connect(CFindDevice,SIGNAL(SigDeviceFind(int,QString)),this,SLOT(SlotMiddleFind(int,QString)));
    connect(CFindCard,SIGNAL(SigFindCard(int,QString,QString,QString,QString)),this,SLOT(SlotFindCard(int,QString,QString,QString,QString)));
    connect(CCard,SIGNAL(SigUpdateTime(QString)),this,SLOT(SlotUpdateTime(QString)));
    connect(CCard,SIGNAL(SigRefurbish(QString)),this,SLOT(SlotRefurbish(QString)));

    connect(SmartControlAction, SIGNAL(triggered()), this, SLOT(SlotSmartControl()));
    connect(PingIpAction, SIGNAL(triggered()), this, SLOT(SlotPingIp()));
    connect(UpdateAction, SIGNAL(triggered()), this, SLOT(SlotUpdate()));
    connect(DeleteAction, SIGNAL(triggered()), this, SLOT(SlotDeleteDevice()));
    connect(FindAction, SIGNAL(triggered()), this, SLOT(SlotFindMiddle()));
    connect(ResetAction, SIGNAL(triggered()), this, SLOT(SlotReset()));

    connect(RepairsUpdateAction, SIGNAL(triggered()), this, SLOT(SlotUpdateRepairs()));
    connect(RepairsDeleteAction, SIGNAL(triggered()), this, SLOT(SlotDeleteRepairs()));
    connect(RepairsAddAction, SIGNAL(triggered()), this, SLOT(SlotAddRepairs()));
    connect(PhoneUpdateAction, SIGNAL(triggered()), this, SLOT(SlotUpdatePhone()));
    connect(PhoneDeleteAction, SIGNAL(triggered()), this, SLOT(SlotDeletePhone()));
    connect(PhoneAddAction, SIGNAL(triggered()), this, SLOT(SlotAddPhone()));
    connect(PhoneDownAction, SIGNAL(triggered()), this, SLOT(SlotDownPhone()));

    //IC卡暂时不做
    connect(CardMessageSaveAction, SIGNAL(triggered()), this, SLOT(SlotCardMessageSave()));
    connect(IcCarUpdateAction, SIGNAL(triggered()), this, SLOT(SlotIcCarUpdate()));
    connect(IcCarDeleteAction, SIGNAL(triggered()), this, SLOT(SlotIcCarDelete()));
    connect(IcCarAddAction, SIGNAL(triggered()), this, SLOT(SlotIcCarAdd()));
    connect(IcCarFindAction, SIGNAL(triggered()), this, SLOT(SlotIcCarFind()));
    connect(CardMessageUpAction, SIGNAL(triggered()), this, SLOT(SlotUpCard()));
    connect(CardMessageDownAllAction, SIGNAL(triggered()), this, SLOT(SlotDownAllCard()));

    connect(CardMessageDownAction, SIGNAL(triggered()), this, SLOT(SlotDownCard()));
    connect(CardLogOutAction, SIGNAL(triggered()), this, SLOT(SlotLogOutCard()));

    connect(UserUpdateAction, SIGNAL(triggered()), this, SLOT(SlotUserUpdate()));
    connect(UserDeleteAction, SIGNAL(triggered()), this, SLOT(SlotUserDelete()));
    connect(UserAddAction, SIGNAL(triggered()), this, SLOT(SlotUserAdd()));

    connect(DealAllAction, SIGNAL(triggered()), this, SLOT(SlotDealAll()));
    connect(RepairsRecordFindAction, SIGNAL(triggered()), this, SLOT(SlotRepairsRecordFind()));
    connect(RepairsRecordDeleteAction, SIGNAL(triggered()), this, SLOT(SlotRepairsRecordDelete()));
    connect(RepairsRecordDealAction, SIGNAL(triggered()), this, SLOT(SlotRepairsRecordDeal()));
    connect(AlarmRecordFindAction, SIGNAL(triggered()), this, SLOT(SlotAlarmRecordFind()));
    connect(AlarmRecordDeleteAction, SIGNAL(triggered()), this, SLOT(SlotAlarmRecordDelete()));
    connect(AlarmDealAction, SIGNAL(triggered()), this, SLOT(SlotAlarmDeal()));
    connect(DealAction,SIGNAL(triggered()), this, SLOT(SlotDeal()));
    connect(StopAction,SIGNAL(triggered()), this, SLOT(SlotStop()));

    connect(CardRecordFindAction, SIGNAL(triggered()), this, SLOT(SlotCardRecordFind()));
    connect(CardRecordDeleteAction, SIGNAL(triggered()), this, SLOT(SlotCardRecordDelete()));
    connect(CallRecordDeleteAction, SIGNAL(triggered()), this, SLOT(SlotCallRecordDelete()));
    connect(DeviceFindAction, SIGNAL(triggered()), this, SLOT(SlotDeviceFind()));
    connect(DeviceDeleteAction, SIGNAL(triggered()), this, SLOT(SlotDeviceDelete()));
    connect(SystemFindAction, SIGNAL(triggered()), this, SLOT(SlotSystemFind()));
    connect(SystemDeleteAction, SIGNAL(triggered()), this, SLOT(SlotSystemDelete()));

    connect(DeleteCardAction,SIGNAL(triggered()),this,SLOT(SlotDeleteCard()));
    connect(SaveCardAction,SIGNAL(triggered()),this,SLOT(SlotSaveUpCard()));
    connect(SaveAllCardAction, SIGNAL(triggered()),this, SLOT(SlotSaveAllUpCard()));
    connect(ClearAllCardAction, SIGNAL(triggered()), this, SLOT(SlotClearAllCard()));

    if(m_sUser.protocol == 0)       //A8
    {
        connect(InitSystemPassAction, SIGNAL(triggered()), this, SLOT(SlotInitSystemPassA8()));//初始化管理密码
        connect(InitLockPassAction, SIGNAL(triggered()), this, SLOT(SlotInitLockPassA8()));   //初始化开锁密码
        connect(RepairsDownAction, SIGNAL(triggered()), this, SLOT(SlotDownRepairsA8()));
        connect(CUdpRecv, SIGNAL(SigCallSuccess()), this, SLOT(SlotUpdateCallRecord()));
        //A8---写IC卡
        connect(CUdpRecv, SIGNAL(SigRecvDownCardA8(unsigned char*,QString)),this,SLOT(SlotRecvDownCard_A8(unsigned char*,QString)));
        connect(CUdpRecv, SIGNAL(SigRecvUpCard_A8(unsigned char*)), this, SLOT(SlotRecvUpCard_A8(unsigned char*)));
        connect(CUdpRecv, SIGNAL(SigRecvRead_A8(unsigned char *,QString)), this, SLOT(SlotRecvRead_A8(unsigned char *,QString)));
    }
    else
    {
        connect(InitSystemPassAction, SIGNAL(triggered()), this, SLOT(SlotInitSystemPass()));//初始化管理密码
        connect(InitLockPassAction, SIGNAL(triggered()), this, SLOT(SlotInitLockPass()));   //初始化开锁密码
        connect(RepairsDownAction, SIGNAL(triggered()), this, SLOT(SlotDownRepairs()));

        connect(CUdpRecv,SIGNAL(SigRecvDownCard(unsigned char *,QString)),this,SLOT(SlotRecvDownCard(unsigned char *,QString)));
        connect(CUdpRecv,SIGNAL(SigRecvUpCard(unsigned char *)),this,SLOT(SlotRecvUpCard(unsigned char *)));
        connect(CUdpRecv,SIGNAL(SigRecvDownAllCard(unsigned char *,QString)),this,SLOT(SlotRecvDownAllCard(unsigned char *,QString)));//A9
        connect(CUdpRecv, SIGNAL(SigRecvRead(unsigned char*,QString)), this, SLOT(SlotRecvRead(unsigned char*,QString)));
    }
}

//全部处理所以报警或报修
void CSERVERCENTER::SlotDealAll()
{
    if(m_MediaObject->state() == Phonon::PlayingState)
        m_MediaObject->stop();
    QString str;
    switch(CUdpRecv->m_nPage)
    {
        case 3://报警
        {
            str = "确定以管理员身份处理所有报警为误报";
            switch( QMessageBox::question( this, tr("提示"),str,tr("Yes"), tr("No"),0, 1 ) )
            {
                case 0:
                {
                    CUdpRecv->m_Mysql._DealAll(1);
                    CUdpRecv->m_nAlarm = 0  ;
                    QFont ft;
                    ft.setPointSize(10);
                    ui->QlblAlarm->setFont(ft);
                    //设置颜色
                    QPalette pa;
                    pa.setColor(QPalette::WindowText,Qt::black);
                    ui->QlblAlarm->setPalette(pa);
                    ui->QlblAlarm->setText(QString::number(CUdpRecv->m_nAlarm));
                    CUdpRecv->_ClearAlarm();
//                    _UpdateAlarmRecord();
                    _UpdateTenementToAlarm();
                    _UpdateAlarm(CUdpRecv->m_AlarmItem);
                }
                    break;
                case 1:
                    break;
            }
        }
            break;
        case 4:
        {
            str = "确定以管理员身份处理所有报修为误报";
            switch( QMessageBox::question( this, tr("提示"),str,tr("Yes"), tr("No"),0, 1 ) )
            {
                case 0:
                    CUdpRecv->m_Mysql._DealAll(2);
//                    _UpdateRepairsRecord();
                    break;
                case 1:
                  break;
            }
        }
            break;
        default :
            break;
    }
    _UpdatePage(CUdpRecv->m_nPage);
}
//智能家居控制
void CSERVERCENTER::SlotSmartControl()
{
    int i = ui->QTenement->currentRow();
    //QTableWidgetItem * item = ui->QTenement->currentItem();
    if(i >= 0)
    {
    QString strIp = ui->QTenement->item(i, 7)->text();
    QString strAddr = ui->QTenement->item(i, 1)->text();
    QString strType = ui->QTenement->item(i, 11)->text();
    CSmartControl->_Updata(strIp,strAddr,strType);
    CSmartControl->show();
    }else{
        QMessageBox::information(this,tr("提示"),tr("请选中目标"));
    }
}

//响应pingIP
void CSERVERCENTER::SlotPingIp()
{
    QString strIp;
    switch(CUdpRecv->m_nPage)
    {
        case 1:
        {
            int i = ui->QTenement->currentRow();
            if(i >= 0)
            {
                strIp = ui->QTenement->item(i, 7)->text();
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
            break;
        case 2:
        {

            int i = ui->QMiddle->currentRow();
            if(i >= 0)
            {
                strIp = ui->QMiddle->item(i, 4)->text();
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
            break;
        default :
            break;
    }
    QStringList args;
    QString str = "ping " + strIp;
    args << "/K" << str;
    QProcess::startDetached("cmd.exe",args);
}
//响应查找IC卡查找界面确定的查找条件，并进行组合处理
void CSERVERCENTER::SlotFindCard(int nFlag,QString strCard,QString strTAddr,QString strMAddr,QString strTime)
{
    int i = 0;
    QString SQL = "SELECT IC_Card_num,IC_Card_addr,IC_Card_time from ic_card where ";
    if(nFlag&1)
    {
        i++;
        SQL = SQL + "IC_Card_num LIKE '%"+strCard+"%' ";
    }
    if(nFlag&2)
    {
        if(i == 0){
            SQL = SQL + "IC_Card_addr LIKE '%"+strTAddr+"%' ";
        }else
            SQL = SQL + "and IC_Card_addr LIKE '%"+strTAddr+"%' ";
        i++;
    }

    if(nFlag&4)
    {
        if(i == 0){
        SQL = SQL + strMAddr+" = 1 ";
        }else
            SQL = SQL + "and "+strMAddr+" = 1 ";
        i++;
    }
    if(nFlag&8)
    {
        if(i == 0){
            SQL = SQL + "IC_Card_time <= '" + strTime + "'";
        }else
            SQL = SQL + "and IC_Card_time <= '" + strTime + "'";

    }
//    qDebug()<<SQL;
    SQL = SQL + "ORDER BY IC_Card_time DESC";
//    QSqlQuery query;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    m_strFindIcCard = SQL;
    if(query.exec(SQL))
    {
        int ti = 0;
        ui->QIcCard->setRowCount(ti);
        while(query.next())
        {
            ui->QIcCard->setRowCount(ti + 1);
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 0, tabledmItem0);
            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(query.value(0).toByteArray().data()));
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 1, tabledmItem1);
            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 2, tabledmItem2);
            QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(2).toDate().toString("yyyy-MM-dd")));
            tabledmItem3->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 3, tabledmItem3);
            ti++;
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//IC卡查找动作响应，显示对应的查找界面
void CSERVERCENTER::SlotIcCarFind()
{
    CFindCard->_Update();
    CFindCard->show();
}
//响应删除IC卡
void CSERVERCENTER::SlotIcCarDelete()
{
    int ti = ui->QIcCard->currentRow();
    int nAllIn = ui->QIcCard->rowCount();
    m_ListSelected.clear();
    int nSize= 0;
    if(_GetSelectedRow(&m_ListSelected,1))
    {
        nSize = m_ListSelected.size();
        if(m_ListSelected.size() == 1)
        {
            QString str1 = ui->QIcCard->item(ti, 1)->text();
            switch( QMessageBox::question( this, tr("提示"),"确定要删除门禁卡:"+str1,tr("Yes"), tr("No"),0, 1 ) )
            {
                case 0:
                    if(CUdpRecv->m_Mysql._DeleteIcCard(str1))
                    {
                        QMessageBox::information( this, tr("提示"),tr("删除成功"));
                        if(nAllIn != nSize)
                            emit SigRefurbish("");
                        else ui->QIcCard->setRowCount(0);
                    }
                    break;
                default :
                    break;
            }
        }
        else
        {
            QString strMessage = "确定要删除选中"+QString::number(nSize)+"张门禁卡";
            switch( QMessageBox::question( this, tr("提示"),strMessage,tr("Yes"), tr("No"),0, 1 ) )
            {
                case 0:
                {
                    int nSuccess = 0;
                    for(int i = 0;i < m_ListSelected.size();i++)
                    {
                        ti = m_ListSelected.at(i);
                        if(CUdpRecv->m_Mysql._DeleteIcCard(ui->QIcCard->item(ti, 1)->text()))
                        {
                            nSuccess ++;
        //                    qDebug()<<nSuccess;
                        }
                    }
                    strMessage.clear();
                    strMessage = "成功删除"+QString::number(nSuccess)+"张";
                    QMessageBox::information( this, tr("提示"),strMessage);
    //                qDebug()<<nAllIn<<nSuccess;
                    if(nAllIn != nSuccess)
                        emit SigRefurbish("");
                    else
                        ui->QIcCard->setRowCount(0);
                }
                    break;
                default :
                    break;
            }
        }

    }
    else
    {
        QMessageBox::information(this,tr("提示"),tr("请选中目标"));
    }
}
//响应IC卡更新动作，显示修改界面
void CSERVERCENTER::SlotIcCarUpdate()
{
    int nRow = ui->QIcCard->currentRow();
    if(nRow >= 0)
    {
        QString strCard = ui->QIcCard->item(nRow,1)->text();
        QString strAddr = ui->QIcCard->item(nRow,2)->text();
        QString strTime = ui->QIcCard->item(nRow,3)->text();
        CCard->_Update(2,strCard,strAddr,strTime);
        CCard->show();
    }
}
//响应IC卡添加动作，显示添加界面
void CSERVERCENTER::SlotIcCarAdd()
{
    CCard->_Update(1,"","","");
     CCard->show();
}
//
void CSERVERCENTER::SlotRefurbish(QString strCardNum)
{
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    int nCardId = 0;
    if(query.exec(m_strFindIcCard))
    {
        int ti = 0;
        ui->QIcCard->setRowCount(ti);
        while(query.next())
        {
            ui->QIcCard->setRowCount(ti + 1);
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 0, tabledmItem0);
            QString strCard = QString(query.value(0).toByteArray().data());
            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(strCard);
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 1, tabledmItem1);
            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(query.value(1).toByteArray().data()));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 2, tabledmItem2);
            QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(query.value(2).toDate().toString("yyyy-MM-dd")));
            tabledmItem3->setTextAlignment(Qt::AlignCenter);
            ui->QIcCard->setItem(ti, 3, tabledmItem3);
            if(strCard == strCardNum)
            {
                nCardId = ti;
            }
            ti++;
        }
        ui->QIcCard->setCurrentCell(nCardId, QItemSelectionModel::Select);
        _CheckCardMessage();
    }

    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

//2015 03 26
//修改IC卡有效期后的响应
void CSERVERCENTER::SlotUpdateTime(QString strCardNum)
{
    NodeMiddle *q = CUdpRecv->m_MiddleItem;
    while(q->next)
    {
        if(CUdpRecv->m_Mysql._Select(strCardNum,q->data.gcIntraAddr))
            q->data.nCardState = 1;
        q = q->next;
    }
//    CUdpRecv->_SendUpCard();
//    m_Timer->start(3*60*1000);
}

void CSERVERCENTER::SlotRecvReset()
{
QMessageBox::information(this,tr("提示"),tr("重启成功"));
}
//发送IC卡
void CSERVERCENTER::_SendCard()
{
    m_Timer->stop();
    CUdpRecv->_SendDownCard();
}

//上传IC卡响应A8
void CSERVERCENTER::SlotUpCard_A8()
{
    if(ui->Issued_Machine->currentItem()->text(0)!="中间设备")
    {
        QString strAddr = ui->Issued_Machine->currentItem()->text(0);
        QString strType = "门口机";
        QString strIntraAddr = "";
        QByteArray byte;
        bool Upload;
        strIntraAddr = _AddrExplainToAddrA8(strAddr,strType,1);
        byte = strIntraAddr.toUtf8();
        Upload = CUdpRecv->_UpCard_A8(byte);
        if(!Upload)
        {
            QMessageBox::information(this,tr("提示"),tr("上传失败"));
        }
        else
        {
            CUdpRecv->m_CardMap.clear();
            CUdpRecv->m_nPack = 0;
        }
    }
    else
        QMessageBox::information(this,"错误","请选择到门口机设备号");
}

//上传IC卡响应a9
void CSERVERCENTER::SlotUpCard()
{
    int nRow = ui->QCardMessage->currentRow();
    QString strAddr = ui->QCardMessage->item(nRow,1)->text();
    QString strType = ui->QCardMessage->item(nRow,2)->text();
    QString strIntraAddr = _AddrExplainToAddr(strAddr,strType,1);
//    qDebug()<<strIntraAddr;
    QByteArray byte;
    byte = strIntraAddr.toUtf8();
    if(CUdpRecv->_UpCard(byte))
    {
        QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("开始上传");
        tabledmItem8->setTextAlignment(Qt::AlignCenter);
        ui->QCardMessage->setItem(nRow, 3, tabledmItem8);
    }else
        QMessageBox::information(this,tr("提示"),tr("上传失败"));
}

//接收的上传的IC卡响应
void CSERVERCENTER::SlotRecvUpCard(unsigned char *buf)
{
    if(buf[31] == 1)
    {
//    ui->QIcCard->clear();
        m_nAllUpCard = buf[28] + buf[29]*256;
    }
    int nAllUpCard = buf[28] + buf[29]*256;
    if(nAllUpCard >0)
    {
        if(m_nAllUpCard == nAllUpCard)
        {

            int nCard = buf[32] + buf[33]*256;
            char pAddr[20] = {0};
            char pTime[20] = {0};
            char pAddrT[64] = {0};
            memcpy(pAddr,(char *)buf+8,20);
            int nRecvCards = (buf[31]-1)*CARD_MAXLEN;
            QString strResult = "上传总数:"+QString::number(m_nAllUpCard)+"成功上传数:"+QString::number(nRecvCards + nCard);
            int nId = ui->QCardMessage->currentRow();
            QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(strResult);
            tabledmItem3->setTextAlignment(Qt::AlignCenter);
            ui->QCardMessage->setItem(nId, 3, tabledmItem3);

            for(int i = 0;i < nCard;i++)
            {
                QString str2;
                str2.clear();
                for(int j = 0;j<5;j++)
                {
                    int kk = buf[34+j+i*5];
                    if(kk < 16) str2 = str2 + "0";
                    str2 = str2 + QString::number(kk, 16).toUpper();
                    if(j != 4) str2 = str2 + ":";
                }
                CUdpRecv->m_Mysql._RecvCard(str2,pAddr,pTime,pAddrT);
                int ti = nRecvCards +i;
                ui->QIcCard->setRowCount(ti + 1);
                QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
                tabledmItem0->setTextAlignment(Qt::AlignCenter);
                ui->QIcCard->setItem(ti, 0, tabledmItem0);
                QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(str2);
                tabledmItem1->setTextAlignment(Qt::AlignCenter);
                ui->QIcCard->setItem(ti, 1, tabledmItem1);
                QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(pAddrT));
                tabledmItem2->setTextAlignment(Qt::AlignCenter);
                ui->QIcCard->setItem(ti, 2, tabledmItem2);
                QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(pTime));
                tabledmItem3->setTextAlignment(Qt::AlignCenter);
                ui->QIcCard->setItem(ti, 3, tabledmItem3);
            }
        }
    }
    else
    {
        int nId = ui->QCardMessage->currentRow();
        QString strResult = "该设备里面没有IC卡";
        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(strResult);
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QCardMessage->setItem(nId, 3, tabledmItem3);
    }
}

void CSERVERCENTER::SlotRecvUpCard_A8(unsigned char *buf)
{
    if(CUdpRecv->m_CardMap.size() != 0)
    {
        QMap<QString,QString>::iterator tt;
        tt = CUdpRecv->m_CardMap.begin();
        int ti;
        for(ti = 0 ;ti < CUdpRecv->m_CardMap.size(); ti++)
        {

            ui->Upload_IC->setRowCount(ti+1);
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->Upload_IC->setItem(ti, 0, tabledmItem0);
            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(tt.key()));
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->Upload_IC->setItem(ti, 1, tabledmItem1);
            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(_AddrToAddrExplainA8(QString(tt.value()),"室内机"));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->Upload_IC->setItem(ti, 2, tabledmItem2);
            qDebug()<<QString(tt.key())<<QString(tt.value());
            tt++;
        }
        ui->AllUpCard->setText(QString::number(ti+1));
        QMessageBox::information(this,tr("提示"),tr("上传成功"));
    }
    else
    {
        int nId = ui->QCardMessage->currentRow();
        QString strResult = "该设备里面没有IC卡";
        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(strResult);
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QCardMessage->setItem(nId, 3, tabledmItem3);
    }
}

void CSERVERCENTER::SlotRecvLogOutCard(unsigned char *buf,QString strIp)
{
    char pAddr[20] = {0};
    memcpy(pAddr,(char *)buf+8,20);
    QString strMiddle = QString(pAddr);
    int nAllCard = buf[28] + buf[29]*256;
    int nThisCard = buf[32] + buf[33]*256;
    int nSendCard = CARD_MAXLEN*(buf[31]-1) + nThisCard;
//    qDebug()<<nSendCard<<"接收到card"<<buf[31]<<buf[32];
    QString strResult = "注销总数:"+QString::number(nAllCard)+"成功注销数:"+QString::number(nSendCard);
    int nId = ui->QCardMessage->currentRow();
    QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(strResult);
    tabledmItem3->setTextAlignment(Qt::AlignCenter);
    ui->QCardMessage->setItem(nId, 3, tabledmItem3);
//    if(nAllCard == CUdpRecv->m_Mysql._SelectAllCard(strMiddle))

    for(int j = 0;j < nThisCard;j++)
    {
    QString strCardNum;
    for(int i = 0;i<5;i++)
    {
        int k = buf[34+i+j*5];
        if(k < 16) strCardNum = strCardNum + "0";
        strCardNum = strCardNum + QString::number(k, 16).toUpper();
        if(i != 4) strCardNum = strCardNum + ":";
    }
    CUdpRecv->m_Mysql._ChangeCardPermissions(strMiddle,strCardNum,0);
    }

    if(nAllCard == m_ListSelected.size())
    {
        if(nAllCard != nSendCard)
        {
            int nPack = nSendCard/CARD_MAXLEN + 1;//发送了的包 + 1即包序
            int nAllPack = 0;//总报数
            if(nAllCard%CARD_MAXLEN)
            {
                nAllPack = nAllCard/CARD_MAXLEN + 1;
            }else nAllPack = nAllCard/CARD_MAXLEN;
            char buf[CARD_MAXLEN*5] = {0};
            QString str,str1;
            int i = 0;//定位开始位置
            int k = 0;//定位结束位置
            if(nPack == nAllPack)
            {
                i = (nPack-1)*CARD_MAXLEN;
                k = nAllCard;
            }else{
                i = (nPack-1)*CARD_MAXLEN;
                k = nPack*CARD_MAXLEN;
            }
            int l = 0;//定位buf
            int nThisSendCard = k - i;
            for(;i < k;i++)
            {
                str.clear();
                str1.clear();
                str = ui->QIcCard->item(m_ListSelected.at(i),1)->text();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                bool ok;
                buf[0+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[1+l*5] = (str1.left(1).toInt())*16 + str1.right(1).toInt();
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[2+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[3+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[4+l*5] = str1.toInt(&ok,16);
                l++;
            }
            CUdpRecv->_LogOutCard(m_Byte,buf,nAllCard,nThisSendCard,nPack);
        }
    }
}

void CSERVERCENTER::SlotRecvDownAllCard(unsigned char *buf,QString strIp)
{
    char pAddr[20] = {0};
    memcpy(pAddr,(char *)buf+8,20);
    QString strMiddle = QString(pAddr);
    int nAllCard = buf[28] + buf[29]*256;
    int nSendCard = CARD_MAXLEN*(buf[31]-1) + buf[32] + buf[33]*256;
//    qDebug()<<nSendCard<<"card"<<buf[31]<<buf[32];
//    QString strResult = "下载总数:"+QString::number(nAllCard)+"成功下载数:"+QString::number(nSendCard);
//    int nId = ui->QCardMessage->currentRow();
//    QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(strResult);
//    tabledmItem3->setTextAlignment(Qt::AlignCenter);
//    ui->QCardMessage->setItem(nId, 3, tabledmItem3);
    if(nAllCard == CUdpRecv->m_Mysql._SelectAllCard(strMiddle))
    {
        if(nAllCard != nSendCard)
        {
            int nPack = nSendCard/CARD_MAXLEN + 1;//发送了的包 + 1即包序
            int nAllPack = 0;//总报数
            if(nAllCard%CARD_MAXLEN)
            {
                nAllPack = nAllCard/CARD_MAXLEN + 1;
            }else nAllPack = nAllCard/CARD_MAXLEN;

//            char *buf = new char[CARD_MAXLEN*5];
//            memset(buf,0,CARD_MAXLEN*5);
            char buf[CARD_MAXLEN*5] = {0};
            QString str,str1;
            int i = 0;//定位开始位置
            int k = 0;//定位结束位置
            if(nPack == nAllPack)
            {
                i = (nPack-1)*CARD_MAXLEN;
                k = nAllCard;
            }else{
                i = (nPack-1)*CARD_MAXLEN;
                k = nPack*CARD_MAXLEN;
            }
            int l = 0;//定位buf
            int nThisSendCard = k - i;

            QDateTime dateTime=QDateTime::currentDateTime();
            QString strDtime = dateTime.toString("yyyy-MM-dd");
            QString SQL = "select IC_Card_num from ic_card where IC_Card_time > '" + strDtime + "' and " + strMiddle + "= 1 ";
//            QSqlQuery query;
            QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
            QSqlQuery query(db);
            query.exec(SQL);
            int t = 0;
            while(query.next())
            {
            if(t>=i&&t<k)
            {

                str.clear();
                str1.clear();
                str = QString(query.value(0).toByteArray().data());
                str1 = str.left(2);
                str = str.right(str.length()-3);
                bool ok;
                buf[0+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[1+l*5] = (str1.left(1).toInt())*16 + str1.right(1).toInt();
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[2+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[3+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[4+l*5] = str1.toInt(&ok,16);
                l++;
            }
            t++;
            }
            CCONNECTIONPOOLMYSQL::closeConnection(db);
            CUdpRecv->_DownAllCard(m_Byte,buf,nAllCard,nThisSendCard,nPack);
        }else{
            char pAddr[20] = {0};
            memcpy(pAddr,(char *)buf+8,20);
            CLIST::ItemMiddleUpdateCardToUp(pAddr,CUdpRecv->m_MiddleItem,2);
        }
    }
}
void CSERVERCENTER::SlotRecvDownCard(unsigned char *buf,QString strIp)
{
    char pAddr[20] = {0};
    memcpy(pAddr,(char *)buf+8,20);
    QString strMiddle = QString(pAddr);
    int nAllCard = buf[28] + buf[29]*256;
    int nThisCard = buf[32] + buf[33]*256;
    int nSendCard = CARD_MAXLEN*(buf[31]-1) + nThisCard;
//    qDebug()<<nSendCard<<"接收到card"<<buf[31]<<buf[32];
    QString strResult = "注册总数:"+QString::number(nAllCard)+"成功注册数:"+QString::number(nSendCard);
    int nId = ui->QCardMessage->currentRow();
    QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(strResult);
    tabledmItem3->setTextAlignment(Qt::AlignCenter);
    ui->QCardMessage->setItem(nId, 3, tabledmItem3);
    for(int j = 0;j < nThisCard;j++)
    {
        QString strCardNum ;
        for(int i = 0;i<5;i++)
        {
            int k = buf[34+i+j*5];
            if(k < 16)
                strCardNum = strCardNum + "0";
            strCardNum = strCardNum + QString::number(k, 16).toUpper();
            if(i != 4)
                strCardNum = strCardNum + ":";
        }
        CUdpRecv->m_Mysql._ChangeCardPermissions(strMiddle,strCardNum,1);
    }
//    if(nAllCard == CUdpRecv->m_Mysql._SelectAllCard(strMiddle))
    if(nAllCard == m_ListSelected.size())
    {
        if(nAllCard != nSendCard)
        {
            int nPack = nSendCard/CARD_MAXLEN + 1;//发送了的包 + 1即包序
            int nAllPack = 0;//总报数
            if(nAllCard%CARD_MAXLEN)
            {
                nAllPack = nAllCard/CARD_MAXLEN + 1;
            }
            else
                nAllPack = nAllCard/CARD_MAXLEN;
            char buf[CARD_MAXLEN*5] = {0};
            QString str,str1;
            int i = 0;//定位开始位置
            int k = 0;//定位结束位置
            if(nPack == nAllPack)
            {
                i = (nPack-1)*CARD_MAXLEN;
                k = nAllCard;
            }
            else
            {
                i = (nPack-1)*CARD_MAXLEN;
                k = nPack*CARD_MAXLEN;
            }
            int l = 0;//定位buf
            int nThisSendCard = k - i;
            for(;i < k;i++)
            {
                str.clear();
                str1.clear();
                str = ui->QIcCard->item(m_ListSelected.at(i),1)->text();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                bool ok;
                buf[0+l*5] = str1.toInt(&ok,16);

                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[1+l*5] = (str1.left(1).toInt())*16 + str1.right(1).toInt();

                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[2+l*5] = str1.toInt(&ok,16);

                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[3+l*5] = str1.toInt(&ok,16);

                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[4+l*5] = str1.toInt(&ok,16);
                l++;
            }
            CUdpRecv->_DownCard(m_Byte,buf,nAllCard,nThisSendCard,nPack);
        }
    }

/*
    if(nAllCard == ui->QIcCard->rowCount())
    {//总数是否相同
        if(nAllCard != nSendCard)
        {
            int nPack = nSendCard/CARD_MAXLEN + 1;//发送了的包 + 1即包序
            int nAllPack = 0;//总报数
            if(nAllCard%CARD_MAXLEN)
            {
                nAllPack = nAllCard/CARD_MAXLEN + 1;
            }else nAllPack = nAllCard/CARD_MAXLEN;

            char *buf = new char[CARD_MAXLEN*5];
            memset(buf,0,CARD_MAXLEN*5);
            QString str,str1;
            int i = 0;//定位开始位置
            int k = 0;//定位结束位置
            if(nPack == nAllPack)
            {
                i = (nPack-1)*CARD_MAXLEN;
                k = nAllCard;
            }else{
                i = (nPack-1)*CARD_MAXLEN;
                k = nPack*CARD_MAXLEN;
            }
            int l = 0;//定位buf
            int nThisSendCard = k - i;
            for(i;i < k;i++)
            {

                str.clear();
                str1.clear();
                str = ui->QIcCard->item(i, 1)->text();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                bool ok;
                buf[0+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[1+l*5] = (str1.left(1).toInt())*16 + str1.right(1).toInt();
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[2+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[3+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[4+l*5] = str1.toInt(&ok,16);
                l++;
            }
            CUdpRecv->_DownCard(m_Byte,buf,nAllCard,nThisSendCard,nPack);
        }
    }*/
}
//A8写IC卡
void CSERVERCENTER::SlotRecvDownCard_A8(unsigned char *buf, QString strIp)
{
//    qDebug()<<"界面还没实现";
    //一个包最大定额卡数：50
    char pAddr[20] = {0};       //地址
    memcpy(pAddr,(char *)buf+8,20);
    QString strMiddle = QString(pAddr);
    int nAllCard = buf[32] + buf[33]*256 +(buf[34]<<8)*256 + (buf[35]<<16)*256;//卡总数
    int nThisCard = buf[36] + buf[37]*256 +(buf[38]<<8)*256 + (buf[39]<<16)*256;//当前包卡数
    int nSendCard = CARD_MAXLEN*(buf[44] + buf[45]*256 +(buf[46]<<8)*256 + (buf[47]<<16)*256-1) + nThisCard;//当前注册的卡序
    ui->AllDownCard->setText(QString::number(nSendCard));
    if(nAllCard == nSendCard)
    {
        QMessageBox::information(this,tr("提示"),tr("下载成功"));
        return;
    }
    unsigned int nFlag = buf[28] + buf[29]*256 +(buf[30]<<8)*256 + (buf[31]<<16)*256;   //序号
    if(nAllCard == m_AddrofCardMap.size())
    {
        if(nAllCard != nSendCard)
        {
            int nPack = nSendCard/CARD_MAXLEN + 1;//发送了的包 + 1即包序    每包5个
            int nAllPack = 0;//总包数
            if(nAllCard % CARD_MAXLEN)  //计算总包数
            {
                nAllPack = nAllCard/CARD_MAXLEN + 1;
            }
            else
                nAllPack = nAllCard/CARD_MAXLEN;
            char buf[CARD_MAXLEN*24] = {0};     //buf[120]
            QString str,str1;
            int i = 0;      //定位开始位置
            int k = 0;      //定位结束位置
            if(nPack == nAllPack)       //包序 == 总包数
            {
                i = (nPack-1)*CARD_MAXLEN;  //nSendCard 当前注册的卡数
                k = nAllCard;
            }
            else
            {
                i = (nPack-1)*CARD_MAXLEN;
                k = nPack*CARD_MAXLEN;
            }
            int l = 0;//定位buf
            int nThisSendCard = k - i;
            for(i;i < k;i++)
            {
                str.clear();
                str1.clear();
                str = QString(tt.key());
                str1 = str.left(2);
                str = str.right(str.length()-3);
                bool ok;
                buf[0+l*24] = str1.toInt(&ok,16);

                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[1+l*24] = (str1.left(1).toInt())*16 + str1.right(1).toInt();

                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[2+l*24] = str1.toInt(&ok,16);

                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[3+l*24] = str1.toInt(&ok,16);

                QString addr = QString(tt.value());
                QByteArray byte = addr.toUtf8();
                memcpy(buf+(4+l*24),byte.data(),byte.length());
                tt++;
                l++;
            }
            CUdpRecv->_DownCard_A8(m_Byte,buf,nAllCard,nThisSendCard,nPack,nFlag);
        }
    }
}


//2015 03 26
//接收到下载IC卡的返回的响应
void CSERVERCENTER::SlotRecvDownCard1(unsigned char *buf,QString strIp)
{
    char pAddr[20] = {0};
    memcpy(pAddr,(char *)buf+8,20);
    QString strMiddle = QString(pAddr);
    int nAllCard = buf[28] + buf[29]*256;
    int nSendCard = CARD_MAXLEN*(buf[31]-1) + buf[32] + buf[33]*256;
//    qDebug()<<nSendCard<<"接收到card"<<buf[31]<<buf[32];
    QString strResult = "下载总数:"+QString::number(nAllCard)+"成功下载数:"+QString::number(nSendCard);
    int nId = ui->QCardMessage->currentRow();
    QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(strResult);
    tabledmItem3->setTextAlignment(Qt::AlignCenter);
    ui->QCardMessage->setItem(nId, 3, tabledmItem3);
    if(nAllCard == CUdpRecv->m_Mysql._SelectAllCard(strMiddle))
    {
        if(nAllCard != nSendCard)
        {
            int nPack = nSendCard/CARD_MAXLEN + 1;//发送了的包 + 1即包序
            int nAllPack = 0;//总报数
            if(nAllCard%CARD_MAXLEN)
            {
                nAllPack = nAllCard/CARD_MAXLEN + 1;
            }else nAllPack = nAllCard/CARD_MAXLEN;

//            char *buf = new char[CARD_MAXLEN*5];
//            memset(buf,0,CARD_MAXLEN*5);
            char buf[CARD_MAXLEN*5] = {0};
            QString str,str1;
            int i = 0;//定位开始位置
            int k = 0;//定位结束位置
            if(nPack == nAllPack)
            {
                i = (nPack-1)*CARD_MAXLEN;
                k = nAllCard;
            }else{
                i = (nPack-1)*CARD_MAXLEN;
                k = nPack*CARD_MAXLEN;
            }
            int l = 0;//定位buf
            int nThisSendCard = k - i;

            QDateTime dateTime=QDateTime::currentDateTime();
            QString strDtime = dateTime.toString("yyyy-MM-dd");
            QString SQL = "select IC_Card_num from ic_card where IC_Card_time >= '" + strDtime + "' and " + strMiddle + "= 1 ";
//            QSqlQuery query;
            QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
            QSqlQuery query(db);
            query.exec(SQL);
            int t = 0;
            while(query.next())
            {
            if(t>i&&t<=k)
            {

                str.clear();
                str1.clear();
                str = QString(query.value(0).toByteArray().data());
                str1 = str.left(2);
                str = str.right(str.length()-3);
                bool ok;
                buf[0+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[1+l*5] = (str1.left(1).toInt())*16 + str1.right(1).toInt();
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[2+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[3+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[4+l*5] = str1.toInt(&ok,16);
                l++;
            }
            t++;
            }
            CCONNECTIONPOOLMYSQL::closeConnection(db);
            CUdpRecv->_DownCard(m_Byte,buf,nAllCard,nThisSendCard,nPack);
        }else{
            char pAddr[20] = {0};
            memcpy(pAddr,(char *)buf+8,20);
            CLIST::ItemMiddleUpdateCardToUp(pAddr,CUdpRecv->m_MiddleItem,2);
        }
    }




/*
    if(nAllCard == ui->QIcCard->rowCount())
    {//总数是否相同
        if(nAllCard != nSendCard)
        {
            int nPack = nSendCard/CARD_MAXLEN + 1;//发送了的包 + 1即包序
            int nAllPack = 0;//总报数
            if(nAllCard%CARD_MAXLEN)
            {
                nAllPack = nAllCard/CARD_MAXLEN + 1;
            }else nAllPack = nAllCard/CARD_MAXLEN;

            char *buf = new char[CARD_MAXLEN*5];
            memset(buf,0,CARD_MAXLEN*5);
            QString str,str1;
            int i = 0;//定位开始位置
            int k = 0;//定位结束位置
            if(nPack == nAllPack)
            {
                i = (nPack-1)*CARD_MAXLEN;
                k = nAllCard;
            }else{
                i = (nPack-1)*CARD_MAXLEN;
                k = nPack*CARD_MAXLEN;
            }
            int l = 0;//定位buf
            int nThisSendCard = k - i;
            for(i;i < k;i++)
            {

                str.clear();
                str1.clear();
                str = ui->QIcCard->item(i, 1)->text();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                bool ok;
                buf[0+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[1+l*5] = (str1.left(1).toInt())*16 + str1.right(1).toInt();
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[2+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[3+l*5] = str1.toInt(&ok,16);
                str1.clear();
                str1 = str.left(2);
                str = str.right(str.length()-3);
                buf[4+l*5] = str1.toInt(&ok,16);
                l++;
            }
            CUdpRecv->_DownCard(m_Byte,buf,nAllCard,nThisSendCard,nPack);
        }
    }*/
}
//响应保存IC卡权限修改的动作
void CSERVERCENTER::SlotCardMessageSave()
{
    if(ui->QIcCard->selectedItems().count())
    {
    NodeCard *q = m_ItemCard;
    while(q->next)
    {
        if(q->data.nNewCheck != q->data.nOldCheck)
        {
            CUdpRecv->m_Mysql._UpdateCardMessage(m_strIcCard,QString(q->data.gcAddr),q->data.nNewCheck);
            q->data.nOldCheck = q->data.nNewCheck;
        }
        q = q->next;
    }
    }else
        QMessageBox::information(this,tr("提示"),tr("请选择需要修改的IC卡"));
}

void CSERVERCENTER::SlotRecvDownAloneCard(int nAllCard)
{
    int nRow = ui->QCardMessage->currentRow();
    QString strResult = QString::number(nAllCard) + "张卡注册成功";
    QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(strResult);
    tabledmItem3->setTextAlignment(Qt::AlignCenter);
    ui->QCardMessage->setItem(nRow, 3, tabledmItem3);
}

void CSERVERCENTER::SlotLogOutCard()
{
    int nRow = ui->QCardMessage->currentRow();
    QString strAddr = ui->QCardMessage->item(nRow,1)->text();
    QString strType = ui->QCardMessage->item(nRow,2)->text();
    QString strIntraAddr = _AddrExplainToAddr(strAddr,strType,1);
    m_Byte.clear();
    m_Byte = strIntraAddr.toUtf8();
    m_ListSelected.clear();
    if(_GetSelectedRow(&m_ListSelected,1))
    {
    int nAllCard = m_ListSelected.size();
    char buf[CARD_MAXLEN*5] = {0};
    QString str,str1;
    if(nAllCard > CARD_MAXLEN )
    {
        for(int i = 0; i < CARD_MAXLEN;i++)
        {
            str.clear();
            str1.clear();
            str = ui->QIcCard->item(m_ListSelected.at(i),1)->text();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            bool ok;
            buf[0+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[1+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[2+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[3+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[4+i*5] = str1.toInt(&ok,16);
        }
    if(CUdpRecv->_LogOutCard(m_Byte,buf,nAllCard,CARD_MAXLEN,1) == 0)
        QMessageBox::information(this,tr("提示"),tr("注销失败"));
    }else {
        for(int i = 0; i < nAllCard;i++)
        {
            str.clear();
            str1.clear();
            str = ui->QIcCard->item(m_ListSelected.at(i),1)->text();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            bool ok;
            buf[0+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[1+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[2+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[3+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[4+i*5] = str1.toInt(&ok,16);
        }
//        qDebug()<<"mabager"<<nAllCard;
        if(CUdpRecv->_LogOutCard(m_Byte,buf,nAllCard,nAllCard,1) == 0)
            QMessageBox::information(this,tr("提示"),tr("注销失败"));
    }
}else{
    QMessageBox::information(this,tr("提示"),tr("请选择需要注销的IC卡"));
}
}


bool CSERVERCENTER::_GetSelectedRow(QList<int> *set_row,int nfalg)
{
    QList<QTableWidgetItem*> items = ui->QIcCard->selectedItems();
    int item_count = items.count();
    if(item_count <= 0)
    {
        return false;
    }
    for(int i=0; i<item_count;i++)
    {
        //获取选中的行
        int item_row = ui->QIcCard->row(items.at(i));
        if(set_row->indexOf(item_row) < 0){
//            qDebug()<<item_row<<"item_row";
        QString strTime = ui->QIcCard->item(item_row,3)->text();
        QDate ValidTime= QDate::fromString(strTime,"yyyy-MM-dd");
        QDate NowTime= QDateTime::currentDateTime().date();
        if(nfalg == 2){
        if(ValidTime.daysTo(NowTime) <= 0) set_row->append(item_row);
        }else{
            set_row->append(item_row);
        }
        }
    }
    return  true;
}
//覆盖
void CSERVERCENTER::SlotDownAllCard()
{
    int nRow = ui->QCardMessage->currentRow();
    QString strAddr = ui->QCardMessage->item(nRow,1)->text();
    QString strType = ui->QCardMessage->item(nRow,2)->text();
    QString strIntraAddr = _AddrExplainToAddr(strAddr,strType,1);
    m_Byte.clear();
    m_Byte = strIntraAddr.toUtf8();
    int nAllCard = CUdpRecv->m_Mysql._SelectAllCard(strIntraAddr);
//    char *buf = new char[CARD_MAXLEN*5];
//    memset(buf,0,CARD_MAXLEN*5);
    char buf[CARD_MAXLEN*5] = {0};
    QString str,str1;
    if(nAllCard > CARD_MAXLEN )
    {
        QDateTime dateTime=QDateTime::currentDateTime();
        QString strDtime = dateTime.toString("yyyy-MM-dd");
        QString SQL = "select IC_Card_num from ic_card where IC_Card_time > '" + strDtime + "' and " + strIntraAddr + "= 1 ";
//        QSqlQuery query;
        QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
        QSqlQuery query(db);
        query.exec(SQL);
        int i = 0;
        while(query.next())
        {
        if(i>=0&&i<CARD_MAXLEN)
        {
        str.clear();
        str1.clear();
        str = QString(query.value(0).toByteArray().data());
        str1 = str.left(2);
        str = str.right(str.length()-3);
        bool ok;
        buf[0+i*5] = str1.toInt(&ok,16);
        str1.clear();
        str1 = str.left(2);
        str = str.right(str.length()-3);
        buf[1+i*5] = str1.toInt(&ok,16);
        str1.clear();
        str1 = str.left(2);
        str = str.right(str.length()-3);
        buf[2+i*5] = str1.toInt(&ok,16);
        str1.clear();
        str1 = str.left(2);
        str = str.right(str.length()-3);
        buf[3+i*5] = str1.toInt(&ok,16);
        str1.clear();
        str1 = str.left(2);
        str = str.right(str.length()-3);
        buf[4+i*5] = str1.toInt(&ok,16);
        }
        i++;
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
    if(CUdpRecv->_DownAllCard(m_Byte,buf,nAllCard,CARD_MAXLEN,1) ==0 )
        QMessageBox::information(this,tr("提示"),tr("下载失败"));
    }else {
        QDateTime dateTime=QDateTime::currentDateTime();
        QString strDtime = dateTime.toString("yyyy-MM-dd");
        QString SQL = "select IC_Card_num from ic_card where IC_Card_time > '" + strDtime + "' and " + strIntraAddr + "= 1 ";
//        QSqlQuery query;
        QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
        QSqlQuery query(db);
        query.exec(SQL);
        int i = 0;
        while(query.next())
        {
            str.clear();
            str1.clear();
            str = QString(query.value(0).toByteArray().data());
//            qDebug()<<SQL<<"2222"<<str;
            str1 = str.left(2);
            str = str.right(str.length()-3);
            bool ok;
            buf[0+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[1+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[2+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[3+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[4+i*5] = str1.toInt(&ok,16);
            i++;
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        if(CUdpRecv->_DownAllCard(m_Byte,buf,nAllCard,nAllCard,1) == 0)
            QMessageBox::information(this,tr("提示1"),tr("注册失败"));
    }
}

void CSERVERCENTER::SlotDownCard_A8()
{
    if(ui->Issued_Machine->currentItem()->text(0) != "中间设备")
    {
        QString strAddr = ui->Issued_Machine->currentItem()->text(0);
        QString strType = "门口机";
        QString strIntraAddr = _AddrExplainToAddrA8(strAddr,strType,1);
        m_Byte.clear();
        m_Byte = strIntraAddr.toUtf8();
        int nAllCard = 0;
        nAllCard = m_AddrofCardMap.size();
        if(nAllCard > 0)
        {
            char buf[CARD_MAXLEN*24] = {0};
            QString str,str1;
            if(nAllCard > CARD_MAXLEN)
            {
                tt = m_AddrofCardMap.begin();
                for(int i = 0; i<CARD_MAXLEN; i++)
                {
                    str.clear();
                    str1.clear();
                    str = QString(tt.key());
//                    qDebug()<<str;
                    str1 = str.left(2);
                    str = str.right(str.length()-3);

                    bool ok;
                    buf[0+i*24] = str1.toInt(&ok,16);

                    int tBuf = buf[0+i*24];
                    str1.clear();
                    str1 = str.left(2);
                    str = str.right(str.length()-3);
                    buf[1+i*24] = str1.toInt(&ok,16);
                    str1.clear();
                    str1 = str.left(2);
                    str = str.right(str.length()-3);
                    buf[2+i*24] = str1.toInt(&ok,16);
                    str1.clear();
                    str1 = str.left(2);
                    str = str.right(str.length()-3);
                    buf[3+i*24] = str1.toInt(&ok,16);
                    QString addr = QString(tt.value());
//                    qDebug()<<addr;
                    QByteArray byte = addr.toUtf8();
                    memcpy(buf+(4+i*24),byte.data(),byte.length());
                    tt++;
                }
                if(CUdpRecv->_DownCard_A8(m_Byte,buf,nAllCard,CARD_MAXLEN,1) == 0)
                    QMessageBox::information(this,tr("提示2"),tr("注册失败"));
            }
            else
            {   tt = m_AddrofCardMap.begin();
                for(int i = 0; i < nAllCard;i++)
                {
                    str.clear();
                    str1.clear();
                    str = tt.key();
                    str1 = str.left(2);
                    str = str.right(str.length()-3);

                    bool ok;
                    buf[0+i*24] = str1.toInt(&ok,16);
                    str1.clear();
                    str1 = str.left(2);
                    str = str.right(str.length()-3);
                    buf[1+i*24] = str1.toInt(&ok,16);
                    str1.clear();
                    str1 = str.left(2);
                    str = str.right(str.length()-3);
                    buf[2+i*24] = str1.toInt(&ok,16);
                    str1.clear();
                    str1 = str.left(2);
                    str = str.right(str.length()-3);
                    buf[3+i*24] = str1.toInt(&ok,16);
                    str1.clear();
                    str1 = str.left(2);
                    str = str.right(str.length()-3);
                    buf[4+i*24] = str1.toInt(&ok,16);
                    QString addr = QString(tt.value());
                    QByteArray byte = addr.toUtf8();
                    memcpy(buf+(4+i*24),byte.data(),byte.length());
                    tt++;
                }
                if(CUdpRecv->_DownCard_A8(m_Byte,buf,nAllCard,nAllCard,1) == 0)
                    QMessageBox::information(this,tr("提示3"),tr("注册失败"));
            }
        }
        else
            QMessageBox::information(this,"提示","此门口机无IC卡下载");
    }
    else
        QMessageBox::information(this,"错误","请选择到门口机的设备号");
}

void CSERVERCENTER::SlotDownCard()
{
    int nRow = ui->QCardMessage->currentRow();
    QString strAddr = ui->QCardMessage->item(nRow,1)->text();
    QString strType = ui->QCardMessage->item(nRow,2)->text();
    QString strIntraAddr = _AddrExplainToAddr(strAddr,strType,1);
    m_Byte.clear();
    m_Byte = strIntraAddr.toUtf8();
    m_ListSelected.clear();
    if(_GetSelectedRow(&m_ListSelected,2))
    {
    int nAllCard = m_ListSelected.size();
    char buf[CARD_MAXLEN*5] = {0};
    QString str,str1;
    if(nAllCard > CARD_MAXLEN )
    {
        for(int i = 0; i < CARD_MAXLEN;i++)
        {
            str.clear();
            str1.clear();
            str = ui->QIcCard->item(m_ListSelected.at(i),1)->text();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            bool ok;
            buf[0+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[1+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[2+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[3+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[4+i*5] = str1.toInt(&ok,16);
        }
    if(CUdpRecv->_DownCard(m_Byte,buf,nAllCard,CARD_MAXLEN,1) == 0)
        QMessageBox::information(this,tr("提示4"),tr("注册失败"));
    }
    else
    {
        for(int i = 0; i < nAllCard;i++)
        {
            str.clear();
            str1.clear();
            str = ui->QIcCard->item(m_ListSelected.at(i),1)->text();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            bool ok;
            buf[0+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[1+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[2+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[3+i*5] = str1.toInt(&ok,16);
            str1.clear();
            str1 = str.left(2);
            str = str.right(str.length()-3);
            buf[4+i*5] = str1.toInt(&ok,16);
        }
//        qDebug()<<"mabager"<<nAllCard;
        if(CUdpRecv->_DownCard(m_Byte,buf,nAllCard,nAllCard,1) == 0)
        {
            QMessageBox::information(this,tr("提示5"),tr("注册失败"));
        }
    }
}
    else
    {
        QMessageBox::information(this,tr("提示"),tr("请选择需要注册的IC卡"));
    }
}

//响应查找界面确定查找条件
void CSERVERCENTER::SlotMiddleFind(int nIndex,QString strFind)
{
    switch(CUdpRecv->m_nPage)
    {
        case 1:
            _UpdateFindTenement(CUdpRecv->m_TenementItem,nIndex,strFind);
            break;
        case 2:
            _UpdateFindMiddle(CUdpRecv->m_MiddleItem,nIndex,strFind);
            break;
        default :
            break;
    }
}
//响应查找动作，显示查找界面
void CSERVERCENTER::SlotFindMiddle()
{
    CFindDevice->_Update(CUdpRecv->m_nPage);
    CFindDevice->show();
}

//响应初始化开始密码动作
void CSERVERCENTER::SlotInitLockPass()
{
    switch(CUdpRecv->m_nPage)
    {
        case 2:
        {
            int i = ui->QMiddle->currentRow();
            if(i >= 0)
            {
                QString strMac = ui->QMiddle->item(i, 3)->text();;
                QString strIp = ui->QMiddle->item(i, 4)->text();
                QString strAddr = ui->QMiddle->item(i, 1)->text();
                QString strType = ui->QMiddle->item(i, 6)->text();
                switch( QMessageBox::question( this, tr("提示"),"确定要重置"+strType+strAddr+" 的开锁密码",tr("Yes"), tr("No"),0, 1 ) )
                {
                    case 0:
                        CUdpRecv->_SendPass(strAddr,strMac,strIp,strType,2);
                        break;
                    default :
                        break;
                }
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
            break;
        default :
            break;
    }
}

void CSERVERCENTER::SlotInitLockPassA8()
{
    switch(CUdpRecv->m_nPage)
    {
        case 2:
        {
            int i = ui->QMiddle->currentRow();
            if(i >= 0)
            {
                QString strIp = ui->QMiddle->item(i, 4)->text();
                QString strAddr = ui->QMiddle->item(i, 2)->text();
                QString strType = ui->QMiddle->item(i, 6)->text();
                switch( QMessageBox::question( this, tr("提示"),"确定要重置"+strType+strAddr+" 的开锁密码",tr("Yes"), tr("No"),0, 1 ) )
                {
                    case 0:
                        CUdpRecv->_SendPassA8(strAddr,strIp,strType,2);
                        break;
                    default :
                        break;
                }
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
            break;
        default :
            break;
    }
}

//响应初始化管理密码动作
void CSERVERCENTER::SlotInitSystemPass()
{
    switch(CUdpRecv->m_nPage)
    {
        case 1:
        {
            int i = ui->QTenement->currentRow();
            if(i >= 0)
            {
                QString strMac = ui->QTenement->item(i, 6)->text();;
                QString strIp = ui->QTenement->item(i, 7)->text();
                QString strAddr = ui->QTenement->item(i, 1)->text();
                QString strType = ui->QTenement->item(i, 11)->text();
                switch( QMessageBox::question( this, tr("提示"),"确定要重置"+strType+strAddr+" 的工程密码",tr("Yes"), tr("No"),0, 1 ) )
                {
                    case 0:
                        CUdpRecv->_SendPass(strAddr,strMac,strIp,strType,1);
                        break;
                    default :
                        break;
              }
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
            break;
        case 2:
        {
            int i = ui->QMiddle->currentRow();
            if(i >= 0)
            {
                QString strMac = ui->QMiddle->item(i, 3)->text();;
                QString strIp = ui->QMiddle->item(i, 4)->text();
                QString strAddr = ui->QMiddle->item(i, 1)->text();
                QString strType = ui->QMiddle->item(i, 6)->text();
                switch( QMessageBox::question( this, tr("提示"),"确定要重置"+strType+strAddr+" 的工程密码",tr("Yes"), tr("No"),0, 1 ) )
                {
                    case 0:
                        CUdpRecv->_SendPass(strAddr,strMac,strIp,strType,1);
                        break;
                    default :
                        break;
                }
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
            break;
        default :
            break;
    }
}

void CSERVERCENTER::SlotInitSystemPassA8()
{
    switch(CUdpRecv->m_nPage)
    {
        case 1:
        {
            int i = ui->QTenement->currentRow();
            if(i >= 0)
            {
                QString strIp = ui->QTenement->item(i, 7)->text();
                QString strAddr = ui->QTenement->item(i, 2)->text();
                QString strType = ui->QTenement->item(i, 11)->text();
                switch( QMessageBox::question( this, tr("提示"),"确定要重置"+strType+strAddr+" 的工程密码",tr("Yes"), tr("No"),0, 1 ) )
                {
                    case 0:
                        CUdpRecv->_SendPassA8(strAddr,strIp,strType,1);
                        break;
                    default :
                        break;
                }
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
            break;
        case 2:
        {
            int i = ui->QMiddle->currentRow();
            if(i >= 0)
            {
                QString strIp = ui->QMiddle->item(i, 4)->text();
                QString strAddr = ui->QMiddle->item(i, 2)->text();
                QString strType = ui->QMiddle->item(i, 6)->text();
                switch( QMessageBox::question( this, tr("提示"),"确定要重置"+strType+strAddr+" 的工程密码",tr("Yes"), tr("No"),0, 1 ) )
                {
                    case 0:
                        CUdpRecv->_SendPassA8(strAddr,strIp,strType,1);
                        break;
                    default :
                        break;
                }
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
    break;
    default :
        break;
    }
}

void CSERVERCENTER::SlotDownRepairsA8()
{
    char p[1024] = {'0'};
    QString str;
    int nAllRepairs = ui->QRepairs->rowCount();
    if(nAllRepairs > 0)
    {
        switch( QMessageBox::question( this, tr("提示"),"确定要下发全部报修类型",tr("Yes"), tr("No"),0, 1 ) )
        {
        case 0:
            for(int i = 0;i < nAllRepairs;i++)
            {
                str.clear();
                str = ui->QRepairs->item(i, 0)->text();
                //        char *p1 = new char [24];
                //        memset(p1,0,24);
                //        p1 = str.toUtf8().data();
                memcpy(p+i*12,str.toLocal8Bit().data(),str.toLocal8Bit().size());
            }
            CUdpRecv->_SendRepairsA8(nAllRepairs,p);
            QMessageBox::information(this,tr("提示"),tr("下发完成"));
            break;
        default :
            break;
        }
    }
    else
    {
        QMessageBox::information(this,tr("提示"),tr("下发类型数为0，请先添加报修类型再下发"));
    }
}

//响应删除设备动作
void CSERVERCENTER::SlotDeleteDevice()
{
    switch(CUdpRecv->m_nPage)
    {
        case 1:         //删除住户设备
        {
            int i = ui->QTenement->currentRow();
            if(i >= 0)
            {
                STenementDevice sTenementDevice;
                memset(sTenementDevice.gcMacAddr,0,20);

                QString strMac = ui->QTenement->item(i, 6)->text();
                QString strIp = ui->QTenement->item(i, 7)->text();
                QString strAddr = ui->QTenement->item(i, 1)->text();
                QString strType = ui->QTenement->item(i, 11)->text();

                switch( QMessageBox::question( this, tr("提示"),"确定要删除"+strType+strAddr,tr("Yes"), tr("No"),0, 1 ) )
                {
                    case 0:
                    {
                        if(CUdpRecv->m_Mysql._DeleteTenement(strMac,strIp,strAddr,strType))
                        {
                            QByteArray byte;
                            byte = strMac.toAscii();
                            memcpy(sTenementDevice.gcMacAddr,byte.data(),byte.size());
                            CUdpRecv->m_TenementItem = CLIST::DeleteItemTenement(sTenementDevice,&(CUdpRecv->m_TenementItem));
                            CUdpRecv->m_nAllTenement --;
                            SlotInitTenement();
                            QMessageBox::information(this,tr("提示"),tr("删除成功"));
                        }
                    }
                        break;
                    case 1:
                        break;
                    default :
                        break;
                }
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
            break;
        case 2:         //删除中间设备
        {
            int i = ui->QMiddle->currentRow();
            if(i >= 0)
            {
                SMiddleDevice sMiddleDevice;
                memset(sMiddleDevice.gcMacAddr,0,20);

                QString strMac = ui->QMiddle->item(i, 3)->text();;
                QString strIp = ui->QMiddle->item(i, 4)->text();
                QString strAddr = ui->QMiddle->item(i, 1)->text();
                QString strType = ui->QMiddle->item(i, 6)->text();
                switch( QMessageBox::question( this, tr("提示"),"确定要删除"+strType+strAddr,tr("Yes"), tr("No"),0, 1 ) )
                {
                    case 0:
                        if(CUdpRecv->m_Mysql._DeleteMiddle(strMac,strIp,strAddr,strType))
                        {
                            QByteArray byte;
                            byte = strMac.toAscii();
                            memcpy(sMiddleDevice.gcMacAddr,byte.data(),byte.size());
                            CUdpRecv->m_MiddleItem = CLIST::DeleteItemMiddle(sMiddleDevice,&(CUdpRecv->m_MiddleItem));
                            CUdpRecv->m_nAllMiddle --;
                            SlotInitMiddle();
                            QMessageBox::information(this,tr("提示"),tr("删除成功"));
                        }
                        break;
                    default :
                        break;
                }
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
            break;
        default :
            break;
    }
}
//响应天气设置里的更新时间
void CSERVERCENTER::SlotUpdateCity(QString strCityNum,QString strCity)
{
    CUdpRecv->_UpdateLoading(strCityNum);
    CWeather->_Update(strCity,CUdpRecv->sWeather.nMinTemp,CUdpRecv->sWeather.nMaxTemp,CUdpRecv->sWeather.nWeather);
}
//响应设置天气界面的设置天气
void CSERVERCENTER::SlotUpdateWeather(int MaxTemp,int MinTemp,int Weather)
{
//    qDebug()<<Weather;
    CUdpRecv->sWeather.nMinTemp = MinTemp;
    CUdpRecv->sWeather.nMaxTemp = MaxTemp;
    CUdpRecv->sWeather.nWeather = Weather;
}
//停止播放报警音乐
void CSERVERCENTER::SlotStop()
{
    if(m_MediaObject->state() == Phonon::PlayingState)
        m_MediaObject->stop();
}
//报警列表里的处理报警的动作的响应
void CSERVERCENTER::SlotDeal()
{
    int i = ui->QAlarm->currentRow();
    if(i >= 0)
    {
        SAlarmDevice sAlarmDevice;
        memset(sAlarmDevice.gcAddr,0,64);
        memset(sAlarmDevice.gcIntraAddr,0,20);
        memset(sAlarmDevice.gcPhone1,0,20);
        memset(sAlarmDevice.gcName,0,64);
        memset(sAlarmDevice.gcIpAddr,0,20);
        memset(sAlarmDevice.gcMacAddr,0,20);
        memset(sAlarmDevice.gcStime,0,20);
        memset(sAlarmDevice.gcEtime,0,20);
        memset(sAlarmDevice.gcType,0,20);

        sAlarmDevice.nId = ui->QAlarm->currentRow();
        if(CUdpRecv->_GetAlarmFromItem(&sAlarmDevice,CUdpRecv->m_AlarmItem))
        {
            CDealAlarm->_Update(sAlarmDevice);
            CDealAlarm->show();
        }
    }else{
        QMessageBox::information(this,tr("提示"),tr("请选中目标"));
    }
}
//报警记录列表出处理报警的动作的响应
void CSERVERCENTER::SlotAlarmDeal()
{
    int ti = ui->QAlarmRecord->currentRow();
    if(ti >= 0 )
    {
        QString strDtime = ui->QAlarmRecord->item(ti, 9)->text();
        if(strDtime == NULL)
        {
            QString strIPAddr = ui->QAlarmRecord->item(ti, 1)->text();
            QString strAddr = ui->QAlarmRecord->item(ti, 2)->text();
            QString strName = ui->QAlarmRecord->item(ti, 4)->text();
            QString strPhone = ui->QAlarmRecord->item(ti, 5)->text();
            QString strFence = ui->QAlarmRecord->item(ti, 6)->text();
            QString strType = ui->QAlarmRecord->item(ti, 7)->text();
            QString strStime = ui->QAlarmRecord->item(ti, 8)->text();
            QString strEtime = ui->QAlarmRecord->item(ti, 10)->text();
            SAlarmDevice sAlarmDevice;
            memset(sAlarmDevice.gcAddr,0,64);
            memset(sAlarmDevice.gcIntraAddr,0,20);
            memset(sAlarmDevice.gcPhone1,0,20);
            memset(sAlarmDevice.gcName,0,64);
            memset(sAlarmDevice.gcIpAddr,0,20);
            memset(sAlarmDevice.gcMacAddr,0,20);
            memset(sAlarmDevice.gcStime,0,20);
            memset(sAlarmDevice.gcEtime,0,20);
            memset(sAlarmDevice.gcType,0,20);

            QByteArray byte;

            byte.clear();
            byte = strIPAddr.toUtf8();
            memcpy(sAlarmDevice.gcIpAddr,byte.data(),byte.size());

            byte.clear();
            byte = strAddr.toUtf8();
            memcpy(sAlarmDevice.gcAddr,byte.data(),byte.size());

            byte.clear();
            byte = strName.toUtf8();
            memcpy(sAlarmDevice.gcName,byte.data(),byte.size());

            byte.clear();
            byte = strPhone.toUtf8();
            memcpy(sAlarmDevice.gcPhone1,byte.data(),byte.size());

            byte.clear();
            byte = strStime.toUtf8();
            memcpy(sAlarmDevice.gcStime,byte.data(),byte.size());

            byte.clear();
            byte = strEtime.toUtf8();
            memcpy(sAlarmDevice.gcEtime,byte.data(),byte.size());

            sAlarmDevice.nFenceId = strFence.right(1).toInt();


            if(strType == "火警")
            {
                sAlarmDevice.nAlarmType= 0;
            }else if(strType == "红外"){
                sAlarmDevice.nAlarmType= 2;
            }else if(strType == "门铃"){
                sAlarmDevice.nAlarmType= 3;
            }else if(strType == "烟感"){
                sAlarmDevice.nAlarmType= 4;
            }else if(strType == "门磁1"){
                sAlarmDevice.nAlarmType= 6;
            }else if(strType == "遥控SOS"){
                sAlarmDevice.nAlarmType= 7;
            }else if(strType == "门磁2"){
                sAlarmDevice.nAlarmType= 8;
            }else if(strType == "水浸"){
                sAlarmDevice.nAlarmType= 10;
            }else if(strType == "煤气"){
                sAlarmDevice.nAlarmType= 12;
            }else if(strType == "门磁3"){
                sAlarmDevice.nAlarmType= 13;
            }else if(strType == "胁迫开门"){
                sAlarmDevice.nAlarmType= 14;
            }else if(strType == "设备强拆"){
                sAlarmDevice.nAlarmType = 15;
            }else if(strType == "开门超时"){
                sAlarmDevice.nAlarmType = 16;
            }else{
                sAlarmDevice.nAlarmType = 17;
            }


            CDealAlarm->_Update(sAlarmDevice);
            CDealAlarm->show();
        }else{
            QMessageBox::information(this,tr("提示"),tr("已处理过的记录"));
            return;
        }
    }else{
QMessageBox::information(this,tr("提示"),tr("请选中目标"));
}
}
//查找界面的确认查找条件的响应
void CSERVERCENTER::SlotFind(int nIndex,int nTime,QString strFind,QString strStime,QString strEtime)
{
    QString SQL;
    SQL.clear();
    m_nFind = 1;
    m_nPage = 0;
    switch(CUdpRecv->m_nPage)
    {
        case 3:             //报警
            switch(nIndex)
            {
                case 0:
                    if(nTime > 0)
                    {
                        SQL  = " (Alarm_addr like '%"+strFind+"%' or Alarm_intra_addr like '%"+strFind+"%') and Alarm_stime > '"+strStime+"' and Alarm_stime < '"+strEtime+"' ORDER BY  Alarm_dtime,Alarm_stime DESC";
                    }
                    else
                    {
                        SQL  = " (Alarm_addr like '%"+strFind+"%' or Alarm_intra_addr like '%"+strFind+"%') ORDER BY  Alarm_dtime,Alarm_stime DESC";
                    }
                    break;
                case 1:
                    if(nTime > 0)
                    {
                        SQL  = " Alarm_ip_addr like '%"+strFind+"%' and Alarm_stime > '"+strStime+"' and Alarm_stime < '"+strEtime+"' ORDER BY  Alarm_dtime,Alarm_stime DESC";
                    }
                    else
                    {
                        SQL  = " Alarm_ip_addr like '%"+strFind+"%' ORDER BY  Alarm_dtime,Alarm_stime DESC";
                    }
                    break;
                case 2:
                    if(nTime > 0)
                    {
                        SQL  = " Alarm_dtime is NULL and Alarm_stime > '"+strStime+"' and Alarm_stime < '"+strEtime+"' ORDER BY  Alarm_dtime,Alarm_stime DESC";
                    }
                    else
                    {
                        SQL  = " Alarm_dtime is NULL ORDER BY  Alarm_dtime,Alarm_stime DESC";
                    }
                    break;
                default :
                    break;
            }
            _UpdateFindAlarmRecord(SQL);
            break;
        case 4:             //报修
            switch(nIndex)
            {
                case 0:
                    if(nTime > 0)
                    {
                        SQL  = " (Repairs_Record_addr like '%"+strFind+"%' or Repairs_Record_intra_addr like '%"+strFind+"%') and Repairs_Record_stime > '"+strStime+"' and Repairs_Record_stime < '"+strEtime+"' ORDER BY  Repairs_Record_stime,Repairs_Record_dtime DESC";
                    }
                    else
                    {
                        SQL  = " (Repairs_Record_addr like '%"+strFind+"%' or Repairs_Record_intra_addr like '%"+strFind+"%') ORDER BY  Repairs_Record_dtime,Repairs_Record_stime DESC";
                    }
                    break;
                case 1:
                    if(nTime > 0)
                    {
                        SQL  = " Repairs_Record_type like '%"+strFind+"%' and Repairs_Record_stime > '"+strStime+"' and Repairs_Record_stime < '"+strEtime+"' ORDER BY  Repairs_Record_stime,Repairs_Record_dtime DESC";
                    }
                    else
                    {
                        SQL  = " Repairs_Record_type like '%"+strFind+"%' ORDER BY  Repairs_Record_dtime,Repairs_Record_stime DESC";
                    }
                    break;
                case 2:
                    if(nTime > 0)
                    {
                        SQL  = " Repairs_Record_dtime is NULL and Repairs_Record_stime > '"+strStime+"' and Repairs_Record_stime < '"+strEtime+"' ORDER BY  Repairs_Record_stime,Repairs_Record_dtime DESC";
                    }
                    else
                    {
                        SQL  = " Repairs_Record_dtime is NULL ORDER BY  Repairs_Record_dtime,Repairs_Record_stime DESC";
                    }
                    break;
                default :
                    break;
            }
            _UpdateFindRepairsRecord(SQL);
            break;
        case 7:             //系统
            switch(nIndex)
            {
                case 0:
                    if(nTime > 0)
                    {
                        SQL  = " System_name like '%"+strFind+"%' and System_time > '"+strStime+"' and System_time < '"+strEtime+"' ORDER BY  System_time DESC";
                    }
                    else
                    {
                        SQL  = " System_name like '%"+strFind+"%' ORDER BY  System_time DESC";
                    }
                    break;
                case 1:
                    if(nTime > 0)
                    {
                        SQL  = " System_human like '%"+strFind+"%' and System_time > '"+strStime+"' and System_time < '"+strEtime+"' ORDER BY  System_time DESC";
                    }
                    else
                    {
                        SQL  = " System_human like '%"+strFind+"%' ORDER BY  System_time DESC";
                    }
                    break;
                case 2:
                    if(nTime > 0)
                    {
                        SQL  = " System_type like '%"+strFind+"%' and System_time > '"+strStime+"' and System_time < '"+strEtime+"' ORDER BY  System_time DESC";
                    }
                    else
                    {
                        SQL  = " System_type like '%"+strFind+"%' ORDER BY  System_time DESC";
                    }
                    break;
                default :
                    break;
            }
            _UpdateFindSystem(SQL);
            break;
        case 8:         //设备
            switch(nIndex)
            {
                case 0:
                    if(nTime > 0)
                    {
                        SQL  = " (Device_addr like '%"+strFind+"%' or Device_intra_addr like '%"+strFind+"%') and Device_time > '"+strStime+"' and Device_time < '"+strEtime+"' ORDER BY  Device_time DESC";
                    }
                    else
                    {
                        SQL  = " (Device_addr like '%"+strFind+"%' or Device_intra_addr like '%"+strFind+"%') ORDER BY  Device_time DESC";
                    }
                    break;
                case 1:
                    if(nTime > 0)
                    {
                        SQL  = " Device_ip_addr like '%"+strFind+"%' and Device_time > '"+strStime+"' and Device_time < '"+strEtime+"' ORDER BY  Device_time DESC";
                    }
                    else
                    {
                        SQL  = " Device_ip_addr like '%"+strFind+"%' ORDER BY  Device_time DESC";
                    }
                    break;
                case 2:
                    if(nTime > 0)
                    {
                        SQL  = " Device_status like '%"+strFind+"%' and Device_time > '"+strStime+"' and Device_time < '"+strEtime+"' ORDER BY  Device_time DESC";
                    }
                    else
                    {
                        SQL  = " Device_status like '%"+strFind+"%' ORDER BY  Device_time DESC";
                    }
                    break;
                default :
                    break;
            }
            _UpdateFindDevice(SQL);
            break;
        case 9:         //刷卡
            switch(nIndex)
            {
                case 0:
                    if(nTime > 0)
                    {
                        SQL  = " Card_Record_card_num like '%"+strFind+"%' and Card_Record_time > '"+strStime+"' and Card_Record_time < '"+strEtime+"' ORDER BY  Card_Record_time DESC";
                    }
                    else
                    {
                        SQL  = " Card_Record_card_num like '%"+strFind+"%' ORDER BY Card_Record_time DESC";
                    }
                    break;
                case 1:
                    if(nTime > 0)
                    {
                        SQL  = " (Card_Record_addr like '%"+strFind+"%' or Card_Record_intra_addr like '%"+strFind+"%') and Card_Record_time > '"+strStime+"' and Card_Record_time < '"+strEtime+"' ORDER BY  Card_Record_time DESC";
                    }
                    else
                    {
                        SQL  = " (Card_Record_addr like '%"+strFind+"%' or Card_Record_intra_addr like '%"+strFind+"%') ORDER BY Card_Record_time DESC";
                    }
                    break;
                case 2:
                    if(nTime > 0)
                    {
                        SQL  = " Card_Record_device_type like '%"+strFind+"%' and Card_Record_time > '"+strStime+"' and Card_Record_time < '"+strEtime+"' ORDER BY  Card_Record_time DESC";
                    }
                    else
                    {
                        SQL  = " Card_Record_device_type like '%"+strFind+"%' ORDER BY Card_Record_time DESC";
                    }
                    break;
                default :
                    break;
            }
            _UpdateFindCardRecord(SQL);
            break;
        default :
            break;
    }
    m_strFindSql = SQL;
}

//通话记录删除动作的响应
void CSERVERCENTER::SlotCallRecordDelete()
{
    CDelete->_Update(CUdpRecv->m_nPage);
    CDelete->show();
}
//删除界面确认删除的动作响应
void CSERVERCENTER::SlotDelete()
{
    _UpdatePage(CUdpRecv->m_nPage);
}
//报修处理界面处理报修确认的响应
void CSERVERCENTER::SlotDealRepairs()
{
    if(m_MediaObject->state() == Phonon::PlayingState)
        m_MediaObject->stop();
    _UpdateRepairsRecord();
}
//操作员管理界面确认修改后的响应
void CSERVERCENTER::SlotUpdateUser()
{
    _UpdateUser();
}
//报修记录查询动作的响应
void CSERVERCENTER::SlotRepairsRecordFind()
{
    CFind->_Update(CUdpRecv->m_nPage);
    CFind->show();
}
//报修记录删除动作的响应
void CSERVERCENTER::SlotRepairsRecordDelete()
{
    CDelete->_Update(CUdpRecv->m_nPage);
    CDelete->show();
}
//报修记录处理动作的响应
void CSERVERCENTER::SlotRepairsRecordDeal()
{
    int ti = ui->QRepairsRecord->currentRow();
    if(ti >= 0)
    {
    QString strAddr = ui->QRepairsRecord->item(ti, 1)->text();
    QString strType = ui->QRepairsRecord->item(ti, 3)->text();
    QString strStime = ui->QRepairsRecord->item(ti, 4)->text();
    QString strEtime = ui->QRepairsRecord->item(ti, 5)->text();
    QString strDtime = ui->QRepairsRecord->item(ti, 6)->text();
    if(strDtime != NULL)
    {
        QMessageBox::information(this,tr("提示"),tr("已经处理过的记录"));
    }else{
        CDealRepairs->_Update(strAddr,strType,strStime,strEtime);
        CDealRepairs->show();
    }
    }else{
        QMessageBox::information(this,tr("提示"),tr("请选中目标"));
    }
}
//报警记录查找动作的响应
void CSERVERCENTER::SlotAlarmRecordFind()
{
    CFind->_Update(CUdpRecv->m_nPage);
    CFind->show();
}
//报警记录删除动作的响应
void CSERVERCENTER::SlotAlarmRecordDelete()
{
    CDelete->_Update(CUdpRecv->m_nPage);
    CDelete->show();
}
//刷卡记录查找动作的响应
void CSERVERCENTER::SlotCardRecordFind()
{
    CFind->_Update(CUdpRecv->m_nPage);
    CFind->show();
}
//刷卡记录删除动作的响应
void CSERVERCENTER::SlotCardRecordDelete()
{
    CDelete->_Update(CUdpRecv->m_nPage);
    CDelete->show();
}
//设备日志查找动作的响应
void CSERVERCENTER::SlotDeviceFind()
{
    CFind->_Update(CUdpRecv->m_nPage);
    CFind->show();
}
//设备日志删除动作的响应
void CSERVERCENTER::SlotDeviceDelete()
{
    CDelete->_Update(CUdpRecv->m_nPage);
    CDelete->show();
}
//系统日志查找动作的响应
void CSERVERCENTER::SlotSystemFind()
{
    CFind->_Update(CUdpRecv->m_nPage);
    CFind->show();
}
//系统日志删除动作的响应
void CSERVERCENTER::SlotSystemDelete()
{
    CDelete->_Update(CUdpRecv->m_nPage);
    CDelete->show();
}
//操作员管理修改动作的响应
void CSERVERCENTER::SlotUserUpdate()
{
    int ti = ui->QUser->currentRow();
    if(ti >= 0)
    {
        QString strName = ui->QUser->item(ti, 2)->text();
        QString strUser = ui->QUser->item(ti, 1)->text();
        int nFlag = 2;
        CUser->_Update(strName,strUser,nFlag,m_sUser.nGrade);
        CUser->show();
    }
    else
    {
        QMessageBox::information(this,tr("提示"),tr("请选中目标"));
    }
}
//操作员管理删除动作的响应
void CSERVERCENTER::SlotUserDelete()
{
    int ti = ui->QUser->currentRow();
    if(ti >= 0)
    {
    QString strName = ui->QUser->item(ti, 2)->text();
    QString strUser = ui->QUser->item(ti, 1)->text();
    QString strGrade = ui->QUser->item(ti,3)->text();
    if(m_sUser.nGrade == 2)
    {
        if(strGrade == "管理员")
        {
            QMessageBox::information(this,tr("提示"),tr("没有权限"));
            return;
        }
    }else if(m_sUser.nGrade == 1){
        if(strUser == m_sUser.strUser)
        {
            QMessageBox::information(this,tr("提示"),tr("没有权限"));
            return;
        }
    }
    switch( QMessageBox::question( this, tr("提示"),"确定要删除用户:"+strUser,tr("Yes"), tr("No"),0, 1 ) )
  {
  case 0:
      if(CUdpRecv->m_Mysql._DeleteUser(strName,strUser))
      {
          QMessageBox::information(this,tr("提示"),tr("删除成功"));
          _UpdateUser();
      }
    break;
  default :
      break;
  }
    }else{
        QMessageBox::information(this,tr("提示"),tr("请选中目标"));
    }
}
//操作员管理添加动作的响应
void CSERVERCENTER::SlotUserAdd()
{
    int nFlag = 1;
    CUser->_Update("","",nFlag,m_sUser.nGrade);
    CUser->show();
}
//报警界面确认处理报警的响应
void CSERVERCENTER::SlotDealAlarm(int nAlarmType,QString strStime,QString strAlarmStatu,QString strDealHuman,QString strMessage)
{
    if(m_MediaObject->state() == Phonon::PlayingState)
    m_MediaObject->stop();
    SAlarmDevice sAlarmDevice;
    memset(sAlarmDevice.gcAddr,0,64);
    memset(sAlarmDevice.gcIntraAddr,0,20);
    memset(sAlarmDevice.gcPhone1,0,20);
    memset(sAlarmDevice.gcName,0,64);
    memset(sAlarmDevice.gcIpAddr,0,20);
    memset(sAlarmDevice.gcMacAddr,0,20);
    memset(sAlarmDevice.gcStime,0,20);
    memset(sAlarmDevice.gcEtime,0,20);
    memset(sAlarmDevice.gcType,0,20);
    QByteArray byte;
    byte = strStime.toUtf8();
    memcpy(sAlarmDevice.gcStime,byte.data(),byte.size());

    if(CLIST::FindItemAlarm(sAlarmDevice,CUdpRecv->m_AlarmItem))//是否实时的报警
    {
        CUdpRecv->m_AlarmItem = CLIST::DeleteItemAlarm(&sAlarmDevice,&(CUdpRecv->m_AlarmItem));
        CUdpRecv->m_nAlarm--;
        CUdpRecv->_DealAlarm(sAlarmDevice.gcIntraAddr,sAlarmDevice.nAlarmType,QString(sAlarmDevice.gcIpAddr));
        _UpdateAlarm(CUdpRecv->m_AlarmItem);
       //查找链表
        int isNULL = CLIST::AddrFindAlarmItem(sAlarmDevice,CUdpRecv->m_AlarmItem);
        //是否更新实时表
        if(isNULL == 1)
        {
            CUdpRecv->m_TenementItem = CLIST::AlarmUpdateTenementItem(sAlarmDevice,CUdpRecv->m_TenementItem);
            _UpdateTenement(CUdpRecv->m_TenementItem);
        }
    }
    if(CUdpRecv->m_nAlarm == 0)
    {
        QFont ft;
        ft.setPointSize(10);
        ui->QlblAlarm->setFont(ft);
        //设置颜色
        QPalette pa;
        pa.setColor(QPalette::WindowText,Qt::black);
        ui->QlblAlarm->setPalette(pa);
    }
    ui->QlblAlarm->setText(QString::number(CUdpRecv->m_nAlarm));

    //数据库更新
    CUdpRecv->m_Mysql._UpdateDealAlarm(sAlarmDevice,strAlarmStatu,strDealHuman,strMessage);
//    CUdpRecv

//    _UpdateAlarmRecord();
    _UpdatePage(CUdpRecv->m_nPage);
}
//常用电话界面的确定响应
void CSERVERCENTER::SlotPhone()
{
    _UpdatePhone();
}
//报修类型界面的确定响应
void CSERVERCENTER::SlotRepairs()
{
    _UpdateRepairs();
}
//修改报修类型动作响应
void CSERVERCENTER::SlotUpdateRepairs()
{
    int ti = ui->QRepairs->currentRow();
    if(ti >= 0)
    {
        QString str = ui->QRepairs->item(ti, 0)->text();
        int nFlag = 2;
        CRepairs->_Update(str,nFlag);
        CRepairs->show();
    }
    else
    {
        QMessageBox::information(this,tr("提示"),tr("请选中目标"));
    }
}
//删除报修类型动作响应
void CSERVERCENTER::SlotDeleteRepairs()
{
    int ti = ui->QRepairs->currentRow();
    if(ti >= 0)
    {
        QString str = ui->QRepairs->item(ti, 0)->text();
        switch( QMessageBox::question( this, tr("提示"),"确定要删除报修类型:"+str,tr("Yes"), tr("No"),0, 1 ) )
        {
            case 0:
                if(CUdpRecv->m_Mysql._DeleteRepairs(str))
                {
                    QMessageBox::information( this, tr("提示"),tr("删除成功"));
                    _UpdateRepairs();
                }
                break;
          default :
              break;
        }
    }
    else
    {
        QMessageBox::information(this,tr("提示"),tr("请选中目标"));
    }
}
//添加报修类型动作的响应
void CSERVERCENTER::SlotAddRepairs()
{
    int ti = ui->QRepairs->rowCount();
    if(ti < 10)
    {
    CRepairs->_Update("",1);
    CRepairs->show();
    }else{
        QMessageBox::information(this,tr("提示"),tr("最大数目为10，请删除后再添加"));
    }
}
//下发报修类型的动作响应
void CSERVERCENTER::SlotDownRepairs()
{
    char p[1024] = {'0'};
    QString str;
    int nAllRepairs = ui->QRepairs->rowCount();
    if(nAllRepairs > 0)
    {
        switch( QMessageBox::question( this, tr("提示"),"确定要下发全部报修类型",tr("Yes"), tr("No"),0, 1 ) )
      {
      case 0:
    for(int i = 0;i < nAllRepairs;i++)
    {
        str.clear();
        str = ui->QRepairs->item(i, 0)->text();
//        char *p1 = new char [24];
//        memset(p1,0,24);
//        p1 = str.toUtf8().data();
        memcpy(p+i*24,str.toUtf8().data(),str.toUtf8().size());
    }
    CUdpRecv->_SendRepairs(nAllRepairs,p);
    QMessageBox::information(this,tr("提示"),tr("下发完成"));
      break;
      default :
          break;
      }
    }else{
        QMessageBox::information(this,tr("提示"),tr("下发类型数为0，请先添加报修类型再下发"));
    }
}
//修改常用电话动作的响应
void CSERVERCENTER::SlotUpdatePhone()
{
    int ti = ui->QPhone->currentRow();
    if(ti >= 0)
    {
    QString str = ui->QPhone->item(ti, 0)->text();
    QString str1 = ui->QPhone->item(ti, 1)->text();
    QString str2 = ui->QPhone->item(ti, 2)->text();
    int nFlag = 2;
    CPhone->_Update(str,str1,str2,nFlag);
    CPhone->show();
    }else{
        QMessageBox::information(this,tr("提示"),tr("请选中目标"));
    }
}
//删除常用电话动作响应
void CSERVERCENTER::SlotDeletePhone()
{
    int ti = ui->QPhone->currentRow();
    if(ti >= 0)
    {
    QString str = ui->QPhone->item(ti, 0)->text();
    QString str1 = ui->QPhone->item(ti, 1)->text();
    QString str2 = ui->QPhone->item(ti, 2)->text();

    switch( QMessageBox::question( this, tr("提示"),"确定要删除联系人:"+str1,tr("Yes"), tr("No"),0, 1 ) )
  {
  case 0:
      if(CUdpRecv->m_Mysql._DeletePhone(str,str1,str2))
      {
          QMessageBox::information( this, tr("提示"),tr("删除成功"));
          _UpdatePhone();
      }
    break;
  default :
      break;
  }

    }else{
        QMessageBox::information(this,tr("提示"),tr("请选中目标"));
    }
}
//添加常用电话动作响应
void CSERVERCENTER::SlotAddPhone()
{
    int ti = ui->QPhone->rowCount();
    if(ti < 5)
    {
    CPhone->_Update("","","",1);
    CPhone->show();
    }else{
        QMessageBox::information(this,tr("提示"),tr("最大数目为5，请删除后再添加"));
    }
}

//下发常用电话动作响应
void CSERVERCENTER::SlotDownPhone()
{
    char p[1024] = {'0'};
    QString str,str1,str2;
    int nAllPhone = ui->QPhone->rowCount();
    if(nAllPhone > 0)
    {
        switch( QMessageBox::question( this, tr("提示"),"确定要下发全部常用电话",tr("Yes"), tr("No"),0, 1 ) )
      {
      case 0:
    for(int i = 0;i < nAllPhone;i++)
    {
        str.clear();
        str = ui->QPhone->item(i, 0)->text();
        str1 = ui->QPhone->item(i, 1)->text();
        str2 = ui->QPhone->item(i, 2)->text();
//        char *p1 = new char [36];
//        memset(p1,0,36);
//        p1 = str.toUtf8().data();
//        memcpy(p+i*72,p1,strlen(p1));
        memcpy(p+i*72,str.toUtf8().data(),str.toUtf8().size());

//        char *p2 = new char [24];
//        memset(p2,0,24);
//        p2 = str1.toUtf8().data();
//        memcpy(p+i*72+36,p2,strlen(p2));
         memcpy(p+i*72+36,str1.toUtf8().data(),str1.toUtf8().size());

//        char *p3 = new char [12];
//        memset(p3,0,12);
//        p3 = str2.toUtf8().data();
//        memcpy(p+i*72+60,p3,strlen(p3));
          memcpy(p+i*72+60,str2.toUtf8().data(),str2.toUtf8().size());
    }
    CUdpRecv->_SendPhone(nAllPhone,p);
    QMessageBox::information(this,tr("提示"),tr("下发完成"));
    break;
      default :
          break;
      }
      }else{
        QMessageBox::information(this,tr("提示"),tr("下发类型数为0，请先添加报修类型再下发"));
    }
}

//接收到初始化密码的回复
void CSERVERCENTER::SlotRecvInitPass(QString strAddr,int nFlag)
{
    if(nFlag == 1)
    {
        strAddr = strAddr + "初始化工程密码成功";
    }
    else
    {
        strAddr = strAddr + "初始化开门密码成功";
    }
    QMessageBox::information(this,tr("提示"),strAddr);
}
//接收到写设备地址的回复
void CSERVERCENTER::SlotUpdateAddr()
{
    CUpdate->_UpdateAddr();
}
//收到读设备地址的确认
void CSERVERCENTER::SlotRead(char *pAddr,QString strIp)
{
    CUdpRecv->_SendRead(pAddr,strIp);
}

//呼叫记录
void CSERVERCENTER::SlotUpdateCallRecord()
{
    if(CUdpRecv->m_nPage == 14)
    {
        _UpdatePage(CUdpRecv->m_nPage);     //呼叫记录
    }
    if(CUdpRecv->m_nPage == 1)
    {
        _UpdateCallNow();       //住户设备
    }
}

void CSERVERCENTER::SlotSendLamp(char *buf,QString strIp)
{
    CUdpRecv->_SendLamp(buf,strIp);
}
//写地址时修改mac地址的回复
void CSERVERCENTER::SlotUpdateMac()
{
    qDebug()<<"index:"<<m_nIndex;
    if(m_nFindDevice == 1)
        return;
    switch(CUdpRecv->m_nPage)
    {
        case 1:
            switch(m_nIndex)
            {
                case 0:
                    _UpdateTenement(CUdpRecv->m_TenementItem);
                    break;
                case 1:
                    _TenementOnline(CUdpRecv->m_TenementItem);
                    break;
                case 2:
                    _TenementNotOnline(CUdpRecv->m_TenementItem);
                    break;
                case 3:
                    _TenementIndoor(CUdpRecv->m_TenementItem);
                    break;
                case 4:
                    _TenementVillaIndoor(CUdpRecv->m_TenementItem);
                    break;
                default :
                    break;
            }
            break;
        case 2:
            switch(m_nIndex)
            {
                case 0:
                    _UpdateMiddle(CUdpRecv->m_MiddleItem);
                    break;
                case 1:
                    _MiddleOnline(CUdpRecv->m_MiddleItem);
                    break;
                case 2:
                    _MiddleNotOnline(CUdpRecv->m_MiddleItem);
                    break;
                case 3:
                    _MiddleM(CUdpRecv->m_MiddleItem);
                    break;
                case 4:
                    _MiddleW(CUdpRecv->m_MiddleItem);
                    break;
                case 5:
                    _MiddleZ(CUdpRecv->m_MiddleItem);
                    break;
                case 6:
                    _MiddleP(CUdpRecv->m_MiddleItem);
                    break;
                case 7:
                    _MiddleH(CUdpRecv->m_MiddleItem);
                    break;
                default :
                    break;
            }
            break;
        default :
            break;
    }
}


//收到写设备地址的回复
void CSERVERCENTER::SlotRecvWrite(unsigned char *buf,QString strIp)
{
    //CUpdate->_WriteUpdate(buf,strIp);
}
//收到读设备地址的回复
void CSERVERCENTER::SlotRecvRead(unsigned char *buf,QString strIp)
{
    CUpdate->_ReadUpdate(buf,strIp);
}
//收到读设备地址的回复A8
void CSERVERCENTER::SlotRecvRead_A8(unsigned char *buf,QString strIp)
{
    CUpdate->_ReadUpdateA8(buf, strIp);
}

//收到写设备地址的确认
void CSERVERCENTER::SlotWrite(char *pBuf,QString strIp)
{
    CUdpRecv->_SendWrite(pBuf,strIp);
}
//锁定动作响应
void CSERVERCENTER::SlotUpdateTreeLock()
{
    CUdpRecv->m_Mysql._InsertSystem(2,m_sUser.strUser,m_sUser.strName);
    CLock->_Update(m_sUser.strUser,m_sUser.strName);
    TrayIcon->hide();
    CLock->show();
}
//锁定界面解除锁定
 void CSERVERCENTER::SlotUnLock()
 {
     TrayIcon->show();
 }
//注销动作响应
void CSERVERCENTER::SlotUpdateTreeLogout()
{
    CUdpRecv->m_Mysql._InsertSystem(5,m_sUser.strUser,m_sUser.strName);
    emit SigLoading();
    this->hide();
    TrayIcon->hide();
}
//帮助动作响应
void CSERVERCENTER::SlotUpdateTreeHelp()
{
    QString strPath = "用户手册.doc";//这里填写你文件的路径，有中文的话可能需要编码转换
    QDesktopServices bs;            //放在当前路径下的文件夹里
    if(bs.openUrl(QUrl(strPath)))
    {
        return;
    }
    else
    {
        strPath = "/用户手册.doc";
        bs.openUrl(QUrl(strPath));
    }
}

//重启动作响应
void CSERVERCENTER::SlotReset()
{
    int i = ui->QTenement->currentRow();
    if(i >= 0)
    {
        QString strIp = ui->QTenement->item(i, 7)->text();
        QString strAddr = ui->QTenement->item(i, 1)->text();
        QString strType = ui->QTenement->item(i, 11)->text();
        CUdpRecv->_Reset(strIp,strAddr,strType);
    }
    else
    {
        QMessageBox::information(this,tr("提示"),tr("请选中目标"));
    }
}
//修改设备地址动作响应
void CSERVERCENTER::SlotUpdate()
{
    switch(CUdpRecv->m_nPage)
    {
        case 1:
        {
            int i = ui->QTenement->currentRow();
            //QTableWidgetItem * item = ui->QTenement->currentItem();
            if(i >= 0)
            {
                QString strIp = ui->QTenement->item(i, 7)->text();
                QString strAddr = ui->QTenement->item(i, 1)->text();
                QString strType = ui->QTenement->item(i, 11)->text();
                CUpdate->_Updata(strIp,strAddr,strType);
                CUpdate->show();
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
            break;
        case 2:
        {
            //QTableWidgetItem * item = ui->QMiddle->currentItem();
            int i = ui->QMiddle->currentRow();
            if(i >= 0)
            {
                QString strIp = ui->QMiddle->item(i, 4)->text();
                QString strAddr = ui->QMiddle->item(i, 1)->text();
                QString strType = ui->QMiddle->item(i, 6)->text();
                CUpdate->_Updata(strIp,strAddr,strType);
                CUpdate->show();
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("请选中目标"));
            }
        }
            break;
        default :
            break;
    }
}
//下载文件结果响应
void CSERVERCENTER::Slotbtn(int Flag)
{
    if(Flag == 1)
    {
        ui->QpbtnBrowse->setEnabled(true);
        ui->QpbtnSendM->setEnabled(true);
        QMessageBox::information(this,tr("提示"),tr("全部下载完成"));
    }
    else if(Flag == 0)
    {
        ui->QpbtnBrowse->setEnabled(false);
        ui->QpbtnSendM->setEnabled(false);
    }
    else if(Flag == 2)
    {
        ui->QpbtnSendT->setEnabled(true);
        QMessageBox::information(this,tr("提示"),tr("全部下载完成"));
    }
    else if(Flag == 3)
    {
        ui->QpbtnSendT->setEnabled(true);
    }
}
//下载视频图片数量响应
void CSERVERCENTER::SlotNewsDonw(int Id,int All,int Finfish)
{
    QString str = "文件数:" + QString::number(All) +"成功发送:" + QString::number(Finfish);
    QTableWidgetItem *tabledmItem8 = new QTableWidgetItem(str);
    tabledmItem8->setTextAlignment(Qt::AlignCenter);
    ui->QNews->setItem(Id, 6, tabledmItem8);
}

//菜单栏动作响应 begin
void CSERVERCENTER::SlotUpdateTreeRepairsmessage()
{
    ui->QlblAllName->hide();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("物业信息",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 11;     //page_12
    _UpdateRepairs();
    _UpdatePhone();
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::SlotUpdateTreeAbout()//关于
{
    CAbout->show();
}

void CSERVERCENTER::SlotUpdateTreeManage()
{
    ui->QlblAllName->show();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("操作员管理",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 6;      //page_7
    _UpdateUser();
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::SlotUpdateTreeCard()
{
    ui->QlblAllName->show();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("刷卡日志",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 9;      //page_10
    _UpdateCardRecord();
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::SlotUpdateTreeDevice()
{
    ui->QlblAllName->show();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("设备日志",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 8;      //page_9
    _UpdateDevice();
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::SlotUpdateTreeSystem()
{
    ui->QlblAllName->show();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("系统日志",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 7;      //page_8
    _UpdateSystem();
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::SlotUpdateTreeAlarm()
{
    ui->QlblAllName->show();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("报警记录",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 3;      //page_4
    _UpdateAlarmRecord();
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::SlotUpdateTreeRepairs()
{
    ui->QlblAllName->show();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("报修记录",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 4;      //page_5
    _UpdateRepairsRecord();
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::SlotUpdateTreeNews()
{
    ui->QlblAllName->show();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("重要新闻",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 12;     //page_13
    ui->QlblAll->setText(QString::number(CUdpRecv->m_nAllMiddle));
    _UpdateNews(CUdpRecv->m_MiddleItem);
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::SlotUpdateTreeIC()
{
    ui->QlblAllName->hide();
    ui->QlblAll->clear();
    if(m_sUser.protocol == 0)
    {
        CUdpRecv->m_nPage = 13;
        ui->IC_stackedWidget->setCurrentIndex(0);
        _SetTableWidgetSatus(1);
        ui->QpbtnAdd->setEnabled(false);
        _UpdateEmpowerIndoor(CUdpRecv->m_TenementItem);
    }
    else
    {
        CUdpRecv->m_nPage = 5;
        _ClearItem();
        _UpdateIcCard();
        _UpdateCardMessage();
    }

    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
    ui->QlblAllName->hide();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("IC卡信息",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
//    CUdpRecv->m_nPage = 5;      //page_6
//    _UpdateIcCard();
//    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::SlotUpdateTreeTenement()
{
    m_nFindDevice == 0;
    ui->QlblAllName->show();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("住户设备",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 1;      //page_2
    ui->QchkTenement->setCurrentIndex(0);
    ui->QlblAll->setText(QString::number(CUdpRecv->m_nAllTenement));
    ui->QlblOnlineTenement->setText(QString::number(CUdpRecv->m_nAllTenementOnline));
    _UpdateTenement(CUdpRecv->m_TenementItem);
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::SlotUpdateTreeMiddle()
{
    m_nFindDevice == 0;
    ui->QlblAllName->show();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("中间设备",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 2;      //page_3
    ui->QchkMiddle->setCurrentIndex(0);
    ui->QlblAll->setText(QString::number(CUdpRecv->m_nAllMiddle));
    ui->QlblOnlineMiddle->setText(QString::number(CUdpRecv->m_nAllMiddleOnline));
    _UpdateMiddle(CUdpRecv->m_MiddleItem);
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

void CSERVERCENTER::SlotUpdateTreeQuit()
{
    switch( QMessageBox::question( this, tr("提示"),tr("确定退出"),tr("Yes"), tr("No"),0, 1 ) )
  {
  case 0:
    CUdpRecv->m_Mysql._InsertSystem(3,m_sUser.strUser,m_sUser.strName);
    emit Sigquit();
    break;
  default :
      break;
  }
}

void CSERVERCENTER::SlotUpdateTreeMessage()
{
    ui->QlblAllName->show();
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("普通信息",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    CUdpRecv->m_nPage = 10;     //page_11
    ui->QlblAll->setText(QString::number(CUdpRecv->m_nAllTenement));
    _UpdateMessage(CUdpRecv->m_TenementItem);
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}

//上一页
void CSERVERCENTER::SlotPreviousPage()
{
    if(m_nPage == 0)
    {
        QMessageBox::information(this,tr("提示"),"已经是最前一页");
    }
    else
    {
        m_nPage--;
        _UpdatePage(CUdpRecv->m_nPage);
    }
}

//下一页
void CSERVERCENTER::SlotNextPage()
{
    int nAllPage = (m_nCount % PAGE_MAXLEN)?(m_nCount / PAGE_MAXLEN + 1):(m_nCount / PAGE_MAXLEN);
    if(m_nPage == (nAllPage - 1))
    {
        QMessageBox::information(this,tr("提示"),"已经是最后一页");
    }
    else
    {
        m_nPage++;
        _UpdatePage(CUdpRecv->m_nPage);
    }
}

//呼叫记录
void CSERVERCENTER::SlotUpdateTreeCallRecord()
{
    QList<QTreeWidgetItem*> list = ui->QTree->findItems("呼叫记录",Qt::MatchWildcard |Qt::MatchRecursive);
    foreach (QTreeWidgetItem *b8,list)
    {
        ui->QTree->setCurrentItem(b8);
    }
    QDateTime dateTime=QDateTime::currentDateTime();
    ui->QdateETime->setDateTime(dateTime);//setDate(dateTime.date());
    dateTime = dateTime.addDays(-5);
    ui->QdateSTime->setDateTime(dateTime);//setDate(dateTime.date());
    ui->QrbtnTime->setChecked(false);
    ui->QdateETime->setEnabled(false);
    ui->QdateSTime->setEnabled(false);
    ui->QchkType->setCurrentIndex(0);

    CUdpRecv->m_nPage = 14;     //page_14
    _UpdateCallRecord();
    ui->stackedWidget->setCurrentIndex(CUdpRecv->m_nPage);
}
//菜单栏动作响应 end

//获取实时天气数据响应
void CSERVERCENTER::SlotUpdateWeatherNew()
{
    CWeather->_Update(m_sUser.strCity,CUdpRecv->sWeather.nMinTemp,CUdpRecv->sWeather.nMaxTemp,CUdpRecv->sWeather.nWeather);
}
void CSERVERCENTER::SlotUpdateDBSet()
{
    if(m_sUser.nGrade == 1)
    {
        CSetOdbc->show();
    }
}

//设置天气动作
void CSERVERCENTER::SlotUpdateWeather()
{
    CWeather->_Update(m_sUser.strCity,CUdpRecv->sWeather.nMinTemp,CUdpRecv->sWeather.nMaxTemp,CUdpRecv->sWeather.nWeather);
    CWeather->show();
    connect(CWeather,SIGNAL(SigUpdateCity(QString,QString)),this,SLOT(SlotUpdateCity(QString,QString)));
    connect(CWeather,SIGNAL(SigUpdateWeather(int,int,int)),this,SLOT(SlotUpdateWeather(int,int,int)));
}

//列表焦点设置
void CSERVERCENTER::_FocusChange()
{
    if(!ui->QAlarm->hasFocus())
    {
        ui->QAlarm->setCurrentItem(NULL);
    }
    switch(CUdpRecv->m_nPage)
    {
        case 1:
            if(!ui->QTenement->hasFocus())
            {
                ui->QTenement->setCurrentItem(NULL);
            }
            break;
        case 2:
            if(!ui->QMiddle->hasFocus())
            {
                 ui->QMiddle->setCurrentItem(NULL);
            }
            break;
        case 3:
            if(!ui->QAlarmRecord->hasFocus())
            {
                 ui->QAlarmRecord->setCurrentItem(NULL);
            }
            break;
        case 4:
            if(!ui->QRepairsRecord->hasFocus())
            {
                 ui->QRepairsRecord->setCurrentItem(NULL);
            }
            break;
        case 6:
            if(!ui->QUser->hasFocus())
            {
                 ui->QUser->setCurrentItem(NULL);
            }
            break;
        case 7:
            if(!ui->QSystem->hasFocus())
            {
                 ui->QSystem->setCurrentItem(NULL);
            }
            break;
        case 8:
            if(!ui->QDevice->hasFocus())
            {
                 ui->QDevice->setCurrentItem(NULL);
            }
            break;
        case 9:
            if(!ui->QCardRecord->hasFocus())
            {
                 ui->QCardRecord->setCurrentItem(NULL);
            }
            break;
        case 10:
            if(!ui->QMessage->hasFocus())
            {
                 ui->QMessage->setCurrentItem(NULL);
            }
            break;
        case 11:
            if(!ui->QRepairs->hasFocus())
            {
                 ui->QRepairs->setCurrentItem(NULL);
            }
            if(!ui->QPhone->hasFocus())
            {
                 ui->QPhone->setCurrentItem(NULL);
            }
            break;
        case 12:
            if(!ui->QNews->hasFocus())
            {
                 ui->QNews->setCurrentItem(NULL);
            }
            break;
    }
}

//响应可用有效期查询
void CSERVERCENTER::SlotDateTime(int nFlag,int nDay)
{
    QString str = "系统运行"+QString::number(nDay)+"天，请联系厂家进行维护，\n安恩达科技有限公司";
    switch(nFlag)
    {
        case 1:
            QMessageBox::information(this,tr("提示4"),tr("系统维护，请联系厂家进行维护，\n 安恩达科技有限公司"));
            CUdpRecv->m_Mysql._InsertSystem(3,m_sUser.strUser,m_sUser.strName);
            emit Sigquit();
            break;
        case 2:
            QMessageBox::information(this,tr("温馨提示"),str);
            break;
        case 3:
            QMessageBox::information(this,tr("郑重提示"),str);
            break;
        case 4:
            QMessageBox::information(this,tr("提示5"),tr("系统维护，请联系厂家进行维护，\n 安恩达科技有限公司"));
            CUdpRecv->m_Mysql._InsertSystem(3,m_sUser.strUser,m_sUser.strName);
            emit Sigquit();
            break;
        default :
            break;
    }
}

//报警列表闪烁
void CSERVERCENTER::_ColorChange()
{
    if(ui->QAlarm->rowCount())
    {
        if(ui->QAlarm->columnCount())
        {
            for(int i = 0;i < ui->QAlarm->rowCount(); i++)
            {
                for(int j = 0 ;j < ui->QAlarm->columnCount();j++)
                {
                    QTableWidgetSelectionRange range(0,0,i,j);
                    QTableWidgetItem *item = ui->QAlarm->item(i, j);
                    if(m_nColor_state)
                    {
                       item->setBackgroundColor(QColor(255,0,0));
                    }
                    else
                        item->setBackgroundColor(QColor(0,125,0,0));
                }
            }
            if(m_nColor_state)
            {
                m_nColor_state = 0;
            }
            else
                m_nColor_state = 1;
        }
    }
}

//响应UDP端口打开失败
void CSERVERCENTER::SlotUdpOpenFail()
{
    m_nUdp = 1;
    QMessageBox::warning(NULL,tr("提示"),tr("udp端口打开失败，请检查后重启"));
    CUdpRecv->m_Mysql._InsertSystem(3,m_sUser.strUser,m_sUser.strName);
    emit Sigquit();
}
//初始化中间设备列表
void CSERVERCENTER::SlotInitMiddle()
{
    ui->QlblAll->setText(QString::number(CUdpRecv->m_nAllMiddle));
    if(CUdpRecv->m_nPage == 2 )     //中间设备
    {
        if(m_nFindDevice == 1)
            return;
        switch(m_nIndex)
        {
            case 0:
                _UpdateMiddle(CUdpRecv->m_MiddleItem);
                break;
            case 1:
                _MiddleOnline(CUdpRecv->m_MiddleItem);
                break;
            case 2:
                _MiddleNotOnline(CUdpRecv->m_MiddleItem);
                break;
            case 3:
                _MiddleM(CUdpRecv->m_MiddleItem);
                break;
            case 4:
                _MiddleW(CUdpRecv->m_MiddleItem);
                break;
            case 5:
                _MiddleZ(CUdpRecv->m_MiddleItem);
                break;
            case 6:
                _MiddleP(CUdpRecv->m_MiddleItem);
                break;
            case 7:
                _MiddleH(CUdpRecv->m_MiddleItem);
                break;
            default :
                break;
        }
    }
    else if(CUdpRecv->m_nPage == 12)    //重要新闻
    {
        _UpdateNews(CUdpRecv->m_MiddleItem);
    }
    else if(CUdpRecv->m_nPage == 8)
    {
        _UpdatePage(CUdpRecv->m_nPage);
    }
}
//修改中间设备数据
void CSERVERCENTER::SlotUpdateMiddle(QVariant data)
{
    if(CUdpRecv->m_nPage == 2)
    {
        if(m_nFindDevice == 1)
            return;
        switch(m_nIndex)
        {
            case 0:
                _OneUpdateMiddle(data);
                break;
            case 1:
                _MiddleOnline(CUdpRecv->m_MiddleItem);
                break;
            case 2:
                _MiddleNotOnline(CUdpRecv->m_MiddleItem);
                break;

            case 3:
                _MiddleM(CUdpRecv->m_MiddleItem);
                break;
            case 4:
                _MiddleW(CUdpRecv->m_MiddleItem);
                break;
            case 5:
                _MiddleZ(CUdpRecv->m_MiddleItem);
                break;
            case 6:
                _MiddleP(CUdpRecv->m_MiddleItem);
                break;
            case 7:
                _MiddleH(CUdpRecv->m_MiddleItem);
                break;
            default :
                break;
        }

    }
    else if(CUdpRecv->m_nPage == 12)
    {
        _UpdateNews(CUdpRecv->m_MiddleItem);
    }
}
//初始化住户设备列表
void CSERVERCENTER::SlotInitTenement()
{
    ui->QlblAll->setText(QString::number(CUdpRecv->m_nAllTenement));
    if(CUdpRecv->m_nPage == 1)
    {
        if(m_nFindDevice == 1)
            return;
        switch(m_nIndex)
        {
            case 0:
                _UpdateTenement(CUdpRecv->m_TenementItem);
                break;
            case 1:
                _TenementOnline(CUdpRecv->m_TenementItem);
                break;
            case 2:
                _TenementNotOnline(CUdpRecv->m_TenementItem);
                break;
            case 3:
                _TenementIndoor(CUdpRecv->m_TenementItem);
                break;
            case 4:
                _TenementVillaIndoor(CUdpRecv->m_TenementItem);
                break;
            default :
                break;
        }
    }
    else if(CUdpRecv->m_nPage == 10)
    {
        _UpdateMessage(CUdpRecv->m_TenementItem);
    }
    else if(CUdpRecv->m_nPage == 8)
    {
        _UpdatePage(CUdpRecv->m_nPage);
    }
}
//修改住户设备列表
void CSERVERCENTER::_OneUpdateTenement(QVariant dataVar)
{
    STenementDevice data;
    data = dataVar.value<STenementDevice>();

    int ti = data.nId ;

    QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(data.gcAddr));
    tabledmItem1->setTextAlignment(Qt::AlignCenter);
    ui->QTenement->setItem(ti, 1, tabledmItem1);

    QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(data.gcAddrExplain));
    tabledmItem2->setTextAlignment(Qt::AlignCenter);
    ui->QTenement->setItem(ti, 2, tabledmItem2);

    QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(data.gcName));
    tabledmItem3->setTextAlignment(Qt::AlignCenter);
    ui->QTenement->setItem(ti, 3, tabledmItem3);

    QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(data.gcPhone1));
    tabledmItem4->setTextAlignment(Qt::AlignCenter);
    ui->QTenement->setItem(ti, 4, tabledmItem4);

    QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(data.gcPhone2));
    tabledmItem5->setTextAlignment(Qt::AlignCenter);
    ui->QTenement->setItem(ti, 5, tabledmItem5);

    QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(data.gcMacAddr));
    tabledmItem6->setTextAlignment(Qt::AlignCenter);
    ui->QTenement->setItem(ti, 6, tabledmItem6);

    QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(data.gcIpAddr));
    tabledmItem7->setTextAlignment(Qt::AlignCenter);
    ui->QTenement->setItem(ti, 7, tabledmItem7);

    if(data.nState <= 0)
    {
        CUdpRecv->m_nAllTenementOnline ++;
        ui->QlblOnlineTenement->setText(QString::number(CUdpRecv->m_nAllTenementOnline));
    }
        QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("已连接");
        tabledmItem8->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 8, tabledmItem8);


    if(data.nFenceState == 0)
    {
        QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("撤防");
        tabledmItem9->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 9, tabledmItem9);
    }
    else
    {
//        qDebug()<<"布防："<<data.nFenceState;
        switch(data.nFenceState)
        {
        case 1:
        {
            QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("外出布防");
            tabledmItem9->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 9, tabledmItem9);
            break;
        }
        case 2:
        {
            QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("在家布防");
            tabledmItem9->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 9, tabledmItem9);
            break;
        }
        default :
        {
            QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("未知布防");
            tabledmItem9->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 9, tabledmItem9);
            break;
        }
        }
    }

    QTableWidgetItem *tabledmItem11 = new QTableWidgetItem(QString(data.gcType));
    tabledmItem11->setTextAlignment(Qt::AlignCenter);
    ui->QTenement->setItem(ti, 11, tabledmItem11);

    if(data.nAlarmState == 0)
    {
        QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("无报警");
        tabledmItem10->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 10, tabledmItem10);
        for(int j = 0;j < 12;j++)
        {
            QTableWidgetItem *item = ui->QTenement->item(ti, j);
            item->setBackgroundColor(QColor(0,125,0,0));
        }
    }else{
        QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("报警");
        tabledmItem10->setTextAlignment(Qt::AlignCenter);
        ui->QTenement->setItem(ti, 10, tabledmItem10);
        for(int j = 0;j < 12;j++)
        {
            QTableWidgetItem *item = ui->QTenement->item(ti, j);
            item->setBackgroundColor(QColor(255,0,0));
        }
    }


        for(int j = 0;j < 12;j++)
        {
            QTableWidgetItem *item = ui->QTenement->item(ti, j);
            item->setTextColor(Qt::blue);
        }

}

//响应单个住户设备信息的修改
void CSERVERCENTER::SlotUpdateTenement(QVariant data)
{
//    qDebug()<<"index:"<<m_nIndex;
    if(CUdpRecv->m_nPage == 1)
    {
        if(m_nFindDevice == 1)
            return;
        switch(m_nIndex)
        {
            case 0:
                _OneUpdateTenement(data);
                break;
            case 1:
                _TenementOnline(CUdpRecv->m_TenementItem);
                break;
            case 2:
                _TenementNotOnline(CUdpRecv->m_TenementItem);
                break;
            case 3:
                _TenementIndoor(CUdpRecv->m_TenementItem);
                break;
            case 4:
                _TenementVillaIndoor(CUdpRecv->m_TenementItem);
                break;
            default :
                break;
        }

    }
    else if(CUdpRecv->m_nPage == 10)
    {
        _UpdateMessage(CUdpRecv->m_TenementItem);
    }
    else if(CUdpRecv->m_nPage == 8)
    {
        _UpdatePage(CUdpRecv->m_nPage);
    }
}
//普通信息发送的结果响应
void CSERVERCENTER::SlotSendMessage(int Id,int Result)
{
    if(CUdpRecv->m_nPage == 10)
    {
        if(Result == 0)
        {
            QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("发送失败");
            ui->QMessage->setItem(Id, 7, tabledmItem8);

            for(int j = 0;j < 8;j++)
            {
                QTableWidgetItem *item = ui->QMessage->item(Id, j);
                item->setTextColor(QColor(0,0,0));
            }
        }
        else
        {
            QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("发送成功");
            ui->QMessage->setItem(Id, 7, tabledmItem8);
            for(int j = 0;j < 8;j++)
            {
                QTableWidgetItem *item = ui->QMessage->item(Id, j);
                item->setTextColor(Qt::blue);
            }
        }
    }
}
//住户设备掉线刷新列表
void CSERVERCENTER::SlotOnlineTenement(int Id,int Alarm)
{
    CUdpRecv->m_nAllTenementOnline--;
    ui->QlblOnlineTenement->setText(QString::number(CUdpRecv->m_nAllTenementOnline));
    if(CUdpRecv->m_nPage == 1)
    {
        if(m_nFindDevice == 1)
            return;
        switch(m_nIndex)
        {
            case 0:
            {
                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("未连接");
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(Id, 8, tabledmItem8);
                if(Alarm)
                {
                    QTableWidgetItem *item = ui->QTenement->item(Id, 8);
                    item->setBackgroundColor(QColor(255,0,0));
                }
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(Id, j);
                    item->setTextColor(QColor(0,0,0));
                }
            }
                break;
            case 1:
                _TenementOnline(CUdpRecv->m_TenementItem);
                break;
            case 2:
                _TenementNotOnline(CUdpRecv->m_TenementItem);

                break;
            case 3:
                _TenementIndoor(CUdpRecv->m_TenementItem);
                break;
            case 4:
                _TenementVillaIndoor(CUdpRecv->m_TenementItem);
                break;
            default :
                break;
        }
    }
    else if(CUdpRecv->m_nPage == 10)
    {
        QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
        tabledmItem5->setTextAlignment(Qt::AlignCenter);
        ui->QMessage->setItem(Id, 5, tabledmItem5);
        for(int j = 0;j < 7;j++)
        {
            QTableWidgetItem *item = ui->QMessage->item(Id, j);
            item->setTextColor(QColor(0,0,0));
        }
    }
    else if(CUdpRecv->m_nPage == 8)
    {
        _UpdatePage(CUdpRecv->m_nPage);
    }
}
//选择项发生变化——（普通信息 新闻信息 开门权限）
void CSERVERCENTER::SlotCheck(int row,int col)
{
    if(col == 0)
    {
        switch(CUdpRecv->m_nPage)
        {
        case 10:
        {
            if(ui->QMessage->item(row, col)->checkState() == Qt::Checked) //选中
            {
                int i =1;
                _UpdateChange(row,i);
            }
            else
            {
                int i = 0;
                _UpdateChange(row,i);
            }
            break;
        }
        case 12:
        {
            if(ui->QNews->item(row, col)->checkState() == Qt::Checked) //选中
            {
                int i =1;
                _UpdateChange(row,i);
            }
            else
            {
                int i = 0;
                _UpdateChange(row,i);
            }
            break;
        }
        case 5:
        {
            if(ui->QCardMessage->item(row, col)->checkState() == Qt::Checked) //选中
            {
                int i =1;
                _UpdateChange(row,i);
            }
            else
            {
                int i = 0;
                _UpdateChange(row,i);
            }
            break;
        }
        default :
            break;
        }
    }
}

//选中项改变更新链表
void CSERVERCENTER::_UpdateChange(int row,int check)
{
    switch(CUdpRecv->m_nPage)
    {
    case 5:
    {
        NodeCard *q;
        q = m_ItemCard;
        int i = 0;
        while(q->next)
        {
            //qDebug()<<QString(q->data.gcAddr)<<i<<row;
            if(i == row)
            {
                q->data.nNewCheck = check;
                return;
            }
            i++;
            q = q->next;
        }
    }
    break;
    case 10:
    {
        NodeTenement *q;
        q = CUdpRecv->m_TenementItem;
        while(q)
        {
            if(q->data.nId == (unsigned long)row)
            {
                q->data.nCheckedState = check;
                return;
            }
            q = q->next;
        }
    }
    break;
    case 12:
    {
        NodeMiddle *q;
        q = CUdpRecv->m_MiddleItem;
        while(q)
        {
            if(q->data.nId == (unsigned long)row)
            {
                q->data.nCheckedState = check;
                return;
            }
            q = q->next;
        }
    }
        break;
    default :
        break;
    }
}
//状态栏
void CSERVERCENTER::_StatusBar(SUser sUser)
{
//    statusbar = ui->statusbar;
    ui->statusbar->setSizeGripEnabled(false);//右下角三角
    ui->statusbar->setStyleSheet(QString("QStatusBar::item{border: 0px}"));//无外框

    QLabel *statulabel = new QLabel("用户:");
    statulabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    statulabel->setMinimumSize(35,20); //设置标签最小尺寸

    statulabe2 = new QLabel(sUser.strUser);
    statulabe2->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statulabe2->setMinimumSize(60,20); //设置标签最小尺寸

    QLabel *statulabe3 = new QLabel("用户级别:");
    statulabe3->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    statulabe3->setMinimumSize(60,20); //设置标签最小尺寸
    statulabe3->setMinimumSize(statulabel->sizeHint());

    QString grade;
    switch(sUser.nGrade)
    {
        case 1:
            grade.clear();
            grade = "超级管理员";
            break;
        case 2:
            grade.clear();
            grade = "管理员";
            break;
        case 3:
            grade.clear();
            grade = "操作员";
            break;
        default :
            grade.clear();
            grade = "错误";
            break;
    }

    statulabe4 = new QLabel(grade);
    statulabe4->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statulabe4->setMinimumSize(80,20); //设置标签最小尺寸

    QLabel *MySQLLabel = new QLabel("MySQL");
    MySQLLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    MySQLLabel->setMinimumSize(30,20);

//    QString MyIconPath = dir.currentPath() + "/pic/green1.png";
    QString MyIconPath = dir.currentPath() + "/pic/" + MyDis;
    MySQLIcon = new QLabel();
    MySQLIcon->setPixmap(QPixmap(MyIconPath));
    MySQLIcon->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    MySQLIcon->setAttribute(Qt::WA_DeleteOnClose);
    MySQLIcon->setScaledContents(true);
    MySQLIcon->resize(15,15);

    QLabel *MSSQLLabel = new QLabel("MSSQL");
    MSSQLLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    MSSQLLabel->setMinimumSize(50,20);

//    QString MSIconPath = dir.currentPath() + "/pic/green1.png";
    QString MSIconPath = dir.currentPath() + "/pic/" + MsDis;
    MSSQLIcon = new QLabel();
    MSSQLIcon->setPixmap(QPixmap(MSIconPath));
    MSSQLIcon->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    MSSQLIcon->setAttribute(Qt::WA_DeleteOnClose);
    MSSQLIcon->setScaledContents(true);
    MSSQLIcon->resize(15,15);

    QDateTime dateTime = QDateTime::currentDateTime();
    QString time = dateTime.toString("yyyy-MM-dd hh:mm:ss");

    QLabel *statulabe7 = new QLabel(tr("登陆时间:"));
    statulabe7->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    statulabe7->setMinimumSize(80,20); //设置标签最小尺寸

    statulabe8 = new QLabel(time);
    statulabe8->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statulabe8->setMinimumSize(140,20);

    QLabel *statulabe5 = new QLabel(tr("系统时间:"));
    statulabe5->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statulabe5->setMinimumSize(80,20); //设置标签最小尺寸
    statulabe5->setMinimumSize(statulabel->sizeHint());

    statulabe6 = new QLabel(time);
    statulabe6->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statulabe6->setMinimumSize(50,20);

    ui->statusbar->addWidget(statulabel);
    ui->statusbar->addWidget(statulabe2);
    ui->statusbar->addWidget(statulabe3);
    ui->statusbar->addWidget(statulabe4);
    ui->statusbar->addWidget(MySQLLabel);
    ui->statusbar->addWidget(MySQLIcon);
    ui->statusbar->addWidget(MSSQLLabel);
    ui->statusbar->addWidget(MSSQLIcon);
    ui->statusbar->addWidget(statulabe7);
    ui->statusbar->addWidget(statulabe8);
    ui->statusbar->addWidget(statulabe5);
    ui->statusbar->addWidget(statulabe6);
}

//刷新系统时间
void CSERVERCENTER::timerEvent( QTimerEvent *event )
{
    if(event->timerId() == m_nTimerId)
    {
        QDateTime dateTime = QDateTime::currentDateTime();
        QString time = dateTime.toString("yyyy-MM-dd hh:mm:ss");
        statulabe6->setText(time);
    }
}

void CSERVERCENTER::on_QchkTenement_activated(int index)
{
    m_nFindDevice == 0;
    m_nIndex = index;
    switch(index)
    {
        case 0:
            _UpdateTenement(CUdpRecv->m_TenementItem);
            break;
        case 1:
            _TenementOnline(CUdpRecv->m_TenementItem);
            break;
        case 2:
            _TenementNotOnline(CUdpRecv->m_TenementItem);
            break;
        case 3:
            _TenementIndoor(CUdpRecv->m_TenementItem);
            break;
        case 4:
            _TenementVillaIndoor(CUdpRecv->m_TenementItem);
            break;
        default :
            break;
    }
}

//别墅室内机设备列表
void CSERVERCENTER::_TenementVillaIndoor(NodeTenement *ItemTenement)
{
    int ti = 0;
    int tOnline = 0;
    ui->QTenement->setRowCount(ti);
    while(ItemTenement->next != NULL)
    {
        if(ItemTenement->data.gcIntraAddr[0] == 'B')
        {
            ui->QTenement->setRowCount(ti + 1);
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 0, tabledmItem0);

            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem( QString(ItemTenement->data.gcAddr));
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 1, tabledmItem1);

            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemTenement->data.gcAddrExplain));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 2, tabledmItem2);

            QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemTenement->data.gcName));
            tabledmItem3->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 3, tabledmItem3);

            QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone1));
            tabledmItem4->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 4, tabledmItem4);

            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone2));
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 5, tabledmItem5);

            QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemTenement->data.gcMacAddr));
            tabledmItem6->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 6, tabledmItem6);

            QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(ItemTenement->data.gcIpAddr));
            tabledmItem7->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 7, tabledmItem7);

            if(ItemTenement->data.nState <= 0)
            {
                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("未连接");
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 8, tabledmItem8);
            }else{
                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("已连接");
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 8, tabledmItem8);
            }

            if(ItemTenement->data.nFenceState == 0)
            {
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("撤防");
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 9, tabledmItem9);
            }else{
                switch(ItemTenement->data.nFenceState){
                case 1:
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("外出布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                case 2:
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("在家布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                default :
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("未知布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                }
            }

            QTableWidgetItem *tabledmItem11 = new QTableWidgetItem(QString(ItemTenement->data.gcType));
            tabledmItem11->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 11, tabledmItem11);

            if(ItemTenement->data.nAlarmState == 0)
            {
                QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("无报警");
                tabledmItem10->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 10, tabledmItem10);
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setBackgroundColor(QColor(0,125,0,0));
                }
            }else{
                QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("报警");
                tabledmItem10->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 10, tabledmItem10);
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setBackgroundColor(QColor(255,0,0));
                }
            }

            if(ItemTenement->data.nState <= 0)
            {
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setTextColor(QColor(0,0,0));
                }
            }else{
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setTextColor(Qt::blue);
                }
            }
            ti++;
        }

        if(ItemTenement->data.nState > 0)
        {
            tOnline++;
        }
        ItemTenement = ItemTenement->next;
    }
    CUdpRecv->m_nAllTenementOnline = tOnline;
    ui->QlblOnlineTenement->setText(QString::number(CUdpRecv->m_nAllTenementOnline));

}
//室内机设备列表
void CSERVERCENTER::_TenementIndoor(NodeTenement *ItemTenement)
{
    int ti = 0;
    int tOnline = 0;
    ui->QTenement->setRowCount(ti);
    while(ItemTenement->next != NULL)
    {
        if(ItemTenement->data.gcIntraAddr[0] == 'S')
        {
            ui->QTenement->setRowCount(ti + 1);
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 0, tabledmItem0);

            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem( QString(ItemTenement->data.gcAddr));
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 1, tabledmItem1);

            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemTenement->data.gcAddrExplain));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 2, tabledmItem2);

            QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemTenement->data.gcName));
            tabledmItem3->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 3, tabledmItem3);

            QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone1));
            tabledmItem4->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 4, tabledmItem4);

            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone2));
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 5, tabledmItem5);

            QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemTenement->data.gcMacAddr));
            tabledmItem6->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 6, tabledmItem6);

            QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(ItemTenement->data.gcIpAddr));
            tabledmItem7->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 7, tabledmItem7);

            if(ItemTenement->data.nState <= 0)
            {
                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("未连接");
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 8, tabledmItem8);
            }else{
                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("已连接");
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 8, tabledmItem8);
            }

            if(ItemTenement->data.nFenceState == 0)
            {
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("撤防");
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 9, tabledmItem9);
            }else{
                switch(ItemTenement->data.nFenceState){
                case 1:
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("外出布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                case 2:
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("在家布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                default :
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("未知布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                }
            }

            QTableWidgetItem *tabledmItem11 = new QTableWidgetItem(QString(ItemTenement->data.gcType));
            tabledmItem11->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 11, tabledmItem11);

            if(ItemTenement->data.nAlarmState == 0)
            {
                QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("无报警");
                tabledmItem10->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 10, tabledmItem10);
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setBackgroundColor(QColor(0,125,0,0));
                }
            }else{
                QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("报警");
                tabledmItem10->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 10, tabledmItem10);
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setBackgroundColor(QColor(255,0,0));
                }
            }

            if(ItemTenement->data.nState <= 0)
            {
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setTextColor(QColor(0,0,0));
                }
            }else{
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setTextColor(Qt::blue);
                }
            }
            ti++;
        }
        if(ItemTenement->data.nState > 0)
        {
            tOnline++;
        }
        ItemTenement = ItemTenement->next;
    }
    CUdpRecv->m_nAllTenementOnline = tOnline;
    ui->QlblOnlineTenement->setText(QString::number(CUdpRecv->m_nAllTenementOnline));

}

//不在线住户设备列表
void CSERVERCENTER::_TenementNotOnline(NodeTenement *ItemTenement)
{
    int ti = 0;
    int tOnline = 0;
    ui->QTenement->setRowCount(ti);
    while(ItemTenement->next != NULL)
    {
        if(ItemTenement->data.nState <= 0)
        {
            ui->QTenement->setRowCount(ti + 1);
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 0, tabledmItem0);

            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem( QString(ItemTenement->data.gcAddr));
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 1, tabledmItem1);


            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemTenement->data.gcAddrExplain));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 2, tabledmItem2);

            QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemTenement->data.gcName));
            tabledmItem3->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 3, tabledmItem3);

            QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone1));
            tabledmItem4->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 4, tabledmItem4);

            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone2));
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 5, tabledmItem5);

            QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemTenement->data.gcMacAddr));
            tabledmItem6->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 6, tabledmItem6);

            QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(ItemTenement->data.gcIpAddr));
            tabledmItem7->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 7, tabledmItem7);

            if(ItemTenement->data.nState <= 0)
            {
                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("未连接");
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 8, tabledmItem8);
            }else{
                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("已连接");
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 8, tabledmItem8);
            }

            if(ItemTenement->data.nFenceState == 0)
            {
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("撤防");
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 9, tabledmItem9);
            }else{
                switch(ItemTenement->data.nFenceState){
                case 1:
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("外出布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                case 2:
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("在家布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                default :
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("未知布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                }
            }

            QTableWidgetItem *tabledmItem11 = new QTableWidgetItem(QString(ItemTenement->data.gcType));
            tabledmItem11->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 11, tabledmItem11);

            if(ItemTenement->data.nAlarmState == 0)
            {
                QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("无报警");
                tabledmItem10->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 10, tabledmItem10);
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setBackgroundColor(QColor(0,125,0,0));
                }
            }else{
                QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("报警");
                tabledmItem10->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 10, tabledmItem10);
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setBackgroundColor(QColor(255,0,0));
                }
            }

            if(ItemTenement->data.nState <= 0)
            {
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setTextColor(QColor(0,0,0));
                }
            }else{
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setTextColor(Qt::blue);
                }
            }
            ti++;
        }else{
            tOnline++;
        }
        ItemTenement = ItemTenement->next;
    }
    CUdpRecv->m_nAllTenementOnline = tOnline;
    ui->QlblOnlineTenement->setText(QString::number(CUdpRecv->m_nAllTenementOnline));

}
//在线住户设备列表
void CSERVERCENTER::_TenementOnline(NodeTenement *ItemTenement)
{
    int ti = 0;
    ui->QTenement->setRowCount(ti);
    while(ItemTenement->next != NULL)
    {
        if(ItemTenement->data.nState > 0)
        {
            ui->QTenement->setRowCount(ti + 1);
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem(QString::number(ti+1));
            tabledmItem0->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 0, tabledmItem0);



            QTableWidgetItem *tabledmItem1 = new QTableWidgetItem( QString(ItemTenement->data.gcAddr));
            tabledmItem1->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 1, tabledmItem1);

            QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemTenement->data.gcAddrExplain));
            tabledmItem2->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 2, tabledmItem2);

            QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemTenement->data.gcName));
            tabledmItem3->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 3, tabledmItem3);

            QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone1));
            tabledmItem4->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 4, tabledmItem4);

            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem(QString(ItemTenement->data.gcPhone2));
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 5, tabledmItem5);

            QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemTenement->data.gcMacAddr));
            tabledmItem6->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 6, tabledmItem6);

            QTableWidgetItem *tabledmItem7 = new QTableWidgetItem(QString(ItemTenement->data.gcIpAddr));
            tabledmItem7->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 7, tabledmItem7);

            if(ItemTenement->data.nState <= 0)
            {
                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("未连接");
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 8, tabledmItem8);
            }else{
                QTableWidgetItem *tabledmItem8 = new QTableWidgetItem("已连接");
                tabledmItem8->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 8, tabledmItem8);
            }

            if(ItemTenement->data.nFenceState == 0)
            {
                QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("撤防");
                tabledmItem9->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 9, tabledmItem9);
            }else{
                switch(ItemTenement->data.nFenceState){
                case 1:
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("外出布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                case 2:
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("在家布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                default :
                {
                    QTableWidgetItem *tabledmItem9 = new QTableWidgetItem("未知布防");
                    tabledmItem9->setTextAlignment(Qt::AlignCenter);
                    ui->QTenement->setItem(ti, 9, tabledmItem9);
                    break;
                }
                }
            }

            QTableWidgetItem *tabledmItem11 = new QTableWidgetItem(QString(ItemTenement->data.gcType));
            tabledmItem11->setTextAlignment(Qt::AlignCenter);
            ui->QTenement->setItem(ti, 11, tabledmItem11);

            if(ItemTenement->data.nAlarmState == 0)
            {
                QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("无报警");
                tabledmItem10->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 10, tabledmItem10);
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setBackgroundColor(QColor(0,125,0,0));
                }
            }else{
                QTableWidgetItem *tabledmItem10 = new QTableWidgetItem("报警");
                tabledmItem10->setTextAlignment(Qt::AlignCenter);
                ui->QTenement->setItem(ti, 10, tabledmItem10);
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setBackgroundColor(QColor(255,0,0));
                }
            }

            if(ItemTenement->data.nState <= 0)
            {
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setTextColor(QColor(0,0,0));
                }
            }else{
                for(int j = 0;j < 12;j++)
                {
                    QTableWidgetItem *item = ui->QTenement->item(ti, j);
                    item->setTextColor(Qt::blue);
                }
            }
            ti++;
        }
        ItemTenement = ItemTenement->next;
    }
    CUdpRecv->m_nAllTenementOnline = ti;
    ui->QlblOnlineTenement->setText(QString::number(CUdpRecv->m_nAllTenementOnline));

}
//更新新闻列表
void CSERVERCENTER::_UpdateNews(NodeMiddle *ItemMiddle)
{
    int ti = 0;
    int tOnline = 0;
    ui->QNews->setRowCount(ti);
    while(ItemMiddle->next != NULL)
    {
        ui->QNews->setRowCount(ti + 1);
        ItemMiddle->data.nId = ti;
        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
        if(ItemMiddle->data.nCheckedState == 0){
        tabledmItem0->setCheckState(Qt::Unchecked);
        }else{
            tabledmItem0->setCheckState(Qt::Checked);
        }
        tabledmItem0->setText(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QNews->setItem(ti, 0, tabledmItem0);
        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QNews->setItem(ti, 1, tabledmItem1);
        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemMiddle->data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QNews->setItem(ti, 2, tabledmItem2);

        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemMiddle->data.gcIpAddr));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QNews->setItem(ti, 3, tabledmItem4);
        if(ItemMiddle->data.nState > 0)
        {
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QNews->setItem(ti, 4, tabledmItem5);
            tOnline++;
        }else{
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QNews->setItem(ti, 4, tabledmItem5);
        }
        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemMiddle->data.gcType));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QNews->setItem(ti, 5, tabledmItem6);

        if(ItemMiddle->data.nState <= 0)
        {
            for(int j = 0;j < 6;j++)
            {
                QTableWidgetItem *item = ui->QNews->item(ti, j);
                item->setTextColor(QColor(0,0,0));
            }
        }else{
            for(int j = 0;j < 6;j++)
            {
                QTableWidgetItem *item = ui->QNews->item(ti, j);
                item->setTextColor(Qt::blue);
            }
        }
        ti++;
        ItemMiddle = ItemMiddle->next;
    }
    CUdpRecv->m_nAllMiddleOnline = tOnline;
}

//更新普通信息列表
void CSERVERCENTER::_UpdateMessage(NodeTenement *ItemTenement)
{
    int ti = 0;
    ui->QMessage->setRowCount(ti);
    while(ItemTenement->next != NULL)
    {
        ui->QMessage->setRowCount(ti + 1);
        ItemTenement->data.nId = ti;
        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
        if(ItemTenement->data.nCheckedState == 0)
        {
            tabledmItem0->setCheckState(Qt::Unchecked);
        }
        else
        {
            tabledmItem0->setCheckState(Qt::Checked);
        }
        tabledmItem0->setText(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QMessage->setItem(ti, 0, tabledmItem0);
        QTableWidgetItem *tabledmItem1 = new QTableWidgetItem(QString(ItemTenement->data.gcAddr));
        tabledmItem1->setTextAlignment(Qt::AlignCenter);
        ui->QMessage->setItem(ti, 1, tabledmItem1);
        QTableWidgetItem *tabledmItem2 = new QTableWidgetItem(QString(ItemTenement->data.gcAddrExplain));
        tabledmItem2->setTextAlignment(Qt::AlignCenter);
        ui->QMessage->setItem(ti, 2, tabledmItem2);
        QTableWidgetItem *tabledmItem3 = new QTableWidgetItem(QString(ItemTenement->data.gcName));
        tabledmItem3->setTextAlignment(Qt::AlignCenter);
        ui->QMessage->setItem(ti, 3, tabledmItem3);
        QTableWidgetItem *tabledmItem4 = new QTableWidgetItem(QString(ItemTenement->data.gcIpAddr));
        tabledmItem4->setTextAlignment(Qt::AlignCenter);
        ui->QMessage->setItem(ti, 4, tabledmItem4);
        if(ItemTenement->data.nState > 0)
        {
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("已连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMessage->setItem(ti, 5, tabledmItem5);
        }else{
            QTableWidgetItem *tabledmItem5 = new QTableWidgetItem("未连接");
            tabledmItem5->setTextAlignment(Qt::AlignCenter);
            ui->QMessage->setItem(ti, 5, tabledmItem5);
        }
        QTableWidgetItem *tabledmItem6 = new QTableWidgetItem(QString(ItemTenement->data.gcType));
        tabledmItem6->setTextAlignment(Qt::AlignCenter);
        ui->QMessage->setItem(ti, 6, tabledmItem6);

        if(ItemTenement->data.nState <= 0)
        {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMessage->item(ti, j);
                item->setTextColor(QColor(0,0,0));
            }
        }
        else
        {
            for(int j = 0;j < 7;j++)
            {
                QTableWidgetItem *item = ui->QMessage->item(ti, j);
                item->setTextColor(Qt::blue);
            }
        }
        ti++;
        ItemTenement = ItemTenement->next;
    }
}
//全选
void CSERVERCENTER::on_QpbtnCheckAllT_clicked()
{
    NodeTenement *q;
    q = CUdpRecv->m_TenementItem;
    int ti = 0;
    while(q->next != NULL)
    {
    QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
    tabledmItem0->setCheckState(Qt::Checked);
    tabledmItem0->setText(QString::number(ti+1));
    ui->QMessage->setItem(ti, 0, tabledmItem0);
    ti++;
    q = q->next;
    }
}
//选中在线
void CSERVERCENTER::on_QpbtnCheckOnlineT_clicked()
{
    NodeTenement *q;
    q = CUdpRecv->m_TenementItem;
    int ti = 0;
    while(q->next != NULL)
    {
        if(q->data.nState > 0)
        {
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
            tabledmItem0->setCheckState(Qt::Checked);
            tabledmItem0->setText(QString::number(ti+1));
            ui->QMessage->setItem(ti, 0, tabledmItem0);
        }else {
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
            tabledmItem0->setCheckState(Qt::Unchecked);
            tabledmItem0->setText(QString::number(ti+1));
            ui->QMessage->setItem(ti, 0, tabledmItem0);
        }
        ti++;
        q = q->next;
    }
}
//反选
void CSERVERCENTER::on_QpbtnCheckUnselectT_clicked()
{
    NodeTenement *q;
    q = CUdpRecv->m_TenementItem;
    int ti = 0;
    while(q->next != NULL)
    {
        if(q->data.nCheckedState == 0)
        {
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
            tabledmItem0->setCheckState(Qt::Checked);
            tabledmItem0->setText(QString::number(ti+1));
            ui->QMessage->setItem(ti, 0, tabledmItem0);
        }else {
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
            tabledmItem0->setCheckState(Qt::Unchecked);
            tabledmItem0->setText(QString::number(ti+1));
            ui->QMessage->setItem(ti, 0, tabledmItem0);
        }
        ti++;
        q = q->next;
    }
}
//全不选
void CSERVERCENTER::on_QpbtnCheckInvertT_clicked()
{
    NodeTenement *q;
    q = CUdpRecv->m_TenementItem;
    int ti = 0;
    while(q->next != NULL)
    {
    QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
    tabledmItem0->setCheckState(Qt::Unchecked);
    tabledmItem0->setText(QString::number(ti+1));
    ui->QMessage->setItem(ti, 0, tabledmItem0);
    ti++;
    q = q->next;
    }
}
//清空发送结果
void CSERVERCENTER::_UpdateMessageResult()
{
    int tRow = ui->QMessage->rowCount();
    for(int ti = 0;ti < tRow;ti++)
    {
        QTableWidgetItem *tabledmItem7 = new QTableWidgetItem("");
        tabledmItem7->setTextAlignment(Qt::AlignCenter);
        ui->QMessage->setItem(ti, 7, tabledmItem7);
    }
}
//发送
void CSERVERCENTER::on_QpbtnSendT_clicked()
{
    _UpdateMessageResult();
    QString strMessage = ui->QtextMessage->toPlainText();
    if(strMessage!="")
    {
        switch( QMessageBox::information( this, tr("提示"),tr("确定要发送?"),tr("是"), tr("否"),0, 1 ) )
        {
        case 0:
            {
            ui->QpbtnSendT->setEnabled(false);
            if(CUdpRecv->_SendMessage(strMessage) == 0)
                QMessageBox::information( this, tr("提示"),tr("请选择发送目标"));
            break;
            }
        case 1:
            break;
        default:
            break;
        }
    }else{
        QMessageBox::information( this, tr("提示"),tr("请输入要发送的内容"));
    }
}

void CSERVERCENTER::on_QchkMiddle_activated(int index)
{
    m_nIndex = index;
    m_nFindDevice == 0;
    switch(index)
    {
        case 0:
            _UpdateMiddle(CUdpRecv->m_MiddleItem);
            break;
        case 1:
            _MiddleOnline(CUdpRecv->m_MiddleItem);
            break;
        case 2:
            _MiddleNotOnline(CUdpRecv->m_MiddleItem);
            break;
        case 3:
            _MiddleM(CUdpRecv->m_MiddleItem);
            break;
        case 4:
            _MiddleW(CUdpRecv->m_MiddleItem);
            break;
        case 5:
            _MiddleZ(CUdpRecv->m_MiddleItem);
            break;
        case 6:
            _MiddleP(CUdpRecv->m_MiddleItem);
            break;
        case 7:
            _MiddleH(CUdpRecv->m_MiddleItem);
            break;
        default :
            break;
    }
}


//浏览
void CSERVERCENTER::on_QpbtnBrowse_clicked()
{
    CUdpRecv->Files = QFileDialog::getOpenFileNames(this, tr("Open File..."),QString("GG-demo3.png,GG-demo4.png,GG-demo5.png,adv1.avi,adv2.avi,adv3.avi"), tr("Image&video Files(*.png *.avi)"));
    CUdpRecv->strPath.clear();
    if(!(CUdpRecv->Files.isEmpty()))
    {
        for(int i = 0;i < CUdpRecv->Files.size();i++)
        {
            QString str = CUdpRecv->Files.at(i);
            int ti = str.lastIndexOf('\\');
            if(ti == -1)
            {
              ti = str.lastIndexOf('/');
            }
            int tj = str.length();
            QString t_strName = str.right(tj-ti-1);
            if((t_strName == "adv1.avi") || (t_strName == "GG-demo3.png")|| (t_strName == "GG-demo4.png")|| (t_strName == "GG-demo5.png") || (t_strName == "adv2.avi")|| (t_strName == "adv3.avi"))
            {
                ;
            }
            else
            {
                CUdpRecv->Files.removeAt(i);
                i--;
            }
        }
        CUdpRecv->nAallfile = CUdpRecv->Files.size();
        CUdpRecv->_Path();
        ui->QlinePath->setText(CUdpRecv->strPath);
    }
    else
    {
        ui->QlinePath->setText(CUdpRecv->strPath);
    }
}
//清空结果
void CSERVERCENTER::_ClearNewsResult()
{
    int tRow = ui->QNews->rowCount();
    for(int ti = 0;ti < tRow;ti++)
    {
        QTableWidgetItem *tabledmItem7 = new QTableWidgetItem("");
        ui->QNews->setItem(ti, 6, tabledmItem7);
    }
}
//发送新闻
void CSERVERCENTER::on_QpbtnSendM_clicked()
{
    CUdpRecv->nSendId = 1;
    _ClearNewsResult();
    if(CUdpRecv->strPath != NULL && (!(CUdpRecv->Files.isEmpty())))
    {
        NodeMiddle *p;
        p = CUdpRecv->m_MiddleItem;
        while(p->next)
        {
            if(p->data.nCheckedState)
            {
                QString IP = QString(p->data.gcIpAddr);
                CUdpRecv->nCheckId = p->data.nId;
                CUdpRecv->_LoadDown(IP);
                return;
            }
            p = p->next;
        }
    }
    else
    {
        QMessageBox::warning(NULL,tr("提示"),tr("请选择要下发的文件"));
        return;
    }
    QMessageBox::warning(NULL,tr("提示"),tr("请选择要下发的目标地址"));
    return;
}


//全选
void CSERVERCENTER::on_QpbtnCheckAllM_clicked()
{
    NodeMiddle *q;
    q = CUdpRecv->m_MiddleItem;
    int ti = 0;
    while(q->next != NULL)
    {
        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
        tabledmItem0->setCheckState(Qt::Checked);
        tabledmItem0->setText(QString::number(ti+1));
        tabledmItem0->setTextAlignment(Qt::AlignCenter);
        ui->QNews->setItem(ti, 0, tabledmItem0);
        ti++;
        q = q->next;
    }
}
//选中在线
void CSERVERCENTER::on_QpbtnCheckOnlineM_clicked()
{
    NodeMiddle *q;
    q = CUdpRecv->m_MiddleItem;
    int ti = 0;
    while(q->next != NULL)
    {
        if(q->data.nState > 0)
        {
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
            tabledmItem0->setCheckState(Qt::Checked);
            tabledmItem0->setText(QString::number(ti+1));
            ui->QNews->setItem(ti, 0, tabledmItem0);
        }
        else
        {
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
            tabledmItem0->setCheckState(Qt::Unchecked);
            tabledmItem0->setText(QString::number(ti+1));
            ui->QNews->setItem(ti, 0, tabledmItem0);
        }
        ti++;
        q = q->next;
    }
}
//反选
void CSERVERCENTER::on_QpbtnCheckUnselectM_clicked()
{
    NodeMiddle *q;
    q = CUdpRecv->m_MiddleItem;
    int ti = 0;
    while(q->next != NULL)
    {
        if(q->data.nCheckedState == 0)
        {
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
            tabledmItem0->setCheckState(Qt::Checked);
            tabledmItem0->setText(QString::number(ti+1));
            ui->QNews->setItem(ti, 0, tabledmItem0);
        }
        else
        {
            QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
            tabledmItem0->setCheckState(Qt::Unchecked);
            tabledmItem0->setText(QString::number(ti+1));
            ui->QNews->setItem(ti, 0, tabledmItem0);
        }
        ti++;
        q = q->next;
    }
}
//全不选
void CSERVERCENTER::on_QpbtnCheckInverM_clicked()
{
    NodeMiddle *q;
    q = CUdpRecv->m_MiddleItem;
    int ti = 0;
    while(q->next != NULL)
    {
        QTableWidgetItem *tabledmItem0 = new QTableWidgetItem();
        tabledmItem0->setCheckState(Qt::Unchecked);
        tabledmItem0->setText(QString::number(ti+1));
        ui->QNews->setItem(ti, 0, tabledmItem0);
        ti++;
        q = q->next;
    }
}

//右键菜单栏 begin
void CSERVERCENTER::on_QMiddle_customContextMenuRequested(const QPoint &pos)
{
//    MiddleMenu->addAction(UpdateAction);
    MiddleMenu->addAction(DeleteAction);
    MiddleMenu->addAction(FindAction);
    MiddleMenu->addAction(PingIpAction);
    MiddleMenu->addAction(InitSystemPassAction);
    if(m_sUser.protocol != 0)
    {
        MiddleMenu->addAction(InitLockPassAction);
    }
    MiddleMenu->exec(QCursor::pos());//在当前鼠标位置显示
}

void CSERVERCENTER::on_QTenement_customContextMenuRequested(const QPoint &pos)
{
//    TenementMenu->addAction(UpdateAction);
    TenementMenu->addAction(DeleteAction);
    if(m_sUser.protocol != 0)       //A8没有重启命令
    {
        TenementMenu->addAction(ResetAction);
    }
    TenementMenu->addAction(FindAction);
//    TenementMenu->addAction(SmartControlAction);
    TenementMenu->addAction(PingIpAction);
    TenementMenu->addAction(InitSystemPassAction);
    TenementMenu->exec(QCursor::pos());//在当前鼠标位置显示
}

void CSERVERCENTER::on_QRepairs_customContextMenuRequested(const QPoint &pos)
{
    RepairsMenu->addAction(RepairsUpdateAction);
    RepairsMenu->addAction(RepairsDeleteAction);
    RepairsMenu->addAction(RepairsAddAction);
    RepairsMenu->addAction(RepairsDownAction);
    RepairsMenu->exec(QCursor::pos());//在当前鼠标位置显示
}

void CSERVERCENTER::on_QPhone_customContextMenuRequested(const QPoint &pos)
{
    PhoneMenu->addAction(PhoneUpdateAction);
    PhoneMenu->addAction(PhoneDeleteAction);
    PhoneMenu->addAction(PhoneAddAction);
    PhoneMenu->addAction(PhoneDownAction);
    PhoneMenu->exec(QCursor::pos());//在当前鼠标位置显示
}

void CSERVERCENTER::on_QIcCard_customContextMenuRequested(const QPoint &pos)
{
    IcCardMenu->addAction(IcCarUpdateAction);
    IcCardMenu->addAction(IcCarDeleteAction);
    IcCardMenu->addAction(IcCarAddAction);
    IcCardMenu->addAction(IcCarFindAction);
    IcCardMenu->exec(QCursor::pos());//在当前鼠标位置显示
}

void CSERVERCENTER::on_QCardMessage_customContextMenuRequested(const QPoint &pos)
{
    CardMessageMenu->addAction(CardMessageUpAction);
    CardMessageMenu->addAction(CardMessageDownAction);
//    CardMessageMenu->addAction(CardMessageDownAllAction);
    CardMessageMenu->addAction(CardMessageSaveAction);
    CardMessageMenu->addAction(CardLogOutAction);
    CardMessageMenu->exec(QCursor::pos());//在当前鼠标位置显示
}

void CSERVERCENTER::on_QUser_customContextMenuRequested(const QPoint &pos)
{
    if(m_sUser.nGrade != 3)
    {
        UserMenu->addAction(UserDeleteAction);
        UserMenu->addAction(UserAddAction);
    }
    UserMenu->addAction(UserUpdateAction);
    UserMenu->exec(QCursor::pos());
}

void CSERVERCENTER::on_QRepairsRecord_customContextMenuRequested(const QPoint &pos)
{
    RepairsRecordMenu->addAction(RepairsRecordDeleteAction);
    RepairsRecordMenu->addAction(RepairsRecordFindAction);
    RepairsRecordMenu->addAction(RepairsRecordDealAction);
    RepairsRecordMenu->addAction(PreviousPageAction);
    RepairsRecordMenu->addAction(NextPageAction);
    RepairsRecordMenu->addAction(DealAllAction);
    RepairsRecordMenu->addAction(StopAction);
    RepairsRecordMenu->exec(QCursor::pos());
}

void CSERVERCENTER::on_QAlarmRecord_customContextMenuRequested(const QPoint &pos)
{
    AlarmRecordMenu->addAction(AlarmRecordFindAction);
    AlarmRecordMenu->addAction(AlarmRecordDeleteAction);
    AlarmRecordMenu->addAction(AlarmDealAction);
    AlarmRecordMenu->addAction(PreviousPageAction);
    AlarmRecordMenu->addAction(NextPageAction);
    AlarmRecordMenu->addAction(DealAllAction);
    AlarmRecordMenu->addAction(StopAction);
    AlarmRecordMenu->exec(QCursor::pos());
}

void CSERVERCENTER::on_QCardRecord_customContextMenuRequested(const QPoint &pos)
{
    CardRecordMenu->addAction(CardRecordFindAction);
    CardRecordMenu->addAction(CardRecordDeleteAction);
    CardRecordMenu->addAction(PreviousPageAction);
    CardRecordMenu->addAction(NextPageAction);
    CardRecordMenu->exec(QCursor::pos());
}

void CSERVERCENTER::on_QDevice_customContextMenuRequested(const QPoint &pos)
{
    DeviceMenu->addAction(DeviceFindAction);
    DeviceMenu->addAction(DeviceDeleteAction);
    DeviceMenu->addAction(PreviousPageAction);
    DeviceMenu->addAction(NextPageAction);
    DeviceMenu->exec(QCursor::pos());
}

void CSERVERCENTER::on_QSystem_customContextMenuRequested(const QPoint &pos)
{
    SystemMenu->addAction(SystemFindAction);
    SystemMenu->addAction(SystemDeleteAction);
    SystemMenu->addAction(PreviousPageAction);
    SystemMenu->addAction(NextPageAction);
    SystemMenu->exec(QCursor::pos());
}

void CSERVERCENTER::on_QAlarm_customContextMenuRequested(const QPoint &pos)
{
    AlarmMenu->addAction(DealAction);
    AlarmMenu->addAction(StopAction);
    AlarmMenu->exec(QCursor::pos());
}

void CSERVERCENTER::on_QCallRecord_customContextMenuRequested(const QPoint &pos)
{
    CallRecordMenu->addAction(CallRecordDeleteAction);
    CallRecordMenu->addAction(PreviousPageAction);
    CallRecordMenu->addAction(NextPageAction);
    CallRecordMenu->exec(QCursor::pos());
}

void CSERVERCENTER::on_Empower_IC_customContextMenuRequested(const QPoint &pos)
{
    DeleteCardMenu->addAction(DeleteCardAction);
    DeleteCardMenu->exec(QCursor::pos());
}

void CSERVERCENTER::on_Upload_IC_customContextMenuRequested(const QPoint &pos)
{
    AddCardMenu->addAction(ClearAllCardAction);
    AddCardMenu->addAction(SaveCardAction);
    AddCardMenu->addAction(SaveAllCardAction);
    AddCardMenu->exec(QCursor::pos());
}
//鼠标右键菜单 end

//删除此卡
void CSERVERCENTER::SlotClearAllCard()
{
    ui->Upload_IC->setRowCount(0);
    ui->AllUpCard->clear();
}

void CSERVERCENTER::SlotDeleteCard()
{
    if(ui->Empower_IC->currentRow() >= 0)
    {
        switch( QMessageBox::question( this, tr("提示"),"确定要删除IC卡：" + ui->Empower_IC->currentItem()->text(),tr("Yes"), tr("No"),0, 1 ) )
        {
            case 0:
            {
                QString SQL = "DELETE FROM ic_card_a8 WHERE ic_card_a8.IC_Card_num = '"+ui->Empower_IC->currentItem()->text()+"'";
                QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
                QSqlQuery query(db);
                query.exec(SQL);
                CCONNECTIONPOOLMYSQL::closeConnection(db);
                if(ui->Empower_Indoor->currentItem()->text(0).right(2)== "设备" && ui->Empower_Indoor->currentItem()->parent()->text(0).right(2) == "单元")
                {
                    QString strAddExplain = ui->Empower_Indoor->currentItem()->parent()->text(0)+ui->Empower_Indoor->currentItem()->text(0);
                    _UpdateEmpowerIC(strAddExplain);
                }
            }
                break;
            case 1:
                break;
        }
    }
    else
    {
        QMessageBox::information(this,"提示","请选择要删除的卡");
    }
}

void CSERVERCENTER::SlotSaveUpCard()
{
    if(ui->Upload_IC->currentRow() > 0)
    {
        QString CardNum = ui->Upload_IC->item(ui->Upload_IC->currentIndex().row(),1)->text();
        QString CardAddr = ui->Upload_IC->item(ui->Upload_IC->currentIndex().row(),2)->text();
        QString strCardSAddr = _AddrExplainToAddrA8(CardAddr,"室内机",1);
        QString CardTime = QDate::currentDate().addYears(2).toString("yyyy-MM-dd");
        QSqlDatabase db  = CCONNECTIONPOOLMYSQL::openConnection();

        QSqlQuery query(db);
        QString selectSQL = "SELECT * FROM ic_card_a8 WHERE IC_Card_addr = '"+strCardSAddr+"' AND IC_Card_num = '"+CardNum+"'";
        query.exec(selectSQL);
        if(query.next())
        {
//          QString updateSQL = "UPDATE ic_card_a8 SET IC_Card_addr = '"+strCardSAddr+"',IC_Card_num = '"+CardNum+"',IC_Card_time = '"+CardTime+"' WHERE IC_Card_addr = '"+CardAddr+"' AND IC_Card_num = '"+CardNum+"'";
//          QSqlQuery updatequery(db);
//          if(updatequery.exec(updateSQL))
//              QMessageBox::information(this,"提示","保存成功");
//          else
            QMessageBox::information(this,"提示","该IC卡已存在");
        }
        else
        {
            QString insertSQL = "INSERT INTO ic_card_a8 (IC_Card_addr,IC_Card_num,IC_Card_time) VALUES ('"+strCardSAddr+"','"+CardNum+"','"+CardTime+"')";
            QSqlQuery insertquery(db);
            if(insertquery.exec(insertSQL))
                QMessageBox::information(this,"提示","保存成功");
            else
                QMessageBox::information(this,"提示","保存失败");
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
    }
    else
    {
        QMessageBox::information(this,"提示","请选择需要保存的IC卡");
    }
}

void CSERVERCENTER::SlotSaveAllUpCard()
{
    if(ui->Upload_IC->rowCount() > 0)
    {
        int nFail = 0;
        int nSave = 0;
        for(int row = 0;row < ui->Upload_IC->rowCount();row++)
        {
            QString CardNum = ui->Upload_IC->item(row,1)->text();
            QString CardAddr = ui->Upload_IC->item(row,2)->text();
            QString strCardSAddr = _AddrExplainToAddrA8(CardAddr,"室内机",1);
            QString CardTime = QDate::currentDate().addYears(20).toString("yyyy-MM-dd");
            QSqlDatabase db  = CCONNECTIONPOOLMYSQL::openConnection();
            QSqlQuery query(db);
            QString selectSQL = "SELECT * FROM ic_card_a8 WHERE IC_Card_addr = '"+strCardSAddr+"' AND IC_Card_num = '"+CardNum+"'";
            query.exec(selectSQL);
            if(query.next() == NULL)
            {
                QString insertSQL = "INSERT INTO ic_card_a8 (IC_Card_addr,IC_Card_num,IC_Card_time) VALUES ('"+strCardSAddr+"','"+CardNum+"','"+CardTime+"')";
                QSqlQuery insertquery(db);
                if(!(insertquery.exec(insertSQL)))
                {
//                    QMessageBox::information(this,"提示",CardNum+"保存失败");
                    nFail++;
                }
            }
            else
            {
                nSave++;
            }
            CCONNECTIONPOOLMYSQL::closeConnection(db);
        }
        QMessageBox::information(this,"提示","已保存全部卡号,其中保存失败" + QString::number(nFail) + "张,已存在的卡：" + QString::number(nSave) + "张");
    }
    else
    {
        QMessageBox::information(this,"提示","没有可保存的IC卡");
    }
}

void CSERVERCENTER::on_QtextMessage_textChanged()
{
    int nLenth = ui->QtextMessage->toPlainText().length();
    if(nLenth >= 201)
    {
        int position = ui->QtextMessage->textCursor().position();
         QTextCursor textCursor = ui->QtextMessage->textCursor();
         ui->QtextMessage->toPlainText().remove(position - (nLenth - 200), nLenth - 200);
         ui->QtextMessage->setText(ui->QtextMessage->toPlainText().left(200));
         textCursor.setPosition(position - (nLenth - 200));
         ui->QtextMessage->setTextCursor(textCursor);
         QMessageBox::information(this,tr("提示"),tr("最大长度为200"));
    }
}

void CSERVERCENTER::on_QIcCard_clicked(const QModelIndex &index)
{
    int nFlag = 0;
    NodeCard *p = m_ItemCard;
    while(p->next)
    {
        if(p->data.nNewCheck != p->data.nOldCheck)
        {
            nFlag = 1;
            break;
        }
        p = p->next;
    }
    if(nFlag == 1)
    {
        switch( QMessageBox::question( this, tr("提示"),tr("是否保存修改"),tr("Yes"), tr("No"),0, 1 ) )
        {
            case 0:
            {
                NodeCard *q = m_ItemCard;
                while(q->next)
                {
                    if(q->data.nNewCheck != q->data.nOldCheck)
                    {
                        CUdpRecv->m_Mysql._UpdateCardMessage(m_strIcCard,QString(q->data.gcAddr),q->data.nNewCheck);
                    }
                    q = q->next;
                }
            }
                break;
            default :
                break;
        }
    }
    m_strIcCard = ui->QIcCard->item(ui->QIcCard->currentRow(),1)->text();
    _CheckCardMessage();
}

void CSERVERCENTER::on_QpbtnExplain_clicked()
{
    QMessageBox::information(this,tr("提示"),tr("下载图片名称只能为：GG-demo3.png,GG-demo4.png,GG-demo5.png\n下载视频名称只能为：adv1.avi,adv2.avi,adv3.avi"));
}
//"IC卡注册"按钮
void CSERVERCENTER::on_Qpbtn_IC_Register_clicked()
{
    ui->IC_stackedWidget->setCurrentIndex(0);
    _SetTableWidgetSatus(1);
    m_nMode = 0;
    ui->QpbtnMode->setText("自动");
    ui->QpbtnAdd->setEnabled(false);
    _UpdateEmpowerIndoor(CUdpRecv->m_TenementItem);
}
//"IC卡授权"按钮
void CSERVERCENTER::on_Qpbtn_Empower_clicked()
{
    ui->IC_stackedWidget->setCurrentIndex(1);
    _SetTableWidgetSatus(2);
    ui->QpbtnAuthorization->setEnabled(false);  //"授权"按钮不可用
    _UpdateRegisterIndoor(CUdpRecv->m_TenementItem);
    _UpdateRegisterMachine(CUdpRecv->m_MiddleItem);
}
//"IC卡下发"按钮
void CSERVERCENTER::on_Qpbtn_lssued_clicked()
{
    ui->IC_stackedWidget->setCurrentIndex(2);
    _SetTableWidgetSatus(3);
    ui->QpbtnUp->setEnabled(false);
    ui->QpbtnDown->setEnabled(false);
    _UpdateIssuedMachine(CUdpRecv->m_MiddleItem);
}

void CSERVERCENTER::_UpdatePage(int nType)
{
    switch(nType)
    {
        case 3://报警记录
        {
            if(m_nFind)
            {
                _UpdateFindAlarmRecord(m_strFindSql);
            }
            else
                _UpdateAlarmRecord();
        }
            break;
        case 4://报修记录
        {
            if(m_nFind)
            {
                _UpdateFindRepairsRecord(m_strFindSql);
            }
            else
                _UpdateRepairsRecord();
        }
            break;
        case 7://系统日志
        {
            if(m_nFind)
            {
                _UpdateFindSystem(m_strFindSql);
            }
            else
                _UpdateSystem();
        }
            break;
        case 8://设备日志
        {
            if(m_nFind)
            {
                _UpdateFindDevice(m_strFindSql);
            }
            else
                _UpdateDevice();
        }
            break;
        case 9://刷卡记录
        {
            if(m_nFind)
            {
                _UpdateFindCardRecord(m_strFindSql);
            }
            else
                _UpdateCardRecord();
        }
            break;
        case 14://呼叫记录
        {
            if(m_nFind)
            {
                _UpdateCallRecord(m_strFindSql,1);
            }
            else
                _UpdateCallRecord();
        }
            break;
        default :
            break;
    }
}

//通话记录
void CSERVERCENTER::_UpdateCallNow()
{
    QString strStime = QDateTime::currentDateTime().addDays(-5).toString("yyyy-MM-dd hh:mm:ss");
    QString SQL = "SELECT * FROM call_photo_a8 where Call_Photo_Time > '"+strStime+"' ORDER BY  Call_Photo_Time DESC limit 0,10";
    QSqlDatabase db  = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    if(query.exec(SQL))
    {
        int ti = 0;
        ui->QCallNow->setRowCount(ti);
        while(query.next())
        {
            QString Maddr,Saddr;
            Maddr = _AddrToAddrExplainA8(query.value(2).toString(),"门口机");
            Saddr = _AddrToAddrExplainA8(query.value(3).toString(),"室内机");
            ui->QCallNow->setRowCount(ti+1);
            QTableWidgetItem *tableItem0 = new QTableWidgetItem(QString::number(ti+1));
            tableItem0->setTextAlignment(Qt::AlignCenter);
            ui->QCallNow->setItem(ti,0,tableItem0);

            QTableWidgetItem *tableItem1 = new QTableWidgetItem(query.value(1).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            tableItem1->setTextAlignment(Qt::AlignCenter);
            ui->QCallNow->setItem(ti,1,tableItem1);

            QTableWidgetItem *tableItem2 = new QTableWidgetItem(Maddr);
            tableItem2->setTextAlignment(Qt::AlignCenter);
            ui->QCallNow->setItem(ti,2,tableItem2);

            QTableWidgetItem *tableItem3 = new QTableWidgetItem(Saddr);
            tableItem3->setTextAlignment(Qt::AlignCenter);
            ui->QCallNow->setItem(ti,3,tableItem3);

            QTableWidgetItem *tableItem4 = new QTableWidgetItem(query.value(4).toString());
            tableItem4->setTextAlignment(Qt::AlignCenter);
            ui->QCallNow->setItem(ti,4,tableItem4);
            ti++;
        }
//        ui->QlblAll->setText(QString::number(ti));
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

//呼叫记录
void CSERVERCENTER::_UpdateCallRecord(QString strSQL, int t)
{
    m_nCount = 0;
    QDateTime dateTime = QDateTime::currentDateTime();
    dateTime = dateTime.addDays(-15);
    QString stime = dateTime.toString("yyyy-MM-dd");

    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db),query1(db);

    QString SQL;
    if(t != 0)
    {
        QString strSQL1;
        if(strSQL != "")
        {
            strSQL1 = "SELECT count(*) FROM call_photo_a8 "+ strSQL;
        }
        else
        {
            strSQL1 = "SELECT count(*) FROM call_photo_a8";
        }
        if(query1.exec(strSQL1))
        {
            while(query1.next())
            {
                m_nCount = query1.value(0).toInt();
            }
        }

        SQL =  "SELECT * FROM call_photo_a8" + strSQL + " ORDER BY Call_Photo_Time DESC";;
    }
    else
    {
        QString strSQL1 = "SELECT count(*) FROM call_photo_a8 WHERE Call_Photo_Time > '"+stime+"'";
        if(query1.exec(strSQL1))
        {
            while(query1.next())
            {
                m_nCount = query1.value(0).toInt();
            }
        }
        SQL = "SELECT * FROM call_photo_a8 WHERE Call_Photo_Time > '"+stime+"'";
    }
    ui->QlblAll->setText(QString::number(m_nCount));
    if(m_nCount == 0)
    {
        ui->QCallRecord->setRowCount(m_nCount);
        QMessageBox::information(this, "提示", "没有最近15天的呼叫记录");
    }
    else
    {
        SQL = SQL + " limit " + QString::number(m_nPage*PAGE_MAXLEN)+","+QString::number(PAGE_MAXLEN);
        if(query.exec(SQL))
        {
            int ti = 0;
            ui->QCallRecord->setRowCount(ti);
            while(query.next())
            {
                QString Maddr,Saddr;
                Maddr = _AddrToAddrExplainA8(query.value(2).toString(),"门口机");
                Saddr = _AddrToAddrExplainA8(query.value(3).toString(),"室内机");
                ui->QCallRecord->setRowCount(ti+1);
                QTableWidgetItem *tableItem0 = new QTableWidgetItem(QString::number(m_nPage*PAGE_MAXLEN + ti+1));
                tableItem0->setTextAlignment(Qt::AlignCenter);
                ui->QCallRecord->setItem(ti,0,tableItem0);

                QTableWidgetItem *tableItem1 = new QTableWidgetItem(query.value(1).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                tableItem1->setTextAlignment(Qt::AlignCenter);
                ui->QCallRecord->setItem(ti,1,tableItem1);

                QTableWidgetItem *tableItem2 = new QTableWidgetItem(Maddr);
                tableItem2->setTextAlignment(Qt::AlignCenter);
                ui->QCallRecord->setItem(ti,2,tableItem2);

                QTableWidgetItem *tableItem3 = new QTableWidgetItem(Saddr);
                tableItem3->setTextAlignment(Qt::AlignCenter);
                ui->QCallRecord->setItem(ti,3,tableItem3);

                QTableWidgetItem *tableItem4 = new QTableWidgetItem(query.value(4).toString());
                tableItem4->setTextAlignment(Qt::AlignCenter);
                ui->QCallRecord->setItem(ti,4,tableItem4);
                ti++;
            }
        }
    }
    ui->Qpbtn_CallPhoto->setStyleSheet("");         //刷新界面时，把按钮上的图片去除，设置空样式表
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

void CSERVERCENTER::on_QCallNow_clicked(const QModelIndex &index)
{
    QString PhotoPath = ui->QCallNow->item(index.row(),4)->text();
    ui->QlblPic->setStyleSheet("border-image:url('"+PhotoPath+"')");
}

void CSERVERCENTER::on_QpbtnMode_clicked()
{
    if(m_nMode == 0)
    {
        m_nMode = 1;
        ui->QpbtnMode->setText("手动");
        ui->QpbtnAdd->setEnabled(true);
    }
    else
    {
        m_nMode = 0;
        ui->QpbtnMode->setText("自动");
        ui->QpbtnAdd->setEnabled(false);
    }
}

void CSERVERCENTER::on_Qpbtn_CallPhoto_clicked()
{
    if(ui->QCallRecord->currentRow() >= 0)
    {
        QString PhotoPath = ui->QCallRecord->item(ui->QCallRecord->currentRow(),4)->text();
        CPic->_Update(PhotoPath);
        CPic->move((QApplication::desktop()->width() - CPic->width())/2, (QApplication::desktop()->height() - CPic->height())/2);
        CPic->show();
    }
}

void CSERVERCENTER::on_QCallRecord_clicked(const QModelIndex &index)
{
    QString PhotoPath = ui->QCallRecord->item(index.row(),4)->text();
    ui->Qpbtn_CallPhoto->setStyleSheet("border-image:url('"+PhotoPath+"')");
}

void CSERVERCENTER::on_QpbtnDown_clicked()
{
    SlotDownCard_A8();
}

void CSERVERCENTER::on_QpbtnUp_clicked()
{
    SlotUpCard_A8();
}

void CSERVERCENTER::on_QrbtnTime_clicked()
{
    if(ui->QrbtnTime->isChecked())
    {
        ui->QdateETime->setEnabled(true);
        ui->QdateSTime->setEnabled(true);
        ui->QdateSTime->setFocus();
    }
    else
    {
        ui->QdateETime->setEnabled(false);
        ui->QdateSTime->setEnabled(false);
    }
}

void CSERVERCENTER::on_QpbtnFind_clicked()
{
    int nT = 0;
    QString strSql = "";
    switch(ui->QchkType->currentIndex())
    {
        case 0:
            break;
        case 1:
            strSql = strSql + " where Call_Photo_DialingAddr like '%"+ui->QlineFind->text()+"%'";
            nT =1;
            break;
        case 2:
            strSql = strSql + " where Call_Photo_CalledAddr like '%"+ui->QlineFind->text()+"%'";
            nT =1;
            break;
        default :
            break;
    }
    if(ui->QrbtnTime->isChecked())
    {
        if(ui->QdateETime > ui->QdateSTime)
        {
            QString strStime = ui->QdateSTime->text();
            QString strEtime = ui->QdateETime->text();
            if(nT)
            {
                strSql = strSql + " and Call_Photo_Time > '"+strStime+"' and Call_Photo_Time < '"+strEtime+"'";
            }
            else
            {
                strSql = strSql + " where Call_Photo_Time > '"+strStime+"' and Call_Photo_Time < '"+strEtime+"'";
            }
        }
        else
        {
            QMessageBox::information(this,tr("提示"),tr("开始时间不能大于结束时间"));
            ui->QdateSTime->setFocus();
            return ;
        }
    }
    _UpdateCallRecord(strSql,1);
}

void CSERVERCENTER::on_QchkType_currentIndexChanged(int index)
{
    if(index)
    {
        ui->QlineFind->setEnabled(true);
    }
    else
        ui->QlineFind->setEnabled(false);
}

void CSERVERCENTER::_SetTableWidgetSatus(int IcOperateType)
{
    switch(IcOperateType)
    {
        case 1:
            ui->Register_IC->setRowCount(0);
            ui->Download_IC->setRowCount(0);
            ui->Empower_IC->setHorizontalHeaderLabels(QStringList()<<"IC卡张数:0");
            break;
        case 2:
            ui->Empower_IC->setRowCount(0);
            ui->Download_IC->setRowCount(0);
            ui->Register_IC->setHorizontalHeaderLabels(QStringList()<<"IC卡张数:0");
            ui->Register_IC->setRowCount(0);
            break;
        case 3:
            ui->Empower_IC->setRowCount(0);
            ui->Register_IC->setRowCount(0);
            ui->Upload_IC->setRowCount(0);
            ui->Download_IC->setRowCount(0);
            ui->AllDownCard->clear();
            ui->AllUpCard->clear();
//            ui->Download_IC->setHorizontalHeaderLabels(QStringList()<<""<<"0");
            break;
        default:
            break;
    }
}

void CSERVERCENTER::_UpdateEmpowerIndoor(NodeTenement *ItemTenement)
{
    m_List.clear();
    ui->Empower_Indoor->clear();
    int ti = 0;
    QTreeWidgetItem *head[100];
    QString HeaderstrAddr;
    while(ItemTenement->next!=NULL)
    {
        HeaderstrAddr = QString(ItemTenement->data.gcAddrExplain).left(10);

        for(int i = 0;i < m_List.length();i++)
        {
            if(HeaderstrAddr == QString(m_List.at(i).data()))
            {
                QTreeWidgetItem *treeItem = new QTreeWidgetItem(QStringList()<<QString(ItemTenement->data.gcAddrExplain).right(10));
                head[i]->addChild(treeItem);
                isHeadItem = false;
            }
        }
        if(isHeadItem)
        {
            head[ti] = new QTreeWidgetItem(QStringList()<<QString(HeaderstrAddr));
            ui->Empower_Indoor->addTopLevelItem(head[ti]);
            QTreeWidgetItem *treeItem = new QTreeWidgetItem(QStringList()<<QString(ItemTenement->data.gcAddrExplain).right(10));
            head[ti]->addChild(treeItem);
            m_List.append(HeaderstrAddr);
            ti++;
        }
        isHeadItem = true;
        ItemTenement = ItemTenement->next;
    }
}

void CSERVERCENTER::on_QpbtnAdd_clicked()
{
    if(ui->QlineICCard->text().length() == 11)
    {
        if(ui->Empower_Indoor->currentItem()->text(0).right(2)== "设备" && ui->Empower_Indoor->currentItem()->parent()->text(0).right(2) == "单元")
        {
            QString strAddExplain;
            strAddExplain = ui->Empower_Indoor->currentItem()->parent()->text(0)+ui->Empower_Indoor->currentItem()->text(0);
            CMYSQL CMysql;
            if(CMysql._SelectIcCardA8(ui->QlineICCard->text()) == 0)
            {
                if(CMysql._AddIcCardA8(ui->QlineICCard->text(),_AddrExplainToAddrA8(strAddExplain,"室内机",1),QDate(QDate::currentDate().addYears(20)).toString("yyyy-MM-dd")));
                {
                    QMessageBox::information(this,tr("提示"),tr("添加成功"));
                    _UpdateEmpowerIC(strAddExplain);
                    ui->QlineICCard->clear();
                }
            }
            else
            {
                QMessageBox::information(this,tr("提示"),tr("卡号已存在"));

            }
        }
        else
            QMessageBox::information(this,"错误","请选择到室内机的设备号");
    }
    else
    {
        QMessageBox::information(this,"错误","请输入正确的卡号");
    }
}

void CSERVERCENTER::on_Empower_Indoor_clicked(const QModelIndex &index)
{
    //    ui->QpbtnAdd->setEnabled(true);
    if(ui->Empower_Indoor->currentItem()->text(0).right(2)== "设备" && ui->Empower_Indoor->currentItem()->parent()->text(0).right(2) == "单元")
    {
        QString strAddExplain;
        ui->QlineICCard->setFocus();
        strAddExplain = ui->Empower_Indoor->currentItem()->parent()->text(0) + ui->Empower_Indoor->currentItem()->text(0);
        _UpdateEmpowerIC(strAddExplain);
    }
    else
    {
        ui->Empower_IC->setRowCount(0);
        ui->Empower_IC->setHorizontalHeaderLabels(QStringList()<<"IC卡张数：" + QString::number(0));
    }
}

void CSERVERCENTER::on_QpbtnAuthorization_clicked()
{
    QString strAddExplain;
    if(ui->Register_Indoor->currentItem()->text(0).right(2)== "设备" && ui->Register_Indoor->currentItem()->parent()->text(0).right(2) == "单元")
    {
        strAddExplain = ui->Register_Indoor->currentItem()->parent()->text(0) + ui->Register_Indoor->currentItem()->text(0);
        CMYSQL CMysql;
        for(int row = 0; row < ui->Register_Machine->rowCount(); row++)
        {
            if(CMysql._DeleteIcJurisdictionA8(_AddrExplainToAddrA8(strAddExplain,"室内机",1),_AddrExplainToAddrA8(ui->Register_Machine->item(row,1)->text(),"门口机",1)))
            {
                if(ui->Register_Machine->item(row,0)->checkState() == Qt::Checked)
                {
                    int status = CMysql._AddIcJurisdictionA8(_AddrExplainToAddrA8(strAddExplain,"室内机",1),_AddrExplainToAddrA8(ui->Register_Machine->item(row,1)->text(),"门口机",1));
                    if(status == 1)
                        QMessageBox::information(this,"提示",strAddExplain + "与"+ui->Register_Machine->item(row,1)->text() + "之间授权成功");
                    else if(status == -1)
                        QMessageBox::information(this,"提示",strAddExplain + "与"+ui->Register_Machine->item(row,1)->text() + "之间已授权");
                    else
                        QMessageBox::information(this,"提示",strAddExplain + "与"+ui->Register_Machine->item(row,1)->text() + "之间授权失败");
                }
            }
        }
    }
    else
        QMessageBox::information(this,"错误","请选择到室内机的设备号");
}

void CSERVERCENTER::_UpdateRegisterIC(QString strAddrExplain)
{
    QString SQL = "SELECT * FROM ic_card_A8 WHERE IC_Card_addr = '"+_AddrExplainToAddrA8(strAddrExplain,"室内机",1)+"'";
    qDebug ()<<"SQL:"<<SQL;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    int ti = 0;
    if(query.exec(SQL))
    {
        ui->Register_IC->setRowCount(ti);
        while(query.next())
        {
            ui->Register_IC->setRowCount(ti+1);
            QTableWidgetItem *tableItem = new QTableWidgetItem(QString(query.value(2).toString()));
            tableItem->setTextAlignment(Qt::AlignCenter);
            ui->Register_IC->setItem(ti,0,tableItem);
            ti++;
        }
    }
    ui->Register_IC->setHorizontalHeaderLabels(QStringList()<<"IC卡张数：" + QString::number(ti));
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

void CSERVERCENTER::on_Register_Indoor_clicked(const QModelIndex &index)
{
    ui->QpbtnAuthorization->setEnabled(true);
    if(ui->Register_Indoor->currentItem()->text(0).right(2)== "设备" && ui->Register_Indoor->currentItem()->parent()->text(0).right(2) == "单元")
    {
        QString strAddExplain;
        strAddExplain = ui->Register_Indoor->currentItem()->parent()->text(0) + ui->Register_Indoor->currentItem()->text(0);
        _UpdateRegisterIC(strAddExplain);

        QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
        for(int row = 0; row < ui->Register_Machine->rowCount(); row++)
        {
            QString SQL = "SELECT * FROM card_jurisdiction_a8 WHERE Card_Jurisdiction_SAddr = '"+_AddrExplainToAddrA8(strAddExplain,"室内机",1)+"' AND Card_Jurisdiction_MAddr = '"+_AddrExplainToAddrA8(ui->Register_Machine->item(row,1)->text(),"门口机",1)+"'";
            QSqlQuery query(db);
            query.exec(SQL);
            if(query.next())
            {
                ui->Register_Machine->item(row,0)->setCheckState(Qt::Checked);
            }
            else
            {
                ui->Register_Machine->item(row,0)->setCheckState(Qt::Unchecked);
            }
        }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
    }
    else
    {
        ui->Register_IC->setRowCount(0);
        ui->Register_IC->setHorizontalHeaderLabels(QStringList()<<"IC卡张数：" + QString::number(0));
    }
}

//检测数据库是否正常连接
void CSERVERCENTER::SlotDisConnect()
{
    QString path = dir.currentPath();
    if(CCONNECTIONPOOLMYSQL::IsNetWork)
    {
        path = path + "/pic/green1.png";
    }
    else
    {
        path = path + "/pic/red1.png";
        qDebug()<<"MySQL已断开";
    }
    MySQLIcon->setPixmap(QPixmap(path));

    QString path1 = dir.currentPath();
    if(CCONNECTIONPOOLMSSQL::IsNetWork)
    {
        path1 = path1 + "/pic/green1.png";
    }
    else
    {
        path1 = path1 + "/pic/red1.png";
        qDebug()<<"MSSQL已断开";
    }
    MSSQLIcon->setPixmap(QPixmap(path1));
}
