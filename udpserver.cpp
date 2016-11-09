#include "udpserver.h"
#include <QMessageBox>
#include"mysql.h"
#include <QDebug>
CUDPSERVER::CUDPSERVER(QString strCityNum,QObject *parent) :
    QThread(parent)
{
    qRegisterMetaType<QVariant>("QVariant");
    m_strCity = strCityNum;
    _Init();
    _QueryNowWeather();
    _QueryDayWeather();
    m_TenementItem = (NodeTenement *)malloc(sizeof(NodeTenement));
    m_TenementItem ->next = NULL;

    m_MiddleItem = (NodeMiddle *)malloc(sizeof(NodeMiddle));
    m_MiddleItem ->next = NULL;

    m_AlarmItem = (NodeAlarm *)malloc(sizeof(NodeAlarm));
    m_AlarmItem ->next = NULL;


    _InitTenement();
    _InitMiddle();

    QTimer *Timer1 = new QTimer();
    connect(Timer1,SIGNAL(timeout()), this, SLOT(_UpdateWeather()) );
    Timer1->start(1000);

    QTimer *Timer2 = new QTimer();
    connect(Timer2,SIGNAL(timeout()), this, SLOT(_TimeSendCard()) );
    Timer2->start(1000);
}
//更新城市编码，并获取天气实时数据
void CUDPSERVER::_UpdateLoading(QString strCityNum)
{
    m_strCity = strCityNum;
    _QueryNowWeather();
    _QueryDayWeather();
}
//初始化成员变量
void CUDPSERVER::_Init()
{
    m_nAllTenement = 0;
    m_nAllMiddle = 0;
    m_nAllTenementOnline = 0;
    m_nAllMiddleOnline = 0;
    m_nPage = 0;
    m_nAlarm = 0;
    memset(m_gHead,0,6);
    memset(m_gMark,0,20);
    memset(m_gName,0,20);
    memcpy(m_gHead,"XXXCID",6);
    memcpy(m_gMark,"hikdsdkkkkdfdsIMAGE",19);
    m_strLocalIP = QNetworkInterface::allAddresses().first().toString();;
    m_strMulticastIP = "238.9.9.1";

    //Weather
    sWeather.nWeather = 0;
    sWeather.nMaxTemp = 0;
    sWeather.nMinTemp = 0;
    m_NetworkManager = new QNetworkAccessManager(this);
    m_NetworkManager1 = new QNetworkAccessManager(this);

    nSendId = nAallfile = nFinishfiles = nPacklen = 0;
    nCheckId = nSendPack = nLength = nFailFile = nAskTime = 0;
}
//获取文件名
void CUDPSERVER::_Path()
{
    strPath = Files.at(nFinishfiles + nFailFile);
    int i = strPath.lastIndexOf('\\');
    if(i == -1){
      i = strPath.lastIndexOf('/');
    }
    int j = strPath.length();
    QString t_strName = strPath.right(j-i-1);
    memset(m_gName,0,20);
    memcpy(m_gName,(t_strName.toAscii().data()),t_strName.length());
}
//下载文件 发送开始下载命令
void CUDPSERVER::_LoadDown(QString strIp)
{
    this->_Path();
    if(File.isOpen()) File.close();
    File.setFileName(strPath);
    if (File.open(QIODevice::ReadOnly))
    {
        nLength = File.size();
        nPacklen = nLength/8000+1;
        nSendPack = 0;
        if(_Begin(strIp))//发送开始成功
        {
            int i = 0;
            emit Sigbtn(i);
            emit SigNewsDonw(nCheckId, nAallfile, nFinishfiles);
            nAskTime = ASK_MESSAGE_TIME;
        }else{//发送开始失败
            _Fail(strIp);
        }
    }else{
        _Fail(strIp);
    }
}
//下载失败
void CUDPSERVER::_Fail(QString strIp)
{
    nFailFile ++;
    nAskTime =0;
    if(File.isOpen()) File.close();
    //发送信号
    emit SigNewsDonw(nCheckId, nAallfile, nFinishfiles);
    if(nAallfile == (nFinishfiles + nFailFile))
    {
        _SendNext();
    }else{
        _LoadDown(strIp);
    }
}
//向下一个选中的设备发送文件
void  CUDPSERVER::_SendNext()
{
    if(File.isOpen()) File.close();
    nFinishfiles = nFailFile = 0;
    memset(m_gName,0,20);
    NodeMiddle *p = m_MiddleItem;
    int i = 0;
    while(p->next)
    {
        if(p->data.nCheckedState == 1)
        {
            i++;
            if(nSendId == (i-1))
            {
                nCheckId = p->data.nId;
                QString strIP = QString(p->data.gcIpAddr);
                _LoadDown(strIP);
                nSendId++;
                return;
            }
        }
        p = p->next;
    }
    int k = 1;
    emit Sigbtn(k);
}
//开始下载文件 的指令
int CUDPSERVER::_Begin(QString strIp)
{
    unsigned char pSend[60]={0};
    memcpy(pSend,m_gHead,6);
    pSend[6] = 223;
    pSend[7] = 1;
    pSend[8] = 1;
    memcpy(pSend+9,m_gMark,19);
    memcpy(pSend+29,m_gName,20);
    if(!_FileMsg(pSend)){
        return 0;
    }
    pSend[57] = 0;
    pSend[58] = 0;
    int nSendLen = 59;
    return _Sendto(strIp,pSend,nSendLen);
}
//发送文件的数据的分包
int CUDPSERVER::_FileMsg(unsigned char *pSend)
{
    if(nLength <= 0XFFFFFFFF){
            pSend[49] = nLength&0XFF;
            pSend[50] = (nLength>>8)&0XFF;
            pSend[51] = (nLength>>16)&0XFF;
            pSend[52] = (nLength>>24)&0XFF;
        }else{
            QMessageBox::warning(NULL,tr("提示"),tr("文件太大"));
            return 0;
        }
        //总包数
         if(nPacklen <= 0XFFFF){
            pSend[53] = nPacklen&0XFF;
            pSend[54] = (nPacklen>>8)&0XFF;
        }else{
            QMessageBox::warning(NULL,tr("提示"),tr("超过总包数最大值"));
            return 0;
        }
        //当前包数
        if(nSendPack <= 0XFFFF){
            pSend[55] = nSendPack&0XFF;
            pSend[56] = (nSendPack>>8)&0XFF;
        }else{
            QMessageBox::warning(NULL,tr("提示"),tr("超过当前包数最大值"));
            return 0;
        }
        return 1;
}
//读取软件有效期配置
void CUDPSERVER::_ReadConfig()
{
    char buf[12] = {0};
    ::GetPrivateProfileStringA("Flag","STime","sa",buf,12,"C:\\WINDOWS\\UTSet.ini");
    QDate Date = QDate::fromString(QString(buf),"yyyy-MM-dd");
    if(Date.isNull()){
        emit SigDateTime(1,0);
        return;
    }
    QDate NowDate = QDateTime::currentDateTime().date();
    int AllDay = Date.daysTo(NowDate);
    int  nFlag = ::GetPrivateProfileIntA("Flag","idex",0,"C:\\WINDOWS\\UTSet.ini");
    int nMonth = ::GetPrivateProfileIntA("Flag","month",0,"C:\\WINDOWS\\UTSet.ini");
    if(nFlag==0){
        emit SigDateTime(1,0);
        return;
    }else if(nFlag==1){
        if(AllDay < (::GetPrivateProfileIntA("1","AllDay",0,"C:\\WINDOWS\\UTSet.ini")))
        {
            emit SigDateTime(1,0);
            return;
        }else{
            if(AllDay >=60 &&AllDay <70){
                if(::GetPrivateProfileIntA("1","Count",0,"C:\\WINDOWS\\UTSet.ini") == 0)
                {
                    emit SigDateTime(2,AllDay);
                    ::WritePrivateProfileStringA("1", "Count", QString::number(1).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
                }
            }else if(AllDay >=70 &&AllDay <80){
                if(::GetPrivateProfileIntA("1","Count",0,"C:\\WINDOWS\\UTSet.ini") <= 1)
                {
                    emit SigDateTime(3,AllDay);
                    ::WritePrivateProfileStringA("1", "Count", QString::number(2).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
                }
            }else if(AllDay >= (80 + 30*nMonth)){
                    emit SigDateTime(4,AllDay);
            }
            ::WritePrivateProfileStringA("1", "AllDay", QString::number(AllDay).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
        }
    }else if(nFlag==2){
        if(AllDay < (::GetPrivateProfileIntA("2","AllDay",0,"C:\\WINDOWS\\UTSet.ini")))
        {
            emit SigDateTime(1,0);
            return;
        }else{
            if(AllDay >=750 &&AllDay <780){
                if(::GetPrivateProfileIntA("2","Count",0,"C:\\WINDOWS\\UTSet.ini") == 0)
                {
                    emit SigDateTime(2,AllDay);
                    ::WritePrivateProfileStringA("2", "Count", QString::number(1).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
                }
            }else if(AllDay >=780 &&AllDay <810){
                if(::GetPrivateProfileIntA("2","Count",0,"C:\\WINDOWS\\UTSet.ini") <= 1)
                {
                    emit SigDateTime(3,AllDay);
                    ::WritePrivateProfileStringA("2", "Count", QString::number(2).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
                }
            }else if(AllDay >= (810 + 30*nMonth) ){
                    emit SigDateTime(4,AllDay);
            }
            ::WritePrivateProfileStringA("2", "AllDay", QString::number(AllDay).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
        }
    }
}

//2015 05 05
//重启设备命令生成
void CUDPSERVER::_Reset(QString strIp,QString strAddr,QString strType)
{
    unsigned char pSend[28] = {0};
    QString Addr;
    Addr.clear();
    memcpy(pSend,m_gHead,6);
    pSend[6] = 13;
    pSend[7] = 1;

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
        pSend[8] = 83;
    }else     if(strType == "别墅室内机")
    {
        pSend[8] = 66;
    }else     if(strType == "门口机")
    {
        pSend[8] = 77;
    }else     if(strType == "围墙机")
    {
        pSend[8] = 87;
    }else     if(strType == "中心机")
    {
        pSend[8] = 90;
    }else    if(strType == "二次门口机")
    {
        pSend[8] = 72;
    }
    QByteArray byte;
    byte = Addr.toUtf8();
    memcpy(pSend+9,byte.data(),byte.size());
    _Sendto(strIp,pSend,28);
}

//2015 03 26
//发送下载IC卡数据
void CUDPSERVER::_SendDownCard()
{
    NodeMiddle *q = m_MiddleItem;
    while(q->next)
    {
        if(q->data.nState && q->data.nCardState == 1)
        {
        QString  strIntraAddr = QString(q->data.gcIntraAddr);
        QByteArray m_Byte;
        m_Byte.clear();
        m_Byte = strIntraAddr.toUtf8();
        int nAllCard = m_Mysql._SelectAllCard(strIntraAddr);
//        char *buf = new char[CARD_MAXLEN*5];
//        memset(buf,0,CARD_MAXLEN*5);
        char buf[CARD_MAXLEN*5] = {0};
        QString str,str1;
        if(nAllCard > CARD_MAXLEN )
        {
            QDateTime dateTime=QDateTime::currentDateTime();
            QString strDtime = dateTime.toString("yyyy-MM-dd");
            QString SQL = "select IC_Card_num from ic_card where IC_Card_time > '" + strDtime + "' and " + strIntraAddr + "= 1 ";
//            QSqlQuery query;
            QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
            QSqlQuery query(db);
            if(query.exec(SQL)){
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
            _DownAllCard(m_Byte,buf,nAllCard,CARD_MAXLEN,1);
            }
        CCONNECTIONPOOLMYSQL::closeConnection(db);
        }else {
            QDateTime dateTime=QDateTime::currentDateTime();
            QString strDtime = dateTime.toString("yyyy-MM-dd");
            QString SQL = "select IC_Card_num from ic_card where IC_Card_time > '" + strDtime + "' and " + strIntraAddr + "= 1 ";
//            QSqlQuery query;
            QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
            QSqlQuery query(db);
            if(query.exec(SQL)){
            int i = 0;
            while(query.next())
            {
                str.clear();
                str1.clear();
                str = QString(query.value(0).toByteArray().data());;
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
            _DownAllCard(m_Byte,buf,nAllCard,nAllCard,1);
            }
            CCONNECTIONPOOLMYSQL::closeConnection(db);
        }
        }
        q = q->next;
    }
}
//有新卡过期或者以前下载未成功，发送上传IC卡指令
void CUDPSERVER::_SendUpCard()
{
    NodeMiddle *q = m_MiddleItem;
    if(m_Mysql._IsExpireCard())
    {
        //清空链表
        while(q->next != NULL)
        {
            if(q->data.nState)
            {
            QString  strIntraAddr = QString(q->data.gcIntraAddr);
            QByteArray byte;
            byte = strIntraAddr.toUtf8();
            _UpCard(byte);
            }
            q->data.nCardState = 0;
            q = q->next;
        }
    }else{
        //发送不为2
        while(q->next != NULL)
        {
            if(q->data.nState && q->data.nCardState != 2)
            {
            QString  strIntraAddr = QString(q->data.gcIntraAddr);
            QByteArray byte;
            byte = strIntraAddr.toUtf8();
            _UpCard(byte);
            }
            q = q->next;
        }
    }
}

void CUDPSERVER::_TimeSendCard()
{
    QDateTime dateTime=QDateTime::currentDateTime();
    int hour = dateTime.time().hour();
    int minute = dateTime.time().minute();
    int second = dateTime.time().second();
    if(hour == 1 )//凌晨时判断上传和下发IC卡
    {
//        if(minute == 0 && second == 0) _SendUpCard();

        if(minute == 10 && second == 1){
            _SendDownCard();
        }
    }
}
//定时器刷新
void CUDPSERVER::_UpdateWeather()
{
    QDateTime dateTime=QDateTime::currentDateTime();
    int hour = dateTime.time().hour();
    int minute = dateTime.time().minute();
    int second = dateTime.time().second();

    if((minute == 5) && (second == 1))//每小时重新获取天气和有效期
    {
        _QueryNowWeather();
        _QueryDayWeather();
        _ReadConfig();
    }
    NodeTenement *q;
    q = m_TenementItem;
    while(q->next)
    {
//        qDebug()<<"shin"<<QString(q->data.gcIntraAddr)<<q->data.nState;
        if(q->data.nState > 0)//在线监测
        {
            q->data.nState --;
            if(q->data.nState <= 0)
            {
                emit SigOnlineTenement(q->data.nId,q->data.nAlarmState);
                m_Mysql._OnlineTenement(q->data);
            }
        }

        if(q->data.nCheckedState >0)
        {
            if(q->data.nAskTime >0)//普通信息应答监测
            {
                q->data.nAskTime --;
                if(q->data.nAskTime == 0)
                {
                    emit SigSendMessage(q->data.nId,0);
                }
            }
        }
        q = q->next;
    }
    NodeMiddle *p;
    p = m_MiddleItem;
    while(p->next)
    {
//        qDebug()<<"menk"<<QString(p->data.gcIntraAddr)<<p->data.nState;
        if(p->data.nState > 0)//在线监测
        {
            p->data.nState --;
            if(p->data.nState <= 0)
            {
                if((m_nPage == 2) || (m_nPage == 12))
                    emit SigOnlineMiddle(p->data.nId);
                m_Mysql._OnlineMiddle(p->data);
            }
        }
        p = p->next;
    }

    if(nAskTime > 0)//新闻视频下发监测
    {
        nAskTime --;
        if(nAskTime == 0)
        {
        emit SigNewsDonw(nCheckId, nAallfile, nFinishfiles);
        _SendNext();
        }
    }
}



//线程启动，绑定udp
void CUDPSERVER::run()
{
    //进行版本协商，加载特定版本的socket动态链接库
    int retVal;
    int port = 8300;
    if((retVal = WSAStartup(MAKEWORD(2,2), &wsd)) != 0)
    {
        perror("socket");
//        qDebug() << "WSAStartup() failed";
        return ;
    }
    Fsocket = socket(PF_INET,SOCK_DGRAM,IPPROTO_IP);
    if(Fsocket == INVALID_SOCKET)
    {
        emit SigUdpOpenFail();
        perror("socket");
//        qDebug()<<WSAGetLastError();
        return;
    }
    bool bOptVal=FALSE;
    retVal = setsockopt(Fsocket,SOL_SOCKET,SO_BROADCAST,(char*)&bOptVal,sizeof(bool));//可以发生广播
    struct sockaddr_in FAddr;
    memset(&FAddr,0,sizeof(FAddr));
    FAddr.sin_family = AF_INET;
    FAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    FAddr.sin_port = htons(port);
    if(bind(Fsocket,(sockaddr*)&FAddr,sizeof(FAddr))==SOCKET_ERROR)
    {
        emit SigUdpOpenFail();
        perror("socket");
//        qDebug() <<WSAGetLastError();
        return;
    }
    struct ip_mreq imreq;
    imreq.imr_multiaddr.s_addr = inet_addr(m_strMulticastIP.toLatin1().data());//inet_addr("238.9.9.1");
    imreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if(setsockopt(Fsocket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&imreq,sizeof(ip_mreq))==SOCKET_ERROR)
    {
        emit SigUdpOpenFail();
        perror("socket");
//        qDebug() <<WSAGetLastError();
        return;
    }
    while(1)
    {
        _Read();
    }
}
//接收到的数据
int CUDPSERVER::_Read()
{
    struct sockaddr_in from;
    int nLength=sizeof(struct sockaddr_in);
    unsigned char buf[8100] = {0};
    char gFromIp[20] = {0};
    int t_nRecvlen =::recvfrom(this->Fsocket,(char*)buf,8100,0,(struct sockaddr * )&from,&nLength);
    if(!t_nRecvlen)
    {
//        qDebug() <<WSAGetLastError();
        return 0;
    }
    strcpy(gFromIp,inet_ntoa(from.sin_addr));
    _Recv(buf,gFromIp,t_nRecvlen);
    return t_nRecvlen;
}

//获取实时天气
void CUDPSERVER::_QueryNowWeather()
{
    QString url = "http://www.weather.com.cn/weather1d/"+m_strCity+".shtml";//"http://www.weather.com.cn/data/sk/"+m_strCity+".html";
//       qDebug()<<url<<"2222222222";
    m_NetworkRequest1.setUrl(QUrl(url));
    m_NetworkReply1 = m_NetworkManager1->get(m_NetworkRequest1);
    connect(m_NetworkReply1, SIGNAL(readyRead()), this, SLOT(SlotReadyRead1()));
}
//获取当天天气
void CUDPSERVER::_QueryDayWeather()
{
    QString url = "http://www.weather.com.cn/data/cityinfo/"+m_strCity+".html";
    m_NetworkRequest.setUrl(QUrl(url));
    m_NetworkReply = m_NetworkManager->get(m_NetworkRequest);
    connect(m_NetworkReply, SIGNAL(readyRead()), this, SLOT(SlotReadyRead()));
}

//获取天气到的数据的解析
void CUDPSERVER::SlotReadyRead()
{
    QString strXmlFile = m_NetworkReply->readAll();
//    qDebug()<<strXmlFile<<"22222222222";
    int t_nLen = strXmlFile.indexOf(":",0) + 1;
    if(t_nLen)
    {
        strXmlFile = strXmlFile.right(strXmlFile.length()-t_nLen);
        strXmlFile = strXmlFile.left(strXmlFile.length()-1);
        QScriptEngine engine;
        QScriptValue sc = engine.evaluate("value="+strXmlFile);
        QString temp = sc.property("temp2").toString();
        sWeather.nMinTemp = temp.left(temp.length()-1).toInt();
        temp.clear();
        temp = sc.property("temp1").toString();
        sWeather.nMaxTemp = temp.left(temp.length()-1).toInt();


//        qDebug()<<sWeather.nMinTemp<<sWeather.nMaxTemp;
        QString str = sc.property("weather").toString();
        if(str.indexOf("雪",0) == -1)
        {
            if(str.indexOf("雨",0) == -1)
            {
                if(str.indexOf("多云",0) == -1)
                {
                    if(str.indexOf("晴",0) == -1)
                    {
                        sWeather.nWeather = 0;
                    }else sWeather.nWeather = 1;
                }else sWeather.nWeather = 2;
            }else sWeather.nWeather = 3;
        }else sWeather.nWeather = 4;
    }
}
//获取天气到的数据的解析
void CUDPSERVER::SlotReadyRead1()
{
    QString strXmlFile = m_NetworkReply1->readAll();
//    qDebug()<<strXmlFile<<"1111111111111111";
//    int t_nLen = strXmlFile.indexOf(":",0) + 1;value
    int t_nLen = strXmlFile.indexOf("input type=\"hidden\" id=\"hidden_title\" value=",0) + 44;
    QString str = strXmlFile.right(strXmlFile.length() - t_nLen).left(30);

    if(str.indexOf("雪",0) == -1)
    {
        if(str.indexOf("雨",0) == -1)
        {
            if(str.indexOf("多云",0) == -1)
            {
                if(str.indexOf("晴",0) == -1)
                {
                    sWeather.nWeather = 0;
                }else sWeather.nWeather = 1;
            }else sWeather.nWeather = 2;
        }else sWeather.nWeather = 3;
    }else sWeather.nWeather = 4;

    t_nLen = str.indexOf("°C",0);
    int t_nLen1 = str.indexOf("/",0);
    if(t_nLen&&t_nLen1)
    {
        sWeather.nMinTemp = str.left(t_nLen).right(t_nLen - t_nLen1 - 1).toInt();
        sWeather.nMaxTemp = str.left(t_nLen1).right(2).toInt();
//        qDebug()<<t_nLen1<<t_nLen<<str.length()<<str.left(t_nLen).right(t_nLen - t_nLen1 - 1)<<str.left(t_nLen1).right(2);
    }

    emit SigUpdateWeatherNew();
//    qDebug()<<str<<sWeather.nMinTemp<<sWeather.nMaxTemp<<sWeather.nWeather;

//    if(t_nLen)
//    {
//        strXmlFile = strXmlFile.right(strXmlFile.length()-t_nLen);
//        strXmlFile = strXmlFile.left(strXmlFile.length()-1);
//        QScriptEngine engine;
//        QScriptValue sc = engine.evaluate("value="+strXmlFile);
//        sWeather.strName = sc.property("city").toString(); //解析字段
//        sWeather.nNowTemp = sc.property("temp").toInteger();
//        QString str =  sc.property("SD").toString();
//        str = str.left(str.length()-1);
//        sWeather.nHumidity = str.toInt();
//        sWeather.strWindDirection = sc.property("WD").toString();
//        str.clear();
//        str = sc.property("WS").toString();
//        sWeather.nWind = str.left(str.length()-1).toInt();
//    }else{
//        sWeather.nNowTemp = 0;
//        sWeather.nWind = 0;
//        sWeather.nHumidity = 0;
//    }

}
//初始化链表
void CUDPSERVER::_InitTenement()
{
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    QString SQL = "SELECT * FROM tenement ORDER BY Tenement_intra_addr DESC";
//    QSqlQuery query;
    int i = query.exec(SQL);
//    qDebug()<<db.connectionName()<<i<<SQL<<"111111";
    if(i)
    {
        m_nAllTenement = 0;
        while(query.next())
        {
            STenementDevice sTenementDevice;
            memset(sTenementDevice.gcAddr,0,20);
            memset(sTenementDevice.gcAddrExplain,0,40);
            memset(sTenementDevice.gcIntraAddr,0,20);
            memset(sTenementDevice.gcIpAddr,0,20);
            memset(sTenementDevice.gcMacAddr,0,20);
            memset(sTenementDevice.gcName,0,64);
            memset(sTenementDevice.gcPhone1,0,20);
            memset(sTenementDevice.gcPhone2,0,20);
            memset(sTenementDevice.gcType,0,20);
            sTenementDevice.nAlarmState = 0;
            sTenementDevice.nAskTime = 0;
            sTenementDevice.nCheckedState = 0;
            sTenementDevice.nFenceState = 0;
            sTenementDevice.nId = 0;
            sTenementDevice.nState = 0;

            sTenementDevice.nId = m_nAllTenement;
            strcpy(sTenementDevice.gcAddr,query.value(1).toByteArray().data());
            strcpy(sTenementDevice.gcAddrExplain,query.value(2).toByteArray().data());
            strcpy(sTenementDevice.gcIntraAddr,query.value(10).toByteArray().data());
            strcpy(sTenementDevice.gcIpAddr,query.value(7).toByteArray().data());
            strcpy(sTenementDevice.gcMacAddr,query.value(6).toByteArray().data());
            strcpy(sTenementDevice.gcName,query.value(3).toByteArray().data());
            strcpy(sTenementDevice.gcPhone1,query.value(4).toByteArray().data());
            strcpy(sTenementDevice.gcPhone2,query.value(5).toByteArray().data());
            strcpy(sTenementDevice.gcType,query.value(8).toByteArray().data());
            sTenementDevice.nFenceState = query.value(9).toInt();
            m_TenementItem = ItemTenementCreatH(sTenementDevice,m_TenementItem);
            m_nAllTenement++;
        }
    }else{
        QMessageBox::warning(NULL,tr("提示"),tr("11数据库查询失败，请检查数据库连接"));
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//初始化中间设备链表
void CUDPSERVER::_InitMiddle()
{
    QString SQL = "SELECT * FROM middle ORDER BY Middle_intra_addr DESC";
//    QSqlQuery query;
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QSqlQuery query(db);
    int i = query.exec(SQL);
    if(i)
    {
        m_nAllMiddle = 0;
        while(query.next())
        {
            SMiddleDevice sMiddleDevice;
            memset(sMiddleDevice.gcAddr,0,20);
            memset(sMiddleDevice.gcAddrExplain,0,40);
            memset(sMiddleDevice.gcIntraAddr,0,20);
            memset(sMiddleDevice.gcIpAddr,0,20);
            memset(sMiddleDevice.gcMacAddr,0,20);
            memset(sMiddleDevice.gcOldAddr,0,64);
            memset(sMiddleDevice.gcType,0,20);

            sMiddleDevice.nAskTime = 0;
            sMiddleDevice.nCheckedState = 0;
            sMiddleDevice.nId = 0;
            sMiddleDevice.nState = 0;
            sMiddleDevice.nCardState = 0;

            sMiddleDevice.nId = m_nAllMiddle;
            memcpy(sMiddleDevice.gcAddr,query.value(1).toByteArray().data(),query.value(1).toByteArray().size());
            memcpy(sMiddleDevice.gcAddrExplain,query.value(2).toByteArray().data(),query.value(2).toByteArray().size());
            memcpy(sMiddleDevice.gcIntraAddr,query.value(6).toByteArray().data(),query.value(6).toByteArray().size());
            memcpy(sMiddleDevice.gcIpAddr,query.value(4).toByteArray().data(),query.value(4).toByteArray().size());
            memcpy(sMiddleDevice.gcMacAddr,query.value(3).toByteArray().data(),query.value(3).toByteArray().size());
            memcpy(sMiddleDevice.gcType,query.value(5).toByteArray().data(),query.value(5).toByteArray().size());
            m_MiddleItem = ItemMiddleCreatH(sMiddleDevice,m_MiddleItem);
            m_nAllMiddle++;
        }
    }else{
        QMessageBox::warning(NULL,tr("提示"),tr("数据库查询失败，请检查数据库连接"));
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

//接收数据的分析分流
void CUDPSERVER::_Recv(unsigned char *buf,char *pFromIP,int nRecvLen)
{
//    qDebug()<<nRecvLen<<"接收";
    if(memcmp(buf,m_gHead,6) == 0)
    {
        switch(buf[7])
        {
        case 1:
            switch(buf[6])
            {
            case 1:
                _DeviceAlarm(buf,pFromIP);
                break;
            case 2:
                _CancelAlarm(buf,pFromIP);
                break;
            case 4://设备在线定时回复
                _DeviceOnline(buf,pFromIP);
                break;
            case 5:
            {
                unsigned char pbuf[8100] = {0};
                memcpy(pbuf,buf,8100);
                emit _test(pbuf);
                break;
            }
            case 8:
                _Repairs(buf,pFromIP);
                break;
            case 9:
                _CancelRepairs(buf,pFromIP);
                break;
            case 25:
                _MiddleAlarm(buf,pFromIP);
                break;
            case 73://刷卡
                _SwipeCard(buf,pFromIP);
                break;
            case 225://刷卡
                _UpCallPic(buf,pFromIP);
                break;
            default:
                break;
            }
            break;
        case 2:
            switch(buf[6])
            {
            case 3:
                _RecvSendMessage(buf,pFromIP);
                break;
            case 7://电话报修回复
                break;
            case 13:
                emit SigReset();
                break;
            case 30://初始化密码回复
                _RecvInitPass(buf,pFromIP);
                break;
            case 40:
                _WrtieAddr(buf,pFromIP);
                break;
            case 41:
                _ReadAddr(buf,pFromIP);
                break;
            case 70://上传卡回应
                _RecvUpCard(buf,pFromIP);
                break;
            case 71:
                _RecvDownAloneCard(buf,pFromIP);
                break;
            case 72:
                _RecvDownAllCard(buf,pFromIP);
                break;
            case 74:
                _RecvLogOutCard(buf,pFromIP);
                break;
            case 75://下发卡回应
                _RecvDownCard(buf,pFromIP);
                break;
            case 223:
            {
                if((memcmp(buf+9,m_gMark,19) == 0) &&(memcmp(buf+29,m_gName,20) == 0))
                {
                    switch(buf[8])
                    {
                    case  1:
                        if(_SendData(buf,pFromIP) == 0)
                        {
                            QString strIp = QString(pFromIP);
                            _Fail(strIp);
                        }else{
                            nAskTime = ASK_MESSAGE_TIME;
                        }
                        break;
                    case  2:
                        {
                        int nRecvPack = buf[55] +buf[56]*256;
                        if(nRecvPack == nSendPack)
                        {
                            if(nRecvPack == (nPacklen - 1))
                            {
                                if(_End(buf,pFromIP))
                                {
                                nAskTime = ASK_MESSAGE_TIME;
                                }else{
                                    QString strIp = QString(pFromIP);
                                    _Fail(strIp);
                                }
                            }else{
                                nSendPack ++;
                                if(_SendData(buf,pFromIP) == 0)
                                {
                                    QString strIp = QString(pFromIP);
                                    _Fail(strIp);
                                }else{
                                    nAskTime = ASK_MESSAGE_TIME;
                                }
                            }
                        }
                        break;
                        }
                    case  3:
                    {
                        nFinishfiles ++;
                        emit SigNewsDonw(nCheckId, nAallfile, nFinishfiles);
                        if((nFinishfiles + nFailFile) != nAallfile)
                        {
                            nAskTime = 0;
                            QString strIp = QString(pFromIP);
                            _LoadDown(strIp);
                        }else{
                            if(_EndAll(buf,pFromIP))
                            {
                                nAskTime = ASK_MESSAGE_TIME;
                            }else{
                                QString strIp = QString(pFromIP);
                                _Fail(strIp);
                            }
                        }
                        break;
                    }
                    case 20:
                        //一个目标全部下载完成，准备下一个目标
                        nAskTime = 0;
                        _SendNext();
                        break;
                    }
                }
            }
            break;
            default :
                break;
            }
            break;
        default :
            break;
        }
    }
}
//发送读设备地址指令
void CUDPSERVER::_ReadAddr(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char gSend[72] = {0};
    memcpy(gSend,buf,72);
    unsigned char buf1[8100] = {0};
    memcpy(buf1,buf,8100);
    emit SigRecvRead(buf1,strIp);
    int nSendlen = 72;
    _Sendto(strIp,gSend,nSendlen);
}
//接收到写设备地址指令
void CUDPSERVER::_WrtieAddr(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    emit SigRecvWrite(buf,strIp);

    char gOldIntraAddr[20] = {0};
    char gMacAddr[20] = {0};
    QString str;
    memcpy(gOldIntraAddr ,(char *)buf + 8,20);
    if((buf[29]>>1)&0X01)
    {
        switch(buf[8])
        {
        case 'M':
        case 'W':
        case 'Z':
        {
        NodeMiddle *q = m_MiddleItem;
        while(q->next)
        {
            if((memcmp(q->data.gcIntraAddr,gOldIntraAddr,20) ==0)&&(memcmp(q->data.gcIpAddr,pFromIP,strlen(pFromIP))==0))
            {
                for(int i = 0;i<6;i++)
                {
                    int kk = buf[50+i];
                    if(kk < 16) str = str + "0";
                    str = str + QString::number(kk, 16).toUpper();
                    if(i != 5) str = str + ":";
                }
                QByteArray byte;
                byte = str.toAscii();
                memcpy(gMacAddr,byte.data(),byte.size());
                QString SQL = "UPDATE middle set Middle_mac_addr = '"+QString(gMacAddr)+"' WHERE Middle_ip_addr =  '"+QString(pFromIP)+"' AND Middle_intra_addr =  '"+QString(gOldIntraAddr)+"'";
//                QSqlQuery query;
                QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
                QSqlQuery query(db);
                int ti = query.exec(SQL);
                if(ti)
                {
                    memcpy(q->data.gcMacAddr,gMacAddr,20);
                    emit SigUpdateMac();
                    emit SigUpdateAddr();
                    return;
                }
                CCONNECTIONPOOLMYSQL::closeConnection(db);

            }
            q = q->next;
        }
        break;
        }
        case 'S':
        {
            NodeTenement *q = m_TenementItem;
            while(q->next)
            {
                if((memcmp(q->data.gcIntraAddr,gOldIntraAddr,20) ==0)&&(memcmp(q->data.gcIpAddr,pFromIP,strlen(pFromIP))==0))
                {
                    for(int i = 0;i<6;i++)
                    {
                        int kk = buf[50+i];
                        if(kk < 16) str = str + "0";
                        str = str + QString::number(kk, 16).toUpper();
                        if(i != 5) str = str + ":";
                    }

                    QByteArray byte;
                    byte = str.toAscii();
                    memcpy(gMacAddr,byte.data(),byte.size());
                    QString SQL = "UPDATE tenement set Tenement_mac_addr = '"+QString(gMacAddr)+"' WHERE Tenement_ip_addr =  '"+QString(pFromIP)+"' AND Tenement_intra_addr =  '"+QString(gOldIntraAddr)+"'";
//                    QSqlQuery query;
                    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
                    QSqlQuery query(db);
                    int ti = query.exec(SQL);
                    if(ti)
                    {
                        memcpy(q->data.gcMacAddr,gMacAddr,20);
                        emit SigUpdateMac();
                        emit SigUpdateAddr();
                        return;
                    }
                    CCONNECTIONPOOLMYSQL::closeConnection(db);
                }
                q = q->next;
            }
            break;
        }
        }
    }
    emit SigUpdateAddr();
}
//发送一个发送完成指令
int CUDPSERVER::_End(unsigned char *buf,char *pFromIP)
{
    unsigned char pSend[59] = {0};
    int size = 59;
    memcpy(pSend,buf,59);
    pSend[7] = 1;
    pSend[8] = 3;
    pSend[57] = 0;
    pSend[58] = 0;
    QString strIp = QString(pFromIP);
    return _Sendto(strIp,pSend,size);
}
//发送全部发送完成指令
int CUDPSERVER::_EndAll(unsigned char *buf,char *pFromIP)
{
    unsigned char pSend[59] = {0};
    int size = 59;
    memcpy(pSend,buf,59);
    pSend[7] = 1;
    pSend[8] = 20;
    pSend[57] = 0;
    pSend[58] = 0;
    QString strIp = QString(pFromIP);
    return _Sendto(strIp,pSend,size);
}
//发送文件的分包数据指令
int CUDPSERVER::_SendData(unsigned char *buf,char *pFromIP)
{
    unsigned char pSend[8100] = {0};
    memcpy(pSend,buf,55);
    int size = 0;
    if((nPacklen-1) == nSendPack )
    {
        size = File.size() - (nPacklen-1)*8000;
    }else{
        size = 8000;
    }
    QByteArray line = File.read(size);
    pSend[7] = 1;
    pSend[8] = 2;
    if(!_FileMsg(pSend)){
        return 0;
    }
    if(size < 0XFFFF)
    {
    pSend[57] = size&0XFF;
    pSend[58] = (size>>8)&0XFF;
    }else{
        QMessageBox::warning(NULL,tr("提示"),tr("超过包数据最大值"));
        return 0;
    }
    memcpy(pSend+59,line.data(),size);
    size = size + 59;
    QString strIp = QString(pFromIP);
    return _Sendto(strIp,pSend,size);
}
//udp发送
int CUDPSERVER::_Sendto(QString strIp,unsigned char *pSend,int nSendlen)
{
    unsigned long nAddr=inet_addr(strIp.toStdString().data());
    To.sin_family=AF_INET;
    To.sin_port=htons(8300);
    To.sin_addr.S_un.S_addr=(int)nAddr;
    if(::sendto(this->Fsocket,(char*)pSend,nSendlen,0,(struct sockaddr*)&To,sizeof(struct sockaddr))==SOCKET_ERROR)
    {
        qDebug() <<WSAGetLastError();
        return 0;
    }
    return 1;
}
//收到设备在线数据
void CUDPSERVER::_DeviceOnline(unsigned char *buf,char *pFromIP)
{
    switch(buf[8])
    {
    case 'M':
    case 'W':
    case 'Z':
    case 'H':
        {
        SMiddleDevice sMiddleDevice;
        memset(sMiddleDevice.gcAddr,0,20);
        memset(sMiddleDevice.gcAddrExplain,0,40);
        memset(sMiddleDevice.gcIntraAddr,0,20);
        memset(sMiddleDevice.gcIpAddr,0,20);
        memset(sMiddleDevice.gcMacAddr,0,20);
        memset(sMiddleDevice.gcOldAddr,0,20);
        memset(sMiddleDevice.gcType,0,20);

        sMiddleDevice.nAskTime = 0;
        sMiddleDevice.nCheckedState = 0;
        sMiddleDevice.nId = 0;
        sMiddleDevice.nState = 0;
        memcpy(sMiddleDevice.gcIntraAddr,(char *)buf + 8,20);
        memcpy(sMiddleDevice.gcIpAddr,pFromIP,strlen(pFromIP));
        QString strIntra = QString(sMiddleDevice.gcIntraAddr);
        QString str;

        for(int i = 0;i<6;i++)
        {
            int k = buf[28+i];
            if(k < 16) str = str + "0";
            str = str + QString::number(k, 16).toUpper();
            if(i != 5) str = str + ":";
        }

        QByteArray byte;
        byte = str.toAscii();

        memcpy(sMiddleDevice.gcMacAddr,byte.data(),byte.size());

        _IntraToAddr(sMiddleDevice.gcIntraAddr,sMiddleDevice.gcAddr,sMiddleDevice.gcType);
        _IntraToAddrExplain(strIntra,sMiddleDevice.gcAddrExplain);


        int nMark = ItemMiddleFind(&sMiddleDevice,m_MiddleItem);
        switch(nMark)
        {
        case -3:
            if(m_Mysql._UpdateMiddle(nMark,sMiddleDevice))
            {
            m_MiddleItem = DeleteItemMiddle(sMiddleDevice,&m_MiddleItem);
            m_MiddleItem = InsertItemMiddle(sMiddleDevice,m_MiddleItem);
            if((m_nPage == 2) || (m_nPage == 12))
                emit SigInitMiddle();
            }
            break;
        case -2:
            break;
        case -1:
            if(m_Mysql._InsertMiddle(sMiddleDevice))
            {
            m_MiddleItem = InsertItemMiddle(sMiddleDevice,m_MiddleItem);
            m_nAllMiddle++;
            if((m_nPage == 2) || (m_nPage == 12))
                emit SigInitMiddle();
            }
            break;
        default :
            if(m_Mysql._UpdateMiddle(nMark,sMiddleDevice))
            {
            m_MiddleItem = UpdateItemMiddle(sMiddleDevice,m_MiddleItem);
            if((m_nPage == 2) || (m_nPage == 12))
            {
            QVariant var1;
            var1.setValue(sMiddleDevice);
            emit SigUpdateMiddle(var1);
            }
            }
            break;
        }
        }

        break;
    case 'S':
    {
    STenementDevice sTenementDevice;
    memset(sTenementDevice.gcAddr,0,20);
    memset(sTenementDevice.gcAddrExplain,0,40);
    memset(sTenementDevice.gcName,0,64);
    memset(sTenementDevice.gcPhone1,0,20);
    memset(sTenementDevice.gcPhone2,0,20);
    memset(sTenementDevice.gcIntraAddr,0,20);
    memset(sTenementDevice.gcIpAddr,0,20);
    memset(sTenementDevice.gcMacAddr,0,20);
    memset(sTenementDevice.gcType,0,20);

    sTenementDevice.nAskTime = 0;
    sTenementDevice.nCheckedState = 0;
    sTenementDevice.nId = 0;
    sTenementDevice.nState = 0;
    sTenementDevice.nFenceState = 0;
    sTenementDevice.nAlarmState = 0;
    sTenementDevice.nOldFenceState = 0;
    memcpy(sTenementDevice.gcIntraAddr,(char *)buf + 8,20);
    memcpy(sTenementDevice.gcIpAddr,pFromIP,strlen(pFromIP));

    QString strIntra = QString(sTenementDevice.gcIntraAddr);
    QString str;

    for(int i = 0;i<6;i++)
    {
        int k = buf[28+i];
        if(k < 16) str = str + "0";
        str = str + QString::number(k, 16).toUpper();
        if(i != 5) str = str + ":";
    }

    QByteArray byte;
    byte = str.toAscii();

    memcpy(sTenementDevice.gcMacAddr,byte.data(),byte.size());

    memcpy(sTenementDevice.gcName,(char *)buf + 96,64);
    memcpy(sTenementDevice.gcPhone1,(char *)buf + 160,13);
    memcpy(sTenementDevice.gcPhone2,(char *)buf + 173,12);
    sTenementDevice.nFenceState = buf[34];

    _IntraToAddr(sTenementDevice.gcIntraAddr,sTenementDevice.gcAddr,sTenementDevice.gcType);
    _IntraToAddrExplain(strIntra,sTenementDevice.gcAddrExplain);

    int nMark = ItemTenementFind(&sTenementDevice,m_TenementItem);
    switch(nMark)
    {
    case -3:
        if(m_Mysql._UpdateTenement(sTenementDevice))
        {
        m_TenementItem = DeleteItemTenement(sTenementDevice,&m_TenementItem);
        m_TenementItem = InsertItemTenement(sTenementDevice,m_TenementItem);
        if((m_nPage == 1) || (m_nPage ==10))
        {
        emit SigInitTenement();
        }
        }
        break;
    case -2:
        break;
    case -1:
        if(m_Mysql._InsertTenement(sTenementDevice))
        {
        m_TenementItem = InsertItemTenement(sTenementDevice,m_TenementItem);
        m_nAllTenement++;
        if((m_nPage == 1) || (m_nPage ==10))
        {
        emit SigInitTenement();
        }
        }
        break;
    default :
        if(m_Mysql._UpdateTenement(sTenementDevice))
        {
        m_TenementItem = UpdateItemTenement(sTenementDevice,m_TenementItem);
        QVariant var1;
        var1.setValue(sTenementDevice);
        if((m_nPage == 1) || (m_nPage ==10))
        {
        emit SigUpdateTenement(var1);
        }
        }
        break;
    }
    }
        break;
    default :
        break;
    }
    _AskDeviceOnline(buf,pFromIP);
}
//发送普通数据指令
int CUDPSERVER::_SendMessage(QString strMessage)
{
    char pSend[1024] = {0};
    QByteArray byte = strMessage.toUtf8();
    NodeTenement *q;
    q = m_TenementItem;
    int ncheck = 0;
    while(q->next)
    {
        if(q->data.nCheckedState == 1)
        {
            ncheck ++;
            memset(pSend,0,1024);
            memcpy(pSend,m_gHead,6);
            pSend[6] = 3;
            pSend[7] = 1;
            memcpy(pSend+8,q->data.gcIntraAddr,20);
            pSend[28] = 1;
            pSend[29] = 1;
            pSend[30] = 0;
            pSend[31] = 0;
            pSend[32] = 0;
            pSend[33] = (byte.size())&0XFF;
            pSend[34] = ((byte.size())>>8)&0XFF;
            memcpy(pSend+35,byte.data(),byte.size());
            int t_nSendlen = 35 + byte.size();
            QString strIp = QString(q->data.gcIpAddr);
            _Sendto(strIp,(unsigned char*)pSend,t_nSendlen);
            q->data.nAskTime = ASK_MESSAGE_TIME;
        }
        q = q->next;
    }
    if(ncheck != 0)
        {
            emit Sigbtn(2);
        }else
            emit Sigbtn(3);
    return ncheck;
}

//接收到注销卡回应
void CUDPSERVER::_RecvLogOutCard(unsigned char *buf,char *pFromIP)
{
    unsigned char pbuf[8100] = {0};
    memcpy(pbuf,buf,8100);
    emit SigRecvLogOutCard(pbuf,QString(pFromIP));
}
//下载
void CUDPSERVER::_RecvDownAllCard(unsigned char *buf,char *pFromIP)
{
//    qDebug()<<"card"<<buf[31]<<buf[32];
    unsigned char pbuf[8100] = {0};
    memcpy(pbuf,buf,8100);
    emit SigRecvDownAllCard(pbuf,QString(pFromIP));
}
//接收注册ic卡的返回
void CUDPSERVER::_RecvDownCard(unsigned char *buf,char *pFromIP)
{
//    qDebug()<<"card"<<buf[31]<<buf[32];
    unsigned char pbuf[8100] = {0};
    memcpy(pbuf,buf,8100);
    emit SigRecvDownCard(pbuf,QString(pFromIP));
}

void CUDPSERVER::_RecvDownAloneCard(unsigned char *buf,char *pFromIP)
{
    int nAllCard = buf[28];
    emit SigRecvDownAloneCard(nAllCard);
}

//2015 03 26
//接收上传卡的返回
void CUDPSERVER::_RecvUpCard(unsigned char *buf,char *pFromIP)
{
   /*int nCard = buf[32] + buf[33]*256;
    int nAllCard = buf[28] + buf[29]*256;
    char pAddr[20] = {0};

    memcpy(pAddr,(char *)buf+8,20);
    int nRecvCards = (buf[31]-1)*CARD_MAXLEN+nCard;
    for(int i = 0;i < nCard;i++)
    {
        QString str2;
        for(int j = 0;j<5;j++)
        {
            int kk = buf[34+j+i*5];
            if(kk < 16) str2 = str2 + "0";
            str2 = str2 + QString::number(kk, 16).toUpper();
            if(j != 4) str2 = str2 + ":";
        }
        if(m_Mysql._UpdateIcCard(str2,QString(pAddr)))
        {
            emit SigRecvUpCard(nRecvCards,nAllCard);
        }
    }
    if(nRecvCards == nAllCard)
    {
        ItemMiddleUpdateCardToUp(pAddr,m_MiddleItem,1);
//        return;
    }*/
    unsigned char pbuf[8100] = {0};
    memcpy(pbuf,buf,8100);
    emit SigRecvUpCard(pbuf);
    _AskRecvUpCard(buf,pFromIP);
}
//2015 03 26
//发送收到IC卡数据确认指令
void CUDPSERVER::_AskRecvUpCard(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[34] = {0};
    memcpy(pSend,buf,34);
    pSend[7] = 1;
    int tSendLen = 34;
    _Sendto(strIp,pSend,tSendLen);
}
//回复设备在线指令
void CUDPSERVER::_AskDeviceOnline(unsigned char *buf,char *pFromIP)
{
    unsigned char send[512] = {0};
    memcpy(send,buf,28);
    send[7] = 2;
    send[28] = 10;
    send[29] = 0;
    QDateTime dateTime=QDateTime::currentDateTime();
    send[30] = dateTime.date().year()/256;
    send[31] = dateTime.date().year()%256;
    send[32] = dateTime.date().month();
    send[33] = dateTime.date().day();
    send[34] = dateTime.time().hour();
    send[35] = dateTime.time().minute();
    send[36] = dateTime.time().second();
    send[37] = sWeather.nWeather;
    send[38] = sWeather.nMinTemp;
    send[39] = sWeather.nMaxTemp;
    int nSendlen = 512;
    QString strIp = QString(pFromIP);
    _Sendto(strIp,send,nSendlen);
}
//接收到初始化密码返回
void CUDPSERVER::_RecvInitPass(unsigned char *buf,char *pFromIP)
{
    int nFlag = buf[34];
    char gcAddr[20] = {0};
    memcpy(gcAddr,(char*)buf + 8,20);
    emit SigRecvInitPass(QString(gcAddr),nFlag);
}
//接收到发送普通信息返回
void CUDPSERVER::_RecvSendMessage(unsigned char *buf,char *pFromIP)
{
    int nId = ItemTenementFindSendID(buf,m_TenementItem);
    if(nId >= 0)
    {
        emit SigSendMessage(nId,1);
    }
}
//收到中间设备报警
void CUDPSERVER::_MiddleAlarm(unsigned char *buf,char *pFromIP)
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


    memcpy(sAlarmDevice.gcIntraAddr ,(char*)buf + 9,20);
    QString str;
    for(int i = 0;i<6;i++)
    {
        int kk = buf[29+i];
        if(kk < 16) str = str + "0";
        str = str + QString::number(kk, 16).toUpper();
         if(i != 5) str = str + ":";
    }

    QByteArray byte;
    byte = str.toAscii();
    memcpy(sAlarmDevice.gcMacAddr,byte.data(),byte.size());
    memcpy(sAlarmDevice.gcIpAddr,pFromIP,strlen(pFromIP));

    sAlarmDevice.nFenceId = 0;
    sAlarmDevice.nTenementId = 0;

    str.clear();
    str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    byte.clear();
    byte = str.toAscii();

    memcpy(sAlarmDevice.gcStime,byte.data(),byte.size());

    sAlarmDevice.nAlarmType = 13 + buf[8];
//    qDebug()<<"sAlarmDevice.nAlarmType"<<sAlarmDevice.nAlarmType;
     if(FindAlarmItem(sAlarmDevice,m_AlarmItem))
     {
         if(!FindItemMiddle(&sAlarmDevice,m_MiddleItem))
         {
             if(m_Mysql._InsertAlarm(sAlarmDevice))
             {
             m_AlarmItem = InsertAlarmItem(sAlarmDevice,m_AlarmItem);
             m_nAlarm ++;
             emit SigInsertAlarm();
             }
         }
     }
    _AskMiddleAlarm(buf,pFromIP);
}

//收到住户设备报警
void CUDPSERVER::_DeviceAlarm(unsigned char *buf,char *pFromIP)
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


    memcpy(sAlarmDevice.gcIntraAddr ,(char*)buf + 8,20);
    QString str;
    for(int i = 0;i<6;i++)
    {
        int kk = buf[28+i];
        if(kk < 16) str = str + "0";
        str = str + QString::number(kk, 16).toUpper();
         if(i != 5) str = str + ":";
    }

    QByteArray byte;
    byte = str.toAscii();

    memcpy(sAlarmDevice.gcMacAddr,byte.data(),byte.size());
    memcpy(sAlarmDevice.gcIpAddr,pFromIP,strlen(pFromIP));

    sAlarmDevice.nFenceId = buf[37];
    sAlarmDevice.nTenementId = 0;

    str.clear();
    str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    byte.clear();
    byte = str.toAscii();

    memcpy(sAlarmDevice.gcStime,byte.data(),byte.size());
    if(buf[35])//特殊报警
    {
     sAlarmDevice.nAlarmType = 0;
     if(FindAlarmItem(sAlarmDevice,m_AlarmItem))
     {
         //特殊报警插入
         if(!FindItemTenement(&sAlarmDevice,m_TenementItem))
         {
             if(m_Mysql._InsertAlarm(sAlarmDevice))
             {
             m_AlarmItem = InsertAlarmItem(sAlarmDevice,m_AlarmItem);
             m_nAlarm ++;
             emit SigInsertAlarm();
             }
         }
     }

    }else{
        if(buf[34])//是否布防
        {
         sAlarmDevice.nAlarmType = buf[36];
         sAlarmDevice.nFenceId = buf[37];
         if(FindAlarmItem(sAlarmDevice,m_AlarmItem))
         {
             if(!FindItemTenement(&sAlarmDevice,m_TenementItem))
             {
                 if(m_Mysql._InsertAlarm(sAlarmDevice))
                 {
                 m_AlarmItem = InsertAlarmItem(sAlarmDevice,m_AlarmItem);
                 m_nAlarm ++;
                 emit SigInsertAlarm();
                 }
             }
         }
         }
        }
    _AskAlarm(buf,pFromIP);
}
//回复中间设备报警
void CUDPSERVER::_AskMiddleAlarm(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[34] = {0};
    memcpy(pSend,buf,34);
    pSend[7] = 2;
    int tSendLen = 34;
    _Sendto(strIp,pSend,tSendLen);
}
//回复住户设备报警
void CUDPSERVER::_AskAlarm(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[72] = {0};
    memcpy(pSend,buf,38);
    pSend[7] = 2;
    int tSendLen = 72;
    _Sendto(strIp,pSend,tSendLen);
}
//收到设备取消报修
void CUDPSERVER::_CancelRepairs(unsigned char *buf,char *pFromIP)
{
    char pMessage[30] = {0};
    QString strAddr,strType,strEtime;
    memcpy(pMessage,(char*)buf+8,20);
    strAddr = QString(pMessage);
    memset(pMessage,0,30);
    memcpy(pMessage,(char*)buf+40,24);
    strType = QString(pMessage);

    int year = buf[34] +2000;
    int month = buf[35];
    int day = buf[36];
    int hour = buf[37];
    int minute = buf[38];
    int sec = buf[39];
    QDateTime dateTime;
    dateTime.date().setYMD(year,month,day);
    dateTime.time().setHMS(hour,minute,sec);
    if(dateTime.isNull() || year > 2048 )
    {
        dateTime = QDateTime::currentDateTime();
    }
    strEtime = dateTime.toString("yyyy-MM-dd hh:mm:ss");

    int nFlag = 1;
    if(m_Mysql._UpdateRepairsRecord(nFlag,"",strType,strEtime,strAddr,""))
      {
          if(m_nPage == 4)
            emit SigRepairsRecord();
          _AskCancelRepairs(buf,pFromIP);
      }
}
//回复设备取消报修
void CUDPSERVER::_AskCancelRepairs(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[64] = {0};
    memcpy(pSend,buf,64);
    pSend[7] = 2;
    int tSendLen = 64;
    _Sendto(strIp,pSend,tSendLen);
}

//收到取消报警
void CUDPSERVER::_CancelAlarm(unsigned char *buf,char *pFromIP)
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

    memcpy(sAlarmDevice.gcIntraAddr ,(char*)buf + 8,20);
    memcpy(sAlarmDevice.gcIpAddr,pFromIP,strlen(pFromIP));


    QString str;
    QByteArray byte;
    str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    byte.clear();
    byte = str.toAscii();
    memcpy(sAlarmDevice.gcEtime,byte.data(),byte.size());

    if(m_Mysql._UpdateAlarm(sAlarmDevice)){
    if(UpdataItemAlarm(sAlarmDevice,m_AlarmItem))
    {
        emit SigInsertAlarm();
    }
    }
    _AskCancelAlarm(buf,pFromIP);
}

//回复取消报警
void CUDPSERVER::_AskCancelAlarm(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[30] = {0};
    memcpy(pSend,buf,30);
    pSend[7] = 2;
    int tSendLen = 30;
    _Sendto(strIp,pSend,tSendLen);
}

void CUDPSERVER::_UpCallPic(unsigned char *buf,char *pFromIP)
{

}

void CUDPSERVER::_SwipeCard(unsigned char *buf,char *pFromIP)
{
    char pIntraAddr[20] = {0};
    char gcAddrExplain[50] = {0};
    memcpy(pIntraAddr,(char*)buf+8,20);
    QString strCardNum = "00:";
    for(int i = 0;i<4;i++)
    {
        int k = buf[28+i];
        if(k < 16) strCardNum = strCardNum + "0";
        strCardNum = strCardNum + QString::number(k, 16).toUpper();
        if(i != 3) strCardNum = strCardNum + ":";
    }

    QString strIntra,strAddrExplain,strType;
    switch(pIntraAddr[0])
    {
    case 'W':
        strType = "围墙机";
        break;
    case 'M':
        strType = "门口机";
        break;
    case 'H':
        strType = "二次门口机";
        break;
    default:
        strType = "错误数据";
        break;
    }

    strIntra = QString(pIntraAddr);
    _IntraToAddrExplain(strIntra,gcAddrExplain);
    strAddrExplain = QString(gcAddrExplain);
    m_Mysql._InsertCardRecord(strType,strIntra,strCardNum,strAddrExplain);

    QString strIp = QString(pFromIP);
    unsigned char pSend[33] = {0};
    memcpy(pSend,buf,33);
    pSend[7] = 2;
    int tSendLen = 33;
    _Sendto(strIp,pSend,tSendLen);
}

void CUDPSERVER::_DealAlarm(char *pBuf,int nAlarmType,QString strIp)
{
    unsigned char gSend[29] = {0};
    memcpy(gSend,m_gHead,6);
    gSend[6] = 26;
    gSend[7] = 1;
    gSend[8] = nAlarmType - 13;
    memcpy(gSend+9,(unsigned char*)pBuf,20);
    int nSendlen = 29;
    _Sendto(strIp,gSend,nSendlen);
}

//发送控制灯
void CUDPSERVER::_SendLamp(char *pBuf,QString strIp)
{
    unsigned char gSend[37] = {0};
    memcpy(gSend,m_gHead,6);
    gSend[6] = 6;
    gSend[7] = 1;
    memcpy(gSend+8,(unsigned char*)pBuf,29);
    int nSendlen = 37;
    _Sendto(strIp,gSend,nSendlen);
}
//发送读地址指令
void CUDPSERVER::_SendRead(char *pAddr,QString strIp)
{
    unsigned char gSend[28] = {0};
    memcpy(gSend,m_gHead,6);
    gSend[6] = 41;
    gSend[7] = 1;
    memcpy(gSend+8,(unsigned char*)pAddr,20);
    int nSendlen = 28;
    _Sendto(strIp,gSend,nSendlen);
}
//发送写地址指令
void CUDPSERVER::_SendWrite(char *pBuf,QString strIp)
{
    unsigned char gSend[72] = {0};
    memcpy(gSend,m_gHead,6);
    gSend[6] = 40;
    gSend[7] = 1;
    memcpy(gSend+8,(unsigned char*)pBuf,64);
    int nSendlen = 72;
    _Sendto(strIp,gSend,nSendlen);
}
//收到设备报修
void CUDPSERVER::_Repairs(unsigned char *buf,char *pFromIP)
{
    char pMessage[64] = {0};
    char pAddr[20] = {0};
    char pType[20] = {0};
    QString strAddr,strType,strStime,strIntra;
    memcpy(pMessage,(char*)buf+8,20);

    switch(pMessage[0])
    {
    case 'S':
        strcpy(pType, "室内机");
        break;
    case 'M':
        strcpy(pType, "门口机");
        break;
    case 'W':
        strcpy(pType, "围墙机");
        break;
    case 'Z':
        strcpy(pType, "中心机");
        break;
    case 'H':
        strcpy(pType, "二次门口机");
        break;
    default :
        break;
    }

//    _IntraToAddr(pMessage,pAddr,pType);
    strIntra = QString(pMessage);
    _IntraToAddrExplain(strIntra,pMessage);
    strAddr = QString(pMessage);

    memset(pMessage,0,64);
    memcpy(pMessage,(char*)buf+40,24);
    strType = QString(pMessage);

    int year = buf[34] +2000;
    int month = buf[35];
    int day = buf[36];
    int hour = buf[37];
    int minute = buf[38];
    int sec = buf[39];
    QDateTime dateTime;
    dateTime.date().setYMD(year,month,day);
    dateTime.time().setHMS(hour,minute,sec);
    if(dateTime.isNull() || year > 2048 )
    {
        dateTime = QDateTime::currentDateTime();
    }
    strStime = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    int nFlag = 0;
    if(m_Mysql._UpdateRepairsRecord(nFlag,strAddr,strType,strStime,strIntra,QString(pType)))
    {
        emit SigRepairsRecord();
        _AskRepairs(buf,pFromIP);
    }
}
//回复设备报修
void CUDPSERVER::_AskRepairs(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[30] = {0};
    memcpy(pSend,buf,30);
    pSend[7] = 2;
    int tSendLen = 30;
    _Sendto(strIp,pSend,tSendLen);
}
//20150304
//内部地址和显示地址的转化
void CUDPSERVER::_IntraToAddr(char *pIntra,char *pAddr,char *pType)
{
    char p[20] = {0};
    memcpy(p,pIntra+1,3);
    p[3] = '-';
    memcpy(p+4,pIntra+4,3);
    p[7] = '-';
    memcpy(p+8,pIntra+7,3);
    p[11] = '-';
    memcpy(p+12,pIntra+10,3);
    p[15] = '-';
    memcpy(p+16,pIntra+13,3);
    memcpy(pAddr,p,20);
    switch(pIntra[0])
    {
    case 'S':
        strcpy(pType, "室内机");
        break;
    case 'M':
        strcpy(pType, "门口机");
        break;
    case 'W':   
        strcpy(pType, "围墙机");
        break;
    case 'Z':
        strcpy(pType, "中心机");
        break;
    case 'H':
        strcpy(pType, "二次门口机");
        break;
    default :
        break;
    }
    return;
}
//20150304
void CUDPSERVER::_IntraToAddrExplain(QString strIntra,char *pAddr)
{
    QString str,strAddr;
    if(strIntra.left(1) == "S")
    {
        strIntra = strIntra.right(strIntra.length() - 1);
        str.clear();
        strAddr.clear();
        str = strIntra.left(3);
        strAddr = QString::number(str.toInt()) + "栋";
        strIntra = strIntra.right(strIntra.length() - 3);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "单元";
        strIntra = strIntra.right(strIntra.length() - 3);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "楼";
        strIntra = strIntra.right(strIntra.length() - 3);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "房";
        strIntra = strIntra.right(strIntra.length() - 3);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "号设备";

        memcpy(pAddr,strAddr.toUtf8().data(),strAddr.toUtf8().size());
    }else if(strIntra.left(1) == "M"){
        strIntra = strIntra.right(strIntra.length() - 1);
        str.clear();
        strAddr.clear();
        str = strIntra.left(3);
        strAddr = QString::number(str.toInt()) + "栋";
        strIntra = strIntra.right(strIntra.length() - 3);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "单元";
        strIntra = strIntra.right(strIntra.length() - 3);

        strIntra = strIntra.right(strIntra.length() - 6);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "号设备";

        memcpy(pAddr,strAddr.toUtf8().data(),strAddr.toUtf8().size());
    }else if(strIntra.left(1) == "W"){
        strIntra = strIntra.right(strIntra.length() - 1);
        str.clear();
        strAddr.clear();
        str = strIntra.left(3);
        strAddr = QString::number(str.toInt()) + "栋";
        strIntra = strIntra.right(strIntra.length() - 3);


        strIntra = strIntra.right(strIntra.length() - 9);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "号设备";

        memcpy(pAddr,strAddr.toUtf8().data(),strAddr.toUtf8().size());
    }else if(strIntra.left(1) == "Z"){
        strIntra = strIntra.right(strIntra.length() - 1);
        str.clear();
        strAddr.clear();
        str = strIntra.left(3);
        strAddr = QString::number(str.toInt()) + "栋";
        strIntra = strIntra.right(strIntra.length() - 3);


        strIntra = strIntra.right(strIntra.length() - 9);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "号设备";

        memcpy(pAddr,strAddr.toUtf8().data(),strAddr.toUtf8().size());
    }else if(strIntra.left(1) == "H"){
        strIntra = strIntra.right(strIntra.length() - 1);
        str.clear();
        strAddr.clear();
        str = strIntra.left(3);
        strAddr = QString::number(str.toInt()) + "栋";
        strIntra = strIntra.right(strIntra.length() - 3);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "单元";
        strIntra = strIntra.right(strIntra.length() - 3);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "楼";
        strIntra = strIntra.right(strIntra.length() - 3);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "房";
        strIntra = strIntra.right(strIntra.length() - 3);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "号设备";

        memcpy(pAddr,strAddr.toUtf8().data(),strAddr.toUtf8().size());
    }
    return ;
}

//下发报修类型
void CUDPSERVER::_SendRepairs(int nLen,char *pMessage)
{
    unsigned char pSend[1024] = {0};
    memcpy(pSend,m_gHead,6);
    pSend[6] = 7;
    pSend[7] = 1;
    pSend[8] = 1;
    pSend[9] = nLen;
    int MessageLen = 24*nLen;
    pSend[10] = MessageLen&0XFF;
    pSend[11] = (MessageLen>>8)&0XFF;
    memcpy(pSend+12,pMessage,MessageLen);
    int nSendlen = 12 + MessageLen;
    QString strIp;
    NodeTenement *q;
    q = m_TenementItem;
    while(q->next)
    {
        if(q->data.nState > 0)
        {
            strIp.clear();
            strIp = QString(q->data.gcIpAddr);
            if(strIp != "")
            {
            _Sendto(strIp,pSend,nSendlen);
            }
        }
        q = q->next;
    }
}
//删除报警
SAlarmDevice CUDPSERVER::_DeleteItemAlarm(SAlarmDevice sAlarmDevice)
{
    m_AlarmItem = DeleteItemAlarm(&sAlarmDevice,&m_AlarmItem);
    return sAlarmDevice;
}
//通过报警设备链表获取链表里的指定设备的信息
int CUDPSERVER::_GetAlarmFromItem(SAlarmDevice *sAlarmDevice,NodeAlarm *ItemAlarm)
{
    NodeAlarm *t;
    t = ItemAlarm;
    while(t->next)
    {
        if(t->data.nId == (*sAlarmDevice).nId )
        {
            (*sAlarmDevice).nAlarmType = t->data.nAlarmType;
            (*sAlarmDevice).nFenceId = t->data.nFenceId;
            memcpy((*sAlarmDevice).gcAddr , t->data.gcAddr,64);
            memcpy((*sAlarmDevice).gcIntraAddr , t->data.gcIntraAddr,20);

            memcpy((*sAlarmDevice).gcType , t->data.gcType,20);
            memcpy((*sAlarmDevice).gcIpAddr , t->data.gcIpAddr,20);
            memcpy((*sAlarmDevice).gcStime , t->data.gcStime,20);
            memcpy((*sAlarmDevice).gcEtime , t->data.gcEtime,20);
            memcpy((*sAlarmDevice).gcPhone1 , t->data.gcPhone1,20);
            memcpy((*sAlarmDevice).gcName , t->data.gcName,64);
            return 1;
        }else
            t = t->next;
    }
    return 0;
}
//20150304
//发送复位密码
void CUDPSERVER::_SendPass(QString strAddr,QString strMac,QString strIp,QString strType,int nFlag)
{
    unsigned char pSend[35] = {0};
    QString Addr;
    Addr.clear();
    memcpy(pSend,m_gHead,6);
    pSend[6] = 30;
    pSend[7] = 1;

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
        pSend[8] = 83;
    }else     if(strType == "别墅室内机")
    {
        pSend[8] = 66;
    }else     if(strType == "门口机")
    {
        pSend[8] = 77;
    }else     if(strType == "围墙机")
    {
        pSend[8] = 87;
    }else     if(strType == "中心机")
    {
        pSend[8] = 90;
    }else    if(strType == "二次门口机")
    {
        pSend[8] = 72;
    }
    QByteArray byte;
    byte = Addr.toUtf8();
    memcpy(pSend+9,byte.data(),byte.size());
    QString str;
    str.clear();
    for(int i = 0;i < 5;i++)
    {
        int j = strMac.indexOf(":");
        str = strMac.left(j);
        bool ok;
        pSend[28+i] = str.toInt(&ok,16);
        strMac = strMac.right(strMac.length() - j - 1);
        if(i == 4)
        {
            pSend[29+i] = strMac.toInt(&ok,16);
        }
    }

    pSend[34] = nFlag;
    _Sendto(strIp,pSend,35);
}
//清空报警链表
bool CUDPSERVER::_ClearAlarm()
{
    NodeAlarm *q;
    q = (NodeAlarm *)malloc(sizeof(NodeAlarm));
    if(m_AlarmItem->next == NULL) return true;
    while(m_AlarmItem->next)
    {
      q=m_AlarmItem->next;
      delete(m_AlarmItem);
      m_AlarmItem=q;
    }
    return true;
}

int CUDPSERVER::_DownAloneCard(QByteArray byte,char *pCard,int nAllCard)
{
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    int nSendLen = 29+nAllCard*5;
    if(ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
    {
    unsigned char pSend[79] = {0};
    memcpy(pSend,m_gHead,6);
    pSend[6] = 71;
    pSend[7] = 1;
    memcpy(pSend+8,pAddr,20);
    pSend[28] = nAllCard;
    memcpy(pSend+29,pCard,nAllCard*5);
    return _Sendto(QString(pSendIp),pSend,nAllCard);
    }else
        return 0;
}


//注销卡
int CUDPSERVER::_LogOutCard(QByteArray byte,char *pCard,int nAllCard,int nSendCard,int nPack)
{
//    qDebug()<<"udp1111"<<pCard[0]<<pCard[1]<<pCard[2]<<pCard[3]<<pCard[4];
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    int nSendLen = 34+nSendCard*5;
    if(ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
    {
    unsigned char pSend[1300] = {0};
    memcpy(pSend,m_gHead,6);
    pSend[6] = 74;
    pSend[7] = 1;
    memcpy(pSend+8,pAddr,20);
    pSend[28] = nAllCard&0XFF;
    pSend[29] = (nAllCard>>8)&0XFF;
    if(nAllCard%CARD_MAXLEN)
    {
        pSend[30] = nAllCard/CARD_MAXLEN + 1;
    }else
        pSend[30] = nAllCard/CARD_MAXLEN;
    pSend[31] = nPack;
    pSend[32] = nSendCard&0XFF;
    pSend[33] = (nSendCard>>8)&0XFF;
    memcpy(pSend+34,pCard,nSendCard*5);
    return _Sendto(QString(pSendIp),pSend,nSendLen);
    }else
        return 0;
}


int CUDPSERVER::_DownAllCard(QByteArray byte,char *pCard,int nAllCard,int nSendCard,int nPack)
{
//    qDebug()<<"udp1111"<<pCard[0]<<pCard[1]<<pCard[2]<<pCard[3]<<pCard[4];
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    int nSendLen = 34+nSendCard*5;
    if(ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
    {
    unsigned char pSend[8100] = {0};
    memcpy(pSend,m_gHead,6);
    pSend[6] = 72;
    pSend[7] = 1;
    memcpy(pSend+8,pAddr,20);
    pSend[28] = nAllCard&0XFF;
    pSend[29] = (nAllCard>>8)&0XFF;
    if(nAllCard%CARD_MAXLEN)
    {
        pSend[30] = nAllCard/CARD_MAXLEN + 1;
    }else
        pSend[30] = nAllCard/CARD_MAXLEN;
    pSend[31] = nPack;
    pSend[32] = nSendCard&0XFF;
    pSend[33] = (nSendCard>>8)&0XFF;
    memcpy(pSend+34,pCard,nSendCard*5);
    return _Sendto(QString(pSendIp),pSend,nSendLen);
    }else{
        return 0;
    }
}
//2015 03 26
//下载IC卡到设备
int CUDPSERVER::_DownCard(QByteArray byte,char *pCard,int nAllCard,int nSendCard,int nPack)
{
//    qDebug()<<"udp1111"<<pCard[0]<<pCard[1]<<pCard[2]<<pCard[3]<<pCard[4];
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    int nSendLen = 34+nSendCard*5;
    if(ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
    {
    unsigned char pSend[8100] = {0};
    memcpy(pSend,m_gHead,6);
    pSend[6] = 75;
    pSend[7] = 1;
    memcpy(pSend+8,pAddr,20);
    pSend[28] = nAllCard&0XFF;
    pSend[29] = (nAllCard>>8)&0XFF;
    if(nAllCard%CARD_MAXLEN)
    {
        pSend[30] = nAllCard/CARD_MAXLEN + 1;
    }else
        pSend[30] = nAllCard/CARD_MAXLEN;
    pSend[31] = nPack;
    pSend[32] = nSendCard&0XFF;
    pSend[33] = (nSendCard>>8)&0XFF;
    memcpy(pSend+34,pCard,nSendCard*5);
    return _Sendto(QString(pSendIp),pSend,nSendLen);
    }else{
        return 0;
    }
}
//上传IC卡指令下发
int CUDPSERVER::_UpCard(QByteArray byte)
{
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    if(ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
    {
    unsigned char pSend[28] = {0};
    memcpy(pSend,m_gHead,6);
    pSend[6] = 70;
    pSend[7] = 1;
    memcpy(pSend+8,pAddr,20);
    int nSend = _Sendto(QString(pSendIp),pSend,28);
    return nSend;
    }else{
        return 0;
    }
}

//下发常用电话
void CUDPSERVER::_SendPhone(int nLen,char *pMessage)
{
    unsigned char pSend[1024] = {0};
    memcpy(pSend,m_gHead,6);
    pSend[6] = 7;
    pSend[7] = 1;
    pSend[8] = 2;
    pSend[9] = nLen;
    int MessageLen = 72*nLen;
    pSend[10] = MessageLen&0XFF;
    pSend[11] = (MessageLen>>8)&0XFF;
    memcpy(pSend+12,pMessage,MessageLen);
    int nSendlen = 12 + MessageLen;
    QString strIp;
    NodeTenement *q;
    q = m_TenementItem;
    while(q->next)
    {
        if(q->data.nState > 0)
        {
            strIp.clear();
            strIp = QString(q->data.gcIpAddr);
            if(strIp != "")
            {
            _Sendto(strIp,pSend,nSendlen);
            }
        }
        q = q->next;
    }
}
