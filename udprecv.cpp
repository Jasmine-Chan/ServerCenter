#include "udprecv.h"
#include <QMessageBox>
#include "mysql.h"
#include <QDebug>

CUDPRECV::CUDPRECV(SUser suser,QObject *parent) :
    QThread(parent)
{
    qRegisterMetaType<QVariant>("QVariant");
    m_sUser = suser;
    m_strCity = suser.strCityNum;
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
void CUDPRECV::_UpdateLoading(QString strCityNum)
{
    m_strCity = strCityNum;
    _QueryNowWeather();
    _QueryDayWeather();
}
//初始化成员变量
void CUDPRECV::_Init()
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
    m_strLocalIP = QNetworkInterface::allAddresses().first().toString();
    qDebug()<<"IP:"<<m_strLocalIP;
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
void CUDPRECV::_Path()
{
    strPath = Files.at(nFinishfiles + nFailFile);
    int i = strPath.lastIndexOf('\\');
    if(i == -1)
    {
        i = strPath.lastIndexOf('/');
    }
    int j = strPath.length();
    QString t_strName = strPath.right(j-i-1);
    memset(m_gName,0,20);
    memcpy(m_gName,(t_strName.toAscii().data()),t_strName.length());
}
//下载文件 发送开始下载命令
void CUDPRECV::_LoadDown(QString strIp)
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
        }
        else    //发送开始失败
        {
            _Fail(strIp);
        }
    }
    else
    {
        _Fail(strIp);
    }
}
//下载失败
void CUDPRECV::_Fail(QString strIp)
{
    nFailFile ++;
    nAskTime =0;
    if(File.isOpen()) File.close();
    //发送信号
    emit SigNewsDonw(nCheckId, nAallfile, nFinishfiles);
    if(nAallfile == (nFinishfiles + nFailFile))
    {
        _SendNext();
    }
    else
    {
        _LoadDown(strIp);
    }
}
//向下一个选中的设备发送文件
void  CUDPRECV::_SendNext()
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
//开始下载文件 的指令A9---223
int CUDPRECV::_Begin(QString strIp)
{
    unsigned char pSend[60]={0};
    memcpy(pSend,m_gHead,6);
    pSend[6] = 223;
    pSend[7] = 1;
    pSend[8] = 1;
    memcpy(pSend+9,m_gMark,19);
    memcpy(pSend+29,m_gName,20);
    if(!_FileMsg(pSend))
    {
        return 0;
    }
    pSend[57] = 0;
    pSend[58] = 0;
    int nSendLen = 59;
    return _Sendto(strIp,pSend,nSendLen);
}
//发送文件的数据的分包
int CUDPRECV::_FileMsg(unsigned char *pSend)
{
    if(nLength <= (int)0XFFFFFFFF)
    {
        pSend[49] = nLength&0XFF;
        pSend[50] = (nLength>>8)&0XFF;
        pSend[51] = (nLength>>16)&0XFF;
        pSend[52] = (nLength>>24)&0XFF;
    }
    else
    {
        QMessageBox::warning(NULL,tr("提示"),tr("文件太大"));
        return 0;
    }
    //总包数
    if(nPacklen <= 0XFFFF)
    {
        pSend[53] = nPacklen&0XFF;
        pSend[54] = (nPacklen>>8)&0XFF;
    }
    else
    {
        QMessageBox::warning(NULL,tr("提示"),tr("超过总包数最大值"));
        return 0;
    }
    //当前包数
    if(nSendPack <= 0XFFFF)
    {
        pSend[55] = nSendPack&0XFF;
        pSend[56] = (nSendPack>>8)&0XFF;
    }
    else
    {
        QMessageBox::warning(NULL,tr("提示"),tr("超过当前包数最大值"));
        return 0;
    }
    return 1;
}
//读取软件有效期配置
void CUDPRECV::_ReadConfig()
{
    char buf[12] = {0};
    ::GetPrivateProfileStringA("Flag","STime","sa",buf,12,"C:\\WINDOWS\\UTSet.ini");
    QDate Date = QDate::fromString(QString(buf),"yyyy-MM-dd");
    qDebug()<<Date;
    if(Date.isNull())
    {
        emit SigDateTime(1,0);
        return;
    }
    QDate NowDate = QDateTime::currentDateTime().date();
    int AllDay = Date.daysTo(NowDate);      //注册日期到现在的天数
    int  nFlag = ::GetPrivateProfileIntA("Flag","idex",0,"C:\\WINDOWS\\UTSet.ini");
    int nMonth = ::GetPrivateProfileIntA("Flag","month",0,"C:\\WINDOWS\\UTSet.ini");
    if(nFlag == 0)
    {
        emit SigDateTime(1,0);
        return;
    }
    else if(nFlag == 1)
    {
        if(AllDay < (int)(::GetPrivateProfileIntA("1","AllDay",0,"C:\\WINDOWS\\UTSet.ini")))
        {
            emit SigDateTime(1,0);     //提示，系统维护
            return;
        }
        else
        {
            if(AllDay >= 60 && AllDay < 70)
            {
                if(::GetPrivateProfileIntA("1","Count",0,"C:\\WINDOWS\\UTSet.ini") == 0)//没有count属性？？？
                {
                    emit SigDateTime(2,AllDay);     //温馨提示
                    ::WritePrivateProfileStringA("1", "Count", QString::number(1).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
                }
            }
            else if(AllDay >= 70 && AllDay < 80)
            {
                if(::GetPrivateProfileIntA("1","Count",0,"C:\\WINDOWS\\UTSet.ini") <= 1)
                {
                    emit SigDateTime(3,AllDay);     //郑重提示
                    ::WritePrivateProfileStringA("1", "Count", QString::number(2).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
                }
            }
            else if(AllDay >= (80 + 30*nMonth))
            {
                    emit SigDateTime(4,AllDay);     //提示，系统维护
            }
            ::WritePrivateProfileStringA("1", "AllDay", QString::number(AllDay).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
        }
    }
    else if(nFlag == 2)
    {
        if(AllDay < (int)(::GetPrivateProfileIntA("2","AllDay",0,"C:\\WINDOWS\\UTSet.ini")))
        {
            emit SigDateTime(1,0);     //提示，系统维护
            return;
        }
        else
        {
            if(AllDay >= 750 && AllDay < 780)
            {
                if(::GetPrivateProfileIntA("2","Count",0,"C:\\WINDOWS\\UTSet.ini") == 0)
                {
                    emit SigDateTime(2,AllDay);     //温馨提示
                    ::WritePrivateProfileStringA("2", "Count", QString::number(1).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
                }
            }
            else if(AllDay >= 780 && AllDay < 810)
            {
                if(::GetPrivateProfileIntA("2","Count",0,"C:\\WINDOWS\\UTSet.ini") <= 1)
                {
                    emit SigDateTime(3,AllDay);     //郑重提示
                    ::WritePrivateProfileStringA("2", "Count", QString::number(2).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
                }
            }
            else if(AllDay >= (810 + 30*nMonth) )
            {
                emit SigDateTime(4,AllDay);     //提示，系统维护
            }
            ::WritePrivateProfileStringA("2", "AllDay", QString::number(AllDay).toUtf8().data(), "C:\\WINDOWS\\UTSet.ini");
        }
    }
}

//2015 05 05
//重启设备命令生成A9---13
void CUDPRECV::_Reset(QString strIp,QString strAddr,QString strType)
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
        pSend[8] = 83;//S
    }
    else if(strType == "别墅室内机")
    {
        pSend[8] = 66;
    }
    else if(strType == "门口机")
    {
        pSend[8] = 77;
    }
    else if(strType == "围墙机")
    {
        pSend[8] = 87;
    }
    else if(strType == "中心机")
    {
        pSend[8] = 90;
    }
    else if(strType == "二次门口机")
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
void CUDPRECV::_SendDownCard()
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
            if(nAllCard > CARD_MAXLEN)
            {
                QDateTime dateTime = QDateTime::currentDateTime();
                QString strDtime = dateTime.toString("yyyy-MM-dd");
                QString SQL = "select IC_Card_num from ic_card where IC_Card_time > '" + strDtime + "' and " + strIntraAddr + "= 1 ";
                QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
                QSqlQuery query(db);
                if(query.exec(SQL))
                {
                    int i = 0;
                    while(query.next())
                    {
                        if(i >= 0 && i<CARD_MAXLEN)
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
            }
            else
            {
                QDateTime dateTime=QDateTime::currentDateTime();
                QString strDtime = dateTime.toString("yyyy-MM-dd");
                QString SQL = "select IC_Card_num from ic_card where IC_Card_time > '" + strDtime + "' and " + strIntraAddr + "= 1 ";
                QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
                QSqlQuery query(db);
                if(query.exec(SQL))
                {
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
void CUDPRECV::_SendUpCard()
{
    NodeMiddle *q = m_MiddleItem;
    if(m_Mysql._IsExpireCard())     //查找是否有IC卡的有效期为今天
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
    }
    else
    {
        //发送不为2     0为初始值 1为需要下发 2为已经下发成功
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

void CUDPRECV::_TimeSendCard()
{
    QDateTime dateTime=QDateTime::currentDateTime();
    int hour = dateTime.time().hour();
    int minute = dateTime.time().minute();
    int second = dateTime.time().second();
    if(hour == 1)      //凌晨时判断上传和下发IC卡
    {
//        if(minute == 0 && second == 0)
          _SendUpCard();

        if(minute == 10 && second == 1)
        {
            _SendDownCard();
        }
    }
}
//定时器刷新
void CUDPRECV::_UpdateWeather()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    int minute = dateTime.time().minute();
    int second = dateTime.time().second();

    if((minute == 5) && (second == 1))//每小时重新获取天气和有效期
    {
        _QueryNowWeather();
        _QueryDayWeather();
        _ReadConfig();      //读取软件有效期
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
                m_Mysql._OnlineTenement(q->data);   //掉线存入设备日志
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
void CUDPRECV::run()
{
    //进行版本协商，加载特定版本的socket动态链接库
    int retVal;
//    int port = 8301;
    int port = 8300;
    if((retVal = WSAStartup(MAKEWORD(2,2), &wsd)) != 0)
    {
        perror("socket1");
//        qDebug() << "WSAStartup() failed";
        return ;
    }
    Fsocket = socket(PF_INET,SOCK_DGRAM,IPPROTO_IP);
    if(Fsocket == INVALID_SOCKET)
    {
        perror("socket2");
        emit SigUdpOpenFail();
//        qDebug()<<WSAGetLastError();
        return;
    }
    bool bOptVal = FALSE;
    retVal = setsockopt(Fsocket,SOL_SOCKET,SO_BROADCAST,(char*)&bOptVal,sizeof(bool));//可以发生广播
    struct sockaddr_in FAddr;
    memset(&FAddr,0,sizeof(FAddr));
    FAddr.sin_family = AF_INET;
    FAddr.sin_addr.s_addr = htonl(INADDR_ANY);    //同一网段的都能检测到
    FAddr.sin_port = htons(port);
    if(bind(Fsocket,(sockaddr*)&FAddr,sizeof(FAddr))==SOCKET_ERROR)
    {
        perror("socket3");
        emit SigUdpOpenFail();
//        qDebug() <<WSAGetLastError();
        return;
    }
    struct ip_mreq imreq;
    imreq.imr_multiaddr.s_addr = inet_addr(m_strMulticastIP.toLatin1().data());//inet_addr("238.9.9.1");
    imreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if(setsockopt(Fsocket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&imreq,sizeof(ip_mreq))==SOCKET_ERROR)
    {
        perror("socket4");
        emit SigUdpOpenFail();
//        qDebug() <<WSAGetLastError();
        return;
    }
    while(1)
    {
//        qDebug ()<<"udpRecv";
        _Read();
    }
}

//接收到的数据
int CUDPRECV::_Read()
{
    struct sockaddr_in from;
    int nLength = sizeof(struct sockaddr_in);
    unsigned char buf[8100] = {0};
    char gFromIp[20] = {0};
    int t_nRecvlen = ::recvfrom(this->Fsocket,(char*)buf,8100,0,(struct sockaddr * )&from,&nLength);
    if(!t_nRecvlen)
    {
//        qDebug() <<WSAGetLastError();
        return 0;
    }
    strcpy(gFromIp,inet_ntoa(from.sin_addr));

    if(m_sUser.protocol == 0)   //A8
    {
        _Recv_A8(buf,gFromIp,t_nRecvlen);
    }
    else        //A9
    {
        _Recv_A9(buf,gFromIp,t_nRecvlen);
    }
    return t_nRecvlen;
}

//获取实时天气
void CUDPRECV::_QueryNowWeather()
{
    QString url = "http://www.weather.com.cn/weather1d/"+m_strCity+".shtml";//"http://www.weather.com.cn/data/sk/"+m_strCity+".html";
    m_NetworkRequest1.setUrl(QUrl(url));
    m_NetworkReply1 = m_NetworkManager1->get(m_NetworkRequest1);
    connect(m_NetworkReply1, SIGNAL(readyRead()), this, SLOT(SlotReadyRead1()));
}
//获取当天天气
void CUDPRECV::_QueryDayWeather()
{
    QString url = "http://www.weather.com.cn/data/cityinfo/"+m_strCity+".html";
    m_NetworkRequest.setUrl(QUrl(url));
    m_NetworkReply = m_NetworkManager->get(m_NetworkRequest);
    connect(m_NetworkReply, SIGNAL(readyRead()), this, SLOT(SlotReadyRead()));
}

//获取天气到的数据的解析
void CUDPRECV::SlotReadyRead()
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
void CUDPRECV::SlotReadyRead1()
{
    QString strXmlFile = m_NetworkReply1->readAll();
    qDebug()<<strXmlFile;
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
void CUDPRECV::_InitTenement()
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
            m_TenementItem = CLIST::ItemTenementCreatH(sTenementDevice,m_TenementItem);
            m_nAllTenement++;
        }
    }else{
        QMessageBox::warning(NULL,tr("提示"),tr("11数据库查询失败，请检查数据库连接"));
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}
//初始化中间设备链表
void CUDPRECV::_InitMiddle()
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
            m_MiddleItem = CLIST::ItemMiddleCreatH(sMiddleDevice,m_MiddleItem);
            m_nAllMiddle++;
        }
    }else{
        QMessageBox::warning(NULL,tr("提示"),tr("数据库查询失败，请检查数据库连接"));
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
}

//接收数据的分析分流---A8
void CUDPRECV::_Recv_A8(unsigned char *buf,char *pFromIP,int nRecvLen)
{
    if(memcmp (buf, m_gHead, 6) == 0)
    {
        switch(buf[7])
        {
            case 1:         //上行
                switch(buf[6])
                {
                    case 1:
                        qDebug()<<"室内机报警";
                        _DevcieAlarm_A8(buf, pFromIP);
                        break;
                    case 2:
                        qDebug()<<"取消报警";
                        _CancelAlarm_A8(buf, pFromIP);
                        break;
                    case 4:
                        qDebug()<<"设备在线报告";
                        _DeviceOnline_A8(buf, pFromIP);
                        break;
                    case 8:
                        switch(buf[35])
                        {
                            case 1:
                                qDebug()<<"设备报修";
                                _Repairs_A8(buf, pFromIP);
                                break;
                            case 2:
                                qDebug()<<"取消设备报修";
                                _CancelRepairs_A8(buf, pFromIP);
                                break;
                        }
                        break;
                    case 56:
                        qDebug()<<"刷IC卡";
                        _SwipeCard_A8(buf, pFromIP);
                        break;
                    case 60:
                        qDebug()<<"单元门口机、围墙机、二次门口机发送呼叫照片->开始发送";
                        _StartUpCall_A8(buf, pFromIP, nRecvLen);
                        break;
                    case 61:
                        qDebug()<<"单元门口机、围墙机、二次口机发送呼叫照片->发送数据";
                        _SendCall_A8(buf, pFromIP, nRecvLen);
                        break;
                    case 62:
                        qDebug()<<"单元门口机、围墙机、二次门口机发送呼叫照片->发送结束（成功）";
                        _SuccessCall_A8(buf, pFromIP, nRecvLen);
                        break;
                    case 155:
                        qDebug()<<"主机名（地址）查找";
                        _FindAddrIP(buf, pFromIP, nRecvLen);
                        break;
                    default:
                        break;
                }
                break;
            case 2:         //下行
                switch(buf[6])
                {
                    case 3:
                        qDebug()<<"普通信息";
                        _CommonInfor_A8(buf, pFromIP);
                        break;
                    case 5:     //暂没实现
                        qDebug()<<"查询设备状态";
                        _CheckDeviceState(buf, pFromIP);
                        break;
                    case 7:     //暂没实现
                        qDebug()<<"物业报修和防区报警";
                        _RepairsType_A8(buf, pFromIP);
                        break;
                    case 30:
                        qDebug()<<"复位密码";
                        _RecvInitPass_A8(buf, pFromIP);
                        break;
                    case 41:
                        qDebug()<<"读地址设置";
                        _ReadAddr_A8(buf, pFromIP);
                        break;
                    case 54:
                        qDebug()<<"写IC卡";
                        _WriteICCard_A8(buf, pFromIP);
                        break;
                    case 55:
                        qDebug()<<"读IC卡";
                        _RecvUpCard_A8(buf, pFromIP);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

//接收数据的分析分流---A9
void CUDPRECV::_Recv_A9(unsigned char *buf,char *pFromIP,int nRecvLen)
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
                    case 73://刷卡---上行
                        _SwipeCard(buf,pFromIP);
                        break;
                    case 225://刷卡
                        _UpCallPic(buf,pFromIP);
                        break;
                    default:
                        break;
                }
                break;
            case 2:         //上行
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
                                    }
                                    else
                                    {
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
                                            }
                                            else
                                            {
                                                QString strIp = QString(pFromIP);
                                                _Fail(strIp);
                                            }
                                        }
                                        else
                                        {
                                            nSendPack ++;
                                            if(_SendData(buf,pFromIP) == 0)
                                            {
                                                QString strIp = QString(pFromIP);
                                                _Fail(strIp);
                                            }
                                            else
                                            {
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
                                    }
                                    else
                                    {
                                        if(_EndAll(buf,pFromIP))
                                        {
                                            nAskTime = ASK_MESSAGE_TIME;
                                        }
                                        else
                                        {
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

//发送一个发送完成指令A9
int CUDPRECV::_End(unsigned char *buf,char *pFromIP)
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
//发送全部发送完成指令A9
int CUDPRECV::_EndAll(unsigned char *buf,char *pFromIP)
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
//发送文件的分包数据指令A9
int CUDPRECV::_SendData(unsigned char *buf,char *pFromIP)
{
    unsigned char pSend[8100] = {0};
    memcpy(pSend,buf,55);
    int size = 0;
    if((nPacklen-1) == nSendPack )
    {
        size = File.size() - (nPacklen-1)*8000;
    }
    else
    {
        size = 8000;
    }
    QByteArray line = File.read(size);
    pSend[7] = 1;
    pSend[8] = 2;
    if(!_FileMsg(pSend))
    {
        return 0;
    }
    if(size < 0XFFFF)
    {
        pSend[57] = size&0XFF;
        pSend[58] = (size>>8)&0XFF;
    }
    else
    {
        QMessageBox::warning(NULL,tr("提示"),tr("超过包数据最大值"));
        return 0;
    }
    memcpy(pSend+59,line.data(),size);
    size = size + 59;
    QString strIp = QString(pFromIP);
    return _Sendto(strIp,pSend,size);
}

/***********  udp发送  ***********/
int CUDPRECV::_Sendto(QString strIp,unsigned char *pSend,int nSendlen)
{
    qDebug ()<<"发送数据为："<<QString((char *)pSend);
    unsigned long nAddr = inet_addr(strIp.toStdString().data());
    To.sin_family = AF_INET;
    To.sin_port = htons(8300);
    To.sin_addr.S_un.S_addr = (int)nAddr;
    if(::sendto(this->Fsocket,(char*)pSend,nSendlen,0,(struct sockaddr*)&To,sizeof(struct sockaddr))==SOCKET_ERROR)
    {
        qDebug() <<WSAGetLastError();
        return 0;
    }
    return 1;
}

//收到设备在线数据
void CUDPRECV:: _DeviceOnline(unsigned char *buf,char *pFromIP)
{
    switch(buf[8])
    {
        case 'M':   //单元门口机
        case 'W':   //围墙机
        case 'Z':   //中心机
        case 'H':   //二次门口机
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

            for(int i = 0;i<6;i++)  //网卡地址
            {
                int k = buf[28+i];
                if(k < 16)
                    str = str + "0";
                str = str + QString::number(k, 16).toUpper();
                if(i != 5)
                    str = str + ":";
            }

            QByteArray byte;
            byte = str.toAscii();

            memcpy(sMiddleDevice.gcMacAddr,byte.data(),byte.size());

            //地址转化
            _IntraToAddr(sMiddleDevice.gcIntraAddr,sMiddleDevice.gcAddr,sMiddleDevice.gcType);
            _IntraToAddrExplain(strIntra,sMiddleDevice.gcAddrExplain);

            int nMark = CLIST::ItemMiddleFind(&sMiddleDevice,m_MiddleItem);
//            qDebug()<<"H_nMark:"<<nMark;
            switch(nMark)
            {
                case -3:    //更新
                    if(m_Mysql._UpdateMiddle(nMark,sMiddleDevice))
                    {
                        m_MiddleItem = CLIST::DeleteItemMiddle(sMiddleDevice,&m_MiddleItem);
                        m_MiddleItem = CLIST::InsertItemMiddle(sMiddleDevice,m_MiddleItem);
                        if((m_nPage == 2) || (m_nPage == 12))   //中间设备 or 重要新闻
                            emit SigInitMiddle();
                    }
                    break;
                case -2:
                    break;
                case -1:    //插入
                    if(m_Mysql._InsertMiddle(sMiddleDevice))
                    {
                        m_MiddleItem = CLIST::InsertItemMiddle(sMiddleDevice,m_MiddleItem);
                        m_nAllMiddle++;
                        if((m_nPage == 2) || (m_nPage == 12))
                            emit SigInitMiddle();
                    }
                    break;
                default :
                    if(m_Mysql._UpdateMiddle(nMark,sMiddleDevice))
                    {
                        m_MiddleItem = CLIST::UpdateItemMiddle(sMiddleDevice,m_MiddleItem);
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
        case 'S':   //室内机
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
            sTenementDevice.nFenceState = buf[34];      //布防状态

            //地址转化
            _IntraToAddr(sTenementDevice.gcIntraAddr,sTenementDevice.gcAddr,sTenementDevice.gcType);
            _IntraToAddrExplain(strIntra,sTenementDevice.gcAddrExplain);

            int nMark = CLIST::ItemTenementFind(&sTenementDevice,m_TenementItem);
//            qDebug()<<"S_nMark:"<<nMark;
            switch(nMark)
            {
                case -3:
                    if(m_Mysql._UpdateTenement(sTenementDevice))
                    {
                        m_TenementItem = CLIST::DeleteItemTenement(sTenementDevice,&m_TenementItem);
                        m_TenementItem = CLIST::InsertItemTenement(sTenementDevice,m_TenementItem);
                        if((m_nPage == 1) || (m_nPage == 10))    //住户设备 or 普通信息
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
                        m_TenementItem = CLIST::InsertItemTenement(sTenementDevice,m_TenementItem);
                        m_nAllTenement++;
                        if((m_nPage == 1) || (m_nPage == 10))
                        {
                            emit SigInitTenement();
                        }
                    }
                    break;
                default :
                    if(m_Mysql._UpdateTenement(sTenementDevice))
                    {
                        m_TenementItem = CLIST::UpdateItemTenement(sTenementDevice,m_TenementItem);
                        QVariant var1;
                        var1.setValue(sTenementDevice);
                        if((m_nPage == 1) || (m_nPage == 10))
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
//发送普通数据指令A9
int CUDPRECV::_SendMessage(QString strMessage)
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
//            _Sendto(strIp,(unsigned char*)pSend,t_nSendlen);
            _Sendto(strIp,(unsigned char*)pSend,1024);
            q->data.nAskTime = ASK_MESSAGE_TIME;
        }
        q = q->next;
    }
    if(ncheck != 0)
    {
        emit Sigbtn(2);
    }
    else
        emit Sigbtn(3);
    return ncheck;
}

//接收到注销卡回应
void CUDPRECV::_RecvLogOutCard(unsigned char *buf,char *pFromIP)
{
    unsigned char pbuf[8100] = {0};
    memcpy(pbuf,buf,8100);
    emit SigRecvLogOutCard(pbuf,QString(pFromIP));
}
//下载
void CUDPRECV::_RecvDownAllCard(unsigned char *buf,char *pFromIP)
{
//    qDebug()<<"card"<<buf[31]<<buf[32];
    unsigned char pbuf[8100] = {0};
    memcpy(pbuf,buf,8100);
    emit SigRecvDownAllCard(pbuf,QString(pFromIP));
}
//接收注册ic卡的返回
void CUDPRECV::_RecvDownCard(unsigned char *buf,char *pFromIP)
{
//    qDebug()<<"card"<<buf[31]<<buf[32];
    unsigned char pbuf[8100] = {0};
    memcpy(pbuf,buf,8100);
    emit SigRecvDownCard(pbuf,QString(pFromIP));
}

void CUDPRECV::_RecvDownAloneCard(unsigned char *buf,char *pFromIP)
{
    int nAllCard = buf[28];
    emit SigRecvDownAloneCard(nAllCard);
}

//2015 03 26
//接收上传卡的返回
void CUDPRECV::_RecvUpCard(unsigned char *buf,char *pFromIP)
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
        CLIST::ItemMiddleUpdateCardToUp(pAddr,m_MiddleItem,1);
//        return;
    }*/
    unsigned char pbuf[8100] = {0};
    memcpy(pbuf,buf,8100);
    emit SigRecvUpCard(pbuf);
    _AskRecvUpCard(buf,pFromIP);
}

void CUDPRECV::_RecvUpCard_A8(unsigned char *buf, char *pFromIP)
{
    unsigned char pbuf[8100] = {0};
    memcpy(pbuf,buf,8100);
    int nAllUpCard = pbuf[32] + pbuf[33]*256 + (pbuf[34]<<8)*256 + (pbuf[35]<<16)*256;
    int nCard = pbuf[36] + pbuf[37]*256 + (pbuf[38]<<8)*256 + (pbuf[39]<<16)*256;
    int nAllPack = pbuf[40] + pbuf[41]*256 + (pbuf[42]<<8)*256 + (pbuf[43]<<16)*256;
    m_nPack++;
    char pAddr[20] = {0};

    char pAddrT[20] = {0};
    memcpy(pAddr,(char *)pbuf+8,20);
    for(int i = 0;i < nCard;i++)
    {
        QString str2;
        str2.clear();
        for(int j = 0;j<4;j++)
        {
            int kk = pbuf[48+j+i*24];
            if(kk < 16) str2 = str2 + "0";
            str2 = str2 + QString::number(kk, 16).toUpper();
            if(j != 3) str2 = str2 + ":";
        }
        memset(pAddrT,0,20);
        memcpy(pAddrT,(char *)pbuf+52+i*24,20);
        m_CardMap.insert(str2,QString(pAddrT));
    }
    if(m_nPack == nAllPack)
        emit SigRecvUpCard_A8(pbuf);

//    _AskRecvUpCardA8(buf,pFromIP);
}

//2015 03 26
//发送收到IC卡数据确认指令
void CUDPRECV::_AskRecvUpCard(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[34] = {0};
    memcpy(pSend,buf,34);
    pSend[7] = 1;
    int tSendLen = 34;
    _Sendto(strIp,pSend,tSendLen);
}
//回复设备在线指令
void CUDPRECV::_AskDeviceOnline(unsigned char *buf,char *pFromIP)
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
void CUDPRECV::_RecvInitPass(unsigned char *buf,char *pFromIP)
{
    int nFlag = buf[34];
    char gcAddr[20] = {0};
    memcpy(gcAddr,(char*)buf + 8,20);   //设备地址
    emit SigRecvInitPass(QString(gcAddr),nFlag);
}
//接收到发送普通信息返回
void CUDPRECV::_RecvSendMessage(unsigned char *buf,char *pFromIP)
{
    int nId = CLIST::ItemTenementFindSendID(buf,m_TenementItem);
    if(nId >= 0)
    {
        emit SigSendMessage(nId,1);
    }
}
//收到中间设备报警
void CUDPRECV::_MiddleAlarm(unsigned char *buf,char *pFromIP)
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
     if(CLIST::FindAlarmItem(sAlarmDevice,m_AlarmItem))
     {
         if(!CLIST::FindItemMiddle(&sAlarmDevice,m_MiddleItem))
         {
             if(m_Mysql._InsertAlarm(sAlarmDevice))
             {
             m_AlarmItem = CLIST::InsertAlarmItem(sAlarmDevice,m_AlarmItem);
             m_nAlarm ++;
             emit SigInsertAlarm();
             }
         }
     }
    _AskMiddleAlarm(buf,pFromIP);
}

//收到住户设备报警
void CUDPRECV::_DeviceAlarm(unsigned char *buf,char *pFromIP)
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
        if(kk < 16)
            str = str + "0";
        str = str + QString::number(kk, 16).toUpper();
        if(i != 5)
            str = str + ":";
    }

    QByteArray byte;
    byte = str.toAscii();

    memcpy(sAlarmDevice.gcMacAddr,byte.data(),byte.size());
    memcpy(sAlarmDevice.gcIpAddr,pFromIP,strlen(pFromIP));

    sAlarmDevice.nFenceId = buf[37];    //报警防区
    sAlarmDevice.nTenementId = 0;       //对应在线表ID

    str.clear();
    str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    byte.clear();
    byte = str.toAscii();

    memcpy(sAlarmDevice.gcStime,byte.data(),byte.size());
    if(buf[35])//特殊报警
    {
        sAlarmDevice.nAlarmType = 0;       //报警类型
        if(CLIST::FindAlarmItem(sAlarmDevice,m_AlarmItem))
        {
            //特殊报警插入
            if(!CLIST::FindItemTenement(&sAlarmDevice,m_TenementItem))
            {
                if(m_Mysql._InsertAlarm(sAlarmDevice))
                {
                    m_AlarmItem = CLIST::InsertAlarmItem(sAlarmDevice,m_AlarmItem);
                    m_nAlarm ++;
                    emit SigInsertAlarm();
                }
            }
         }
    }
    else
    {
        if(buf[34])//是否布防
        {
            sAlarmDevice.nAlarmType = buf[36];  //报警类型
            sAlarmDevice.nFenceId = buf[37];    //报警防区
            if(CLIST::FindAlarmItem(sAlarmDevice,m_AlarmItem))
            {
                if(!CLIST::FindItemTenement(&sAlarmDevice,m_TenementItem))
                {
                    if(m_Mysql._InsertAlarm(sAlarmDevice))
                    {
                        m_AlarmItem = CLIST::InsertAlarmItem(sAlarmDevice,m_AlarmItem);
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
void CUDPRECV::_AskMiddleAlarm(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[34] = {0};
    memcpy(pSend,buf,34);
    pSend[7] = 2;
    int tSendLen = 34;
    _Sendto(strIp,pSend,tSendLen);
}
//回复住户设备报警
void CUDPRECV::_AskAlarm(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[72] = {0};
    memcpy(pSend,buf,38);
    pSend[7] = 2;
    int tSendLen = 72;
    _Sendto(strIp,pSend,tSendLen);
}
//收到设备取消报修
void CUDPRECV::_CancelRepairs(unsigned char *buf,char *pFromIP)
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
void CUDPRECV::_AskCancelRepairs(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[64] = {0};
    memcpy(pSend,buf,64);
    pSend[7] = 2;
    int tSendLen = 64;
    _Sendto(strIp,pSend,tSendLen);
}

//收到取消报警
void CUDPRECV::_CancelAlarm(unsigned char *buf,char *pFromIP)
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

    if(m_Mysql._UpdateAlarm(sAlarmDevice))
    {
        if(CLIST::UpdataItemAlarm(sAlarmDevice,m_AlarmItem))
        {
            emit SigInsertAlarm();
        }
    }
    _AskCancelAlarm(buf,pFromIP);
}

//回复取消报警
void CUDPRECV::_AskCancelAlarm(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[30] = {0};
    memcpy(pSend,buf,30);
    pSend[7] = 2;
    int tSendLen = 30;
    _Sendto(strIp,pSend,tSendLen);
}

void CUDPRECV::_UpCallPic(unsigned char *buf,char *pFromIP)
{

}

//A9刷卡---上行
void CUDPRECV::_SwipeCard(unsigned char *buf,char *pFromIP)
{
    char pIntraAddr[20] = {0};
    char gcAddrExplain[50] = {0};
    memcpy(pIntraAddr,(char*)buf+8,20);
//    qDebug()<<pIntraAddr;
    QString strCardNum = "00:";
    for(int i = 0;i<4;i++)
    {
        int k = buf[28+i];
        if(k < 16)
            strCardNum = strCardNum + "0";
        strCardNum = strCardNum + QString::number(k, 16).toUpper();
        if(i != 3)
            strCardNum = strCardNum + ":";
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
    m_Mysql._InsertCardRecord(strType, strIntra, strCardNum, strAddrExplain, m_Mysql._FindCard(strCardNum));

    QString strIp = QString(pFromIP);
    unsigned char pSend[33] = {0};
    memcpy(pSend,buf,33);
    pSend[7] = 2;
    int tSendLen = 33;
    _Sendto(strIp,pSend,tSendLen);
}

//A9---26:处理门口机报警
void CUDPRECV::_DealAlarm(char *pBuf,int nAlarmType,QString strIp)
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

//发送控制灯A9---6:家居控制
void CUDPRECV::_SendLamp(char *pBuf,QString strIp)
{
    unsigned char gSend[37] = {0};
    memcpy(gSend,m_gHead,6);
    gSend[6] = 6;
    gSend[7] = 1;
    memcpy(gSend+8,(unsigned char*)pBuf,29);

    int nSendlen = 37;
    _Sendto(strIp,gSend,nSendlen);
}
//发送读地址指令A9---41:获取地址信息
void CUDPRECV::_SendRead(char *pAddr,QString strIp)
{
    unsigned char gSend[28] = {0};
    memcpy(gSend,m_gHead,6);
    gSend[6] = 41;
    gSend[7] = 1;
    memcpy(gSend+8,(unsigned char*)pAddr,20);
    int nSendlen = 28;
    _Sendto(strIp,gSend,nSendlen);
}
//发送写地址指令A9---40:修改地址信息
void CUDPRECV::_SendWrite(char *pBuf,QString strIp)
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
void CUDPRECV::_Repairs(unsigned char *buf,char *pFromIP)
{
    char pMessage[64] = {0};
//    char pAddr[20] = {0};
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
void CUDPRECV::_AskRepairs(unsigned char *buf,char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[30] = {0};
    memcpy(pSend,buf,30);
    pSend[7] = 2;
    int tSendLen = 30;
    _Sendto(strIp,pSend,tSendLen);
}

//内部地址和显示地址的转化1
void CUDPRECV::_IntraToAddr(char *pIntra,char *pAddr,char *pType)
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

//内部地址和显示地址的转化2
void CUDPRECV::_IntraToAddrExplain(QString strIntra,char *pAddr)
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
    }
    else if(strIntra.left(1) == "M")
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

        strIntra = strIntra.right(strIntra.length() - 6);

        str.clear();
        str = strIntra.left(3);
        strAddr = strAddr + QString::number(str.toInt()) + "号设备";

        memcpy(pAddr,strAddr.toUtf8().data(),strAddr.toUtf8().size());
    }
    else if(strIntra.left(1) == "W")
    {
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
    }
    else if(strIntra.left(1) == "Z")
    {
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
    }
    else if(strIntra.left(1) == "H")
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
    }
    return ;
}

//内部地址和显示地址的转化3
QString CUDPRECV::AddrtoIntraAddrA8(QString AddrExplain, QString strType,int n)
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

//内部地址转换A8
void CUDPRECV::_IntraToAddrA8(char *pIntra,char *pAddr,char *pType)
{
    char p[20] = {0};
    switch(pIntra[0])
    {
        case 'S':
        {
            strcpy(pType, "室内机");
            memcpy(p,pIntra+1,4);
            p[4] = '-';
            memcpy(p+5,pIntra+5,2);
            p[7] = '-';
            memcpy(p+8,pIntra+7,2);
            p[10] = '-';
            memcpy(p+11,pIntra+9,2);
            p[13] = '-';
            memcpy(p+14,pIntra+11,1);
            break;
        }
        case 'M':
        {
            strcpy(pType, "门口机");
            memcpy(p,pIntra+1,4);
            p[4] = '-';
            memcpy(p+5,pIntra+5,2);
            p[7] = '-';
            memcpy(p+8,pIntra+7,1);
            break;
        }
        case 'W':
        {
            strcpy(pType, "围墙机");
            memcpy(p,pIntra+1,4);
            break;
        }
        case 'Z':
        {
            strcpy(pType, "中心机");
            memcpy(p,pIntra+1,4);
            break;
        }
        case 'H':
        {
            strcpy(pType, "二次门口机");
            memcpy(p,pIntra+1,4);
            p[4] = '-';
            memcpy(p+5,pIntra+5,2);
            p[7] = '-';
            memcpy(p+8,pIntra+7,2);
            p[10] = '-';
            memcpy(p+11,pIntra+9,2);
            p[13] = '-';
            memcpy(p+14,pIntra+11,1);
            break;
        }
        default :
            break;
    }

    memcpy(pAddr,p,20);
    return;
}
//内部地址转换A8
void CUDPRECV::_IntraToAddrExplainA8(QString strIntra,char *pAddr)
{
    QString str,strAddr;
    if(strIntra.left(1) == "S"||strIntra.left(1) == "H")
    {
        str.clear();
        strAddr.clear();
        str = strIntra.left(5);
        strAddr = str + "栋";
        strIntra = strIntra.right(strIntra.length() - 5);

        str.clear();
        str = strIntra.left(2);
        strAddr = strAddr + str + "单元";
        strIntra = strIntra.right(strIntra.length() - 2);

        str.clear();
        str = strIntra.left(2);
        strAddr = strAddr + str + "楼";
        strIntra = strIntra.right(strIntra.length() - 2);

        str.clear();
        str = strIntra.left(2);
        strAddr = strAddr + str + "房";
        strIntra = strIntra.right(strIntra.length() - 2);

        str.clear();
        str = strIntra.left(1);
        strAddr = strAddr + str + "号设备";

        memcpy(pAddr,strAddr.toUtf8().data(),strAddr.toUtf8().size());
        QString strType;
        if(strIntra.left(1) == "S")
            strType = "室内机";
        else if(strIntra.left(1) == "H")
            strType = "二次门口机";
    }
    else if(strIntra.left(1) == "M")
    {
        str.clear();
        strAddr.clear();
        str.clear();
        strAddr.clear();
        str = strIntra.left(5);
        strAddr = str + "栋";
        strIntra = strIntra.right(strIntra.length() - 5);

        str.clear();
        str = strIntra.left(2);
        strAddr = strAddr + str + "单元";
        strIntra = strIntra.right(strIntra.length() - 2);

        str.clear();
        str = strIntra.left(1);
        strAddr = strAddr + str + "号设备";

        memcpy(pAddr,strAddr.toUtf8().data(),strAddr.toUtf8().size());
        QString strType = "单元门口机";
    }
    else if(strIntra.left(1) == "W")
    {
        str.clear();
        strAddr.clear();
        str.clear();
        strAddr.clear();
        str = strIntra.left(5);
        strAddr = str + "号设备";

        memcpy(pAddr,strAddr.toUtf8().data(),strAddr.toUtf8().size());
        QString strType = "围墙机";
    }
    else if(strIntra.left(1) == "Z")
    {
        str.clear();
        strAddr.clear();
        str.clear();
        strAddr.clear();
        str = strIntra.left(5);
        strAddr = str + "号设备";

        memcpy(pAddr,strAddr.toUtf8().data(),strAddr.toUtf8().size());
        QString strType = "中心机";
    }
    return ;
}

//下发报修类型A9---7:电话和报修信息下发
void CUDPRECV::_SendRepairs(int nLen,char *pMessage)
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
//管理机下发A8？
void CUDPRECV::_SendRepairsA8(int nLen,char *pMessage)
{
    unsigned char pSend[1024] = {0};
    memcpy(pSend,m_gHead,6);
    pSend[6] = 7;
    pSend[7] = 1;
    pSend[8] = 1;
    int MessageLen = 12*nLen;
    pSend[11] = MessageLen&0XFF;
    pSend[12] = (MessageLen>>8)&0XFF;
    memcpy(pSend+13,pMessage,MessageLen);
    int nSendlen = 13 + MessageLen;
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
SAlarmDevice CUDPRECV::_DeleteItemAlarm(SAlarmDevice sAlarmDevice)
{
    m_AlarmItem = CLIST::DeleteItemAlarm(&sAlarmDevice,&m_AlarmItem);
    return sAlarmDevice;
}
//通过报警设备链表获取链表里的指定设备的信息
int CUDPRECV::_GetAlarmFromItem(SAlarmDevice *sAlarmDevice,NodeAlarm *ItemAlarm)
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
//发送复位密码A9---30:复位密码
void CUDPRECV::_SendPass(QString strAddr,QString strMac,QString strIp,QString strType,int nFlag)
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
    }
    else     if(strType == "别墅室内机")
    {
        pSend[8] = 66;
    }
    else     if(strType == "门口机")
    {
        pSend[8] = 77;
    }
    else     if(strType == "围墙机")
    {
        pSend[8] = 87;
    }
    else     if(strType == "中心机")
    {
        pSend[8] = 90;
    }
    else    if(strType == "二次门口机")
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
//A8---30:复位密码
void CUDPRECV::_SendPassA8(QString strAddr, QString strIp, QString strType, int nFlag)
{
    unsigned char pSend[100] = {0};
    QString Addr;
    Addr.clear();
    memcpy(pSend,m_gHead,6);
    pSend[6] = 30;
    pSend[7] = 1;
    Addr = AddrtoIntraAddrA8(strAddr,strType,1);
    QByteArray byte;
    byte = Addr.toUtf8();
    memcpy(pSend+8,byte.data(),byte.size());
    pSend[28] = nFlag;
    _Sendto(strIp,pSend,100);
}

//清空报警链表
bool CUDPRECV::_ClearAlarm()
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
//未使用
int CUDPRECV::_DownAloneCard(QByteArray byte,char *pCard,int nAllCard)
{
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    int nSendLen = 29+nAllCard*5;
    if(CLIST::ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
    {
        unsigned char pSend[79] = {0};
        memcpy(pSend,m_gHead,6);
        pSend[6] = 71;
        pSend[7] = 1;
        memcpy(pSend+8,pAddr,20);
        pSend[28] = nAllCard;
        memcpy(pSend+29,pCard,nAllCard*5);
        return _Sendto(QString(pSendIp),pSend,nSendLen);
    }
    else
        return 0;
}


//注销卡A9---74：注销选中卡
int CUDPRECV::_LogOutCard(QByteArray byte,char *pCard,int nAllCard,int nSendCard,int nPack)
{
//    qDebug()<<"udp1111"<<pCard[0]<<pCard[1]<<pCard[2]<<pCard[3]<<pCard[4];
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    int nSendLen = 34+nSendCard*5;
    if(CLIST::ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
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

//A9---72：注册全部卡
int CUDPRECV::_DownAllCard(QByteArray byte,char *pCard,int nAllCard,int nSendCard,int nPack)
{
//    qDebug()<<"udp1111"<<pCard[0]<<pCard[1]<<pCard[2]<<pCard[3]<<pCard[4];
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    int nSendLen = 34+nSendCard*5;
    if(CLIST::ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
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
//下载IC卡到设备A9---75：注册选中卡号信息
int CUDPRECV::_DownCard(QByteArray byte,char *pCard,int nAllCard,int nSendCard,int nPack)
{
//    qDebug()<<"udp1111"<<pCard[0]<<pCard[1]<<pCard[2]<<pCard[3]<<pCard[4];
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    int nSendLen = 34+nSendCard*5;
    if(CLIST::ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
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
        }
        else
            pSend[30] = nAllCard/CARD_MAXLEN;
        pSend[31] = nPack;
        pSend[32] = nSendCard&0XFF;
        pSend[33] = (nSendCard>>8)&0XFF;
        memcpy(pSend+34,pCard,nSendCard*5);
        return _Sendto(QString(pSendIp),pSend,nSendLen);
    }
    else
    {
        return 0;
    }
}
//上传IC卡指令下发A9---70：接收卡号
int CUDPRECV::_UpCard(QByteArray byte)
{
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    if(CLIST::ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
    {
        unsigned char pSend[28] = {0};
        memcpy(pSend,m_gHead,6);
        pSend[6] = 70;
        pSend[7] = 1;
        memcpy(pSend+8,pAddr,20);
        int nSend = _Sendto(QString(pSendIp),pSend,28);
        return nSend;
    }
    else
    {
        return 0;
    }
}

//下发常用电话A9---7：电话和报修信息下发
void CUDPRECV::_SendPhone(int nLen,char *pMessage)
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


/*************************   A8   *************************/
//校验
//int CUDPRECV::_CRCvelocity(BYTE far *pucMsg,BYTE ucMsgLen)
//{
//    int wCRC = 0xffff;
//    int nPos=0;
//    while( ucMsgLen-- )
//    {
////        qDebug()<<*pucMsg<<ucMsgLen<<wCRC;
//        wCRC ^= *pucMsg++;
//        for(nPos=0;nPos<8;nPos++)
//        {
//            if((wCRC & 0x01) == 1)
//            {
//                wCRC = wCRC >>1;
//                wCRC ^= 0xA001;
//            }
//            else
//                wCRC = wCRC >>1;
//        }
//    }
//    // 该算法计算出的效验和说明书上的正好高低字节相反
////    qDebug()<<wCRC;
//    wCRC = (wCRC%0x100)*0x100+(wCRC-wCRC%0x100)/0x100;
//    return (wCRC);
//}

//室内机报警---1
void CUDPRECV::_DevcieAlarm_A8(unsigned char *buf, char *pFromIP)
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
        if(kk < 16)
            str = str + "0";
        str = str + QString::number(kk, 16).toUpper();
         if(i != 5)
            str = str + ":";
    }

    QByteArray byte;
    byte = str.toAscii();

    memcpy(sAlarmDevice.gcMacAddr,byte.data(),byte.size());
    memcpy(sAlarmDevice.gcIpAddr,pFromIP,strlen(pFromIP));

    sAlarmDevice.nFenceId = buf[37];    //报警防区
    sAlarmDevice.nTenementId = 0;       //对应在线表ID

    str.clear();
    str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    byte.clear();
    byte = str.toAscii();

    memcpy(sAlarmDevice.gcStime,byte.data(),byte.size());
    if(buf[35])//特殊报警，不考虑布防状态
    {
        sAlarmDevice.nAlarmType = 0;    //报警类型
        if(CLIST::FindAlarmItem(sAlarmDevice,m_AlarmItem))
        {
            //特殊报警插入
            if(!CLIST::FindItemTenement(&sAlarmDevice,m_TenementItem))
            {
                if(m_Mysql._InsertAlarm(sAlarmDevice))
                {
                    m_AlarmItem = CLIST::InsertAlarmItem(sAlarmDevice,m_AlarmItem);
                    m_nAlarm ++;
                    emit SigInsertAlarm();
                }
            }
        }
    }
    else
    {
        if(buf[34])//是否布防
        {
//         sAlarmDevice.nAlarmType = buf[36];
             sAlarmDevice.nFenceId = buf[37];
             if(CLIST::FindAlarmItem(sAlarmDevice,m_AlarmItem))
             {
                 if(!CLIST::FindItemTenement(&sAlarmDevice,m_TenementItem))
                 {
                     if(m_Mysql._InsertAlarm(sAlarmDevice))
                     {
                         m_AlarmItem = CLIST::InsertAlarmItem(sAlarmDevice,m_AlarmItem);
                         m_nAlarm ++;
                         emit SigInsertAlarm();
                     }
                 }
             }
         }
    }
    _AskAlarmA8(buf,pFromIP);
}

//回复室内机报警
void CUDPRECV::_AskAlarmA8(unsigned char *buf, char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[72] = {0};
    memcpy(pSend,buf,41);
    pSend[7] = 2;
    int tSendLen = 72;
    _Sendto(strIp,pSend,tSendLen);
}

//室内机取消报警---2
void CUDPRECV::_CancelAlarm_A8(unsigned char* buf, char *pFromIP)
{
    _CancelAlarm(buf, pFromIP);     //A8 和 A9 协议的“取消报警”一样
}

//普通信息---3
void CUDPRECV::_CommonInfor_A8(unsigned char *buf, char *pFromIP)
{
    _RecvSendMessage(buf, pFromIP);     //A8 A9相同
}

//设备在线报告---4
void CUDPRECV::_DeviceOnline_A8(unsigned char *buf, char *pFromIP)
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

            _IntraToAddrA8(sMiddleDevice.gcIntraAddr,sMiddleDevice.gcAddr,sMiddleDevice.gcType);
            _IntraToAddrExplainA8(strIntra,sMiddleDevice.gcAddrExplain);

            int nMark = CLIST::ItemMiddleFind(&sMiddleDevice,m_MiddleItem);//查找中间设备的状况
//            qDebug()<<"H_nMark:"<<nMark;
            switch(nMark)
            {
                case -3:
                    if(m_Mysql._UpdateMiddle(nMark,sMiddleDevice))//更新中间设备表
                    {
                        m_MiddleItem = CLIST::DeleteItemMiddle(sMiddleDevice,&m_MiddleItem);//删除中间设备
                        m_MiddleItem = CLIST::InsertItemMiddle(sMiddleDevice,m_MiddleItem);//插入中间设备
                        //2:中间设备        12：重要新闻     8：设备日志
                        if((m_nPage == 2) || (m_nPage == 12) || (m_nPage == 8))
                            emit SigInitMiddle();
                    }
                    break;
                case -2:
                    break;
                case -1:
                    if(m_Mysql._InsertMiddle(sMiddleDevice))
                    {
                        m_MiddleItem = CLIST::InsertItemMiddle(sMiddleDevice,m_MiddleItem);
                        m_nAllMiddle++;
                        if((m_nPage == 2) || (m_nPage == 12) || (m_nPage == 8))
                            emit SigInitMiddle();
                    }
                    break;
                default :
                    if(m_Mysql._UpdateMiddle(nMark,sMiddleDevice))
                    {
                        m_MiddleItem = CLIST::UpdateItemMiddle(sMiddleDevice,m_MiddleItem);//更新中间设备信息
                        if((m_nPage == 2) || (m_nPage == 12) || (m_nPage == 8))
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
            sTenementDevice.nFenceState = buf[34];      //布防状态
//            qDebug()<<"布防状态："<<sTenementDevice.nFenceState;

            _IntraToAddrA8(sTenementDevice.gcIntraAddr,sTenementDevice.gcAddr,sTenementDevice.gcType);
            _IntraToAddrExplainA8(strIntra,sTenementDevice.gcAddrExplain);

            int nMark = CLIST::ItemTenementFind(&sTenementDevice,m_TenementItem);//查找住户设备上线状态
//            qDebug()<<"S_nMark:"<<nMark;
            switch(nMark)
            {
                case -3:
                    if(m_Mysql._UpdateTenement(sTenementDevice))        //更新住户设备
                    {
                        m_TenementItem = CLIST::DeleteItemTenement(sTenementDevice,&m_TenementItem);
                        m_TenementItem = CLIST::InsertItemTenement(sTenementDevice,m_TenementItem);
                        //1:住户设备    10：普通信息     8：设备日志
                        if((m_nPage == 1) || (m_nPage == 10) || (m_nPage == 8))     //???
                        {
                            emit SigInitTenement();
                        }
                    }
                    break;
                case -2:
                    break;
                case -1:
                    if(m_Mysql._InsertTenement(sTenementDevice))        //插入住户设备
                    {
                        m_TenementItem = CLIST::InsertItemTenement(sTenementDevice,m_TenementItem);//插入住户设备
                        m_nAllTenement++;
                        if((m_nPage == 1) || (m_nPage == 10) || (m_nPage == 8))
                        {
                            emit SigInitTenement();
                        }
                    }
                    break;
                default :
                    if(m_Mysql._UpdateTenement(sTenementDevice))        //更新住户设备
                    {
                        //更新住户设备信息
                        m_TenementItem = CLIST::UpdateItemTenement(sTenementDevice,m_TenementItem);
                        if((m_nPage == 1) || (m_nPage == 10) || (m_nPage == 8))
                        {
                            QVariant var1;
                            var1.setValue(sTenementDevice);
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
    _AskDeviceOnline(buf,pFromIP);          //回复设备在线指令
}

//复位密码---30
void CUDPRECV::_RecvInitPass_A8(unsigned char *buf, char *pFromIP)
{
    //相比A9，少了网卡地址
    int nFlag = buf[28];    //复位类型
    char gcAddr[20] = {0};
    memcpy(gcAddr,(char*)buf + 8,20);   //设备地址
    emit SigRecvInitPass(QString(gcAddr),nFlag);
}

//写IC卡---54
void CUDPRECV::_WriteICCard_A8(unsigned char *buf, char *pFromIP)
{
    unsigned char pbuf[8100] = {0};
    memcpy(pbuf,buf,8100);
    emit SigRecvDownCardA8(pbuf,QString(pFromIP));
}

//54：写IC卡
int CUDPRECV::_DownCard_A8(QByteArray byte, char *pCard, int nAllCard, int nSendCard, int nPack,unsigned int nFlag)
{
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    int nSendLen = 48+nSendCard*24;
    if(CLIST::ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
    {
        unsigned char pSend[8100] = {0};
        memcpy(pSend,m_gHead,6);
        pSend[6] = 54;
        pSend[7] = 1;
        memcpy(pSend+8,pAddr,20);
        unsigned int nT = 0;
        if(nFlag == 0)
        {
            QDateTime tDateTime = QDateTime::currentDateTime();
            nT = tDateTime.time().minute()*100 + tDateTime.time().hour()*1000 + tDateTime.time().second()*10 + tDateTime.time().msec();
        }
        else
        {
            nT = nFlag;
        }
        pSend[28] = nT&0XFF;
        pSend[29] = (nT>>8)&0XFF;
        pSend[30] = (nT>>16)&0XFF;
        pSend[31] = (nT>>24)&0XFF;

        pSend[32] = nAllCard&0XFF;
        pSend[33] = (nAllCard>>8)&0XFF;
        pSend[34] = (nAllCard>>16)&0XFF;
        pSend[35] = (nAllCard>>24)&0XFF;

        pSend[36] = nSendCard&0XFF;
        pSend[37] = (nSendCard>>8)&0XFF;
        pSend[38] = (nSendCard>>16)&0XFF;
        pSend[39] = (nSendCard>>24)&0XFF;
        if(nAllCard%CARD_MAXLEN)
        {
            pSend[40] = (nAllCard/CARD_MAXLEN + 1)&0XFF;
            pSend[41] = ((nAllCard/CARD_MAXLEN + 1)>>8)&0XFF;
            pSend[42] = ((nAllCard/CARD_MAXLEN + 1)>>16)&0XFF;
            pSend[43] = ((nAllCard/CARD_MAXLEN + 1)>>24)&0XFF;
        }
        else
        {
            pSend[40] = (nAllCard/CARD_MAXLEN)&0XFF;
            pSend[41] = ((nAllCard/CARD_MAXLEN)>>8)&0XFF;
            pSend[42] = ((nAllCard/CARD_MAXLEN)>>16)&0XFF;
            pSend[43] = ((nAllCard/CARD_MAXLEN)>>24)&0XFF;
        }
        pSend[44] = nPack&0XFF;
        pSend[45] = (nPack>>8)&0XFF;
        pSend[46] = (nPack>>16)&0XFF;
        pSend[47] = (nPack>>24)&0XFF;

        memcpy(pSend+48,pCard,nSendCard*24);
        return _Sendto(QString(pSendIp),pSend,nSendLen);
    }
    else
    {
        return 0;
    }
}

//55：上传IC卡指令下发
int CUDPRECV::_UpCard_A8(QByteArray byte)
{
    char pAddr[20] = {0};
    memcpy(pAddr,byte.data(),byte.size());
    char pSendIp[20] = {0};
    if(CLIST::ItemMiddleFindIp(pAddr,pSendIp,m_MiddleItem))
    {
        unsigned char pSend[28] = {0};
        memcpy(pSend,m_gHead,6);
        pSend[6] = 55;
        pSend[7] = 1;
        memcpy(pSend+8,pAddr,20);
        int nSend = _Sendto(QString(pSendIp),pSend,28);
        return nSend;
    }
    else
    {
        return 0;
    }
}

//刷卡---56
void CUDPRECV::_SwipeCard_A8(unsigned char *buf, char *pFromIP)
{
    char pIntraAddr[20] = {0};
    char gcAddrExplain[50] = {0};
    memcpy(pIntraAddr,(char*)buf+8,20);
    QString strCardNum = "";
    for(int i = 0;i<4;i++)
    {
        int k = buf[30+i];
        if(k < 16)
            strCardNum = strCardNum + "0";
        strCardNum = strCardNum + QString::number(k, 16).toUpper();
        if(i != 3)
            strCardNum = strCardNum + ":";
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
    _IntraToAddrExplainA8(strIntra,gcAddrExplain);
    strAddrExplain = QString(gcAddrExplain);        //
    m_Mysql._InsertCardRecord(strType,strIntra,strCardNum,strAddrExplain,m_Mysql._FindCard(strCardNum));
//    m_Mysql._InsertCardRecord(strType, strIntra, strCardNum, strAddrExplain);

    QString strIp = QString(pFromIP);
    unsigned char pSend[33] = {0};
    memcpy(pSend,buf,33);
    pSend[7] = 2;
    int tSendLen = 33;
    _Sendto(strIp,pSend,tSendLen);
}

//查询设备状态
void CUDPRECV::_CheckDeviceState(unsigned char *buf, char *pFromIP)
{
    unsigned char send[40] = {0};
    memcpy(send,buf,28);
    send[7] = 1;
    send[28] = 10;  //定时时间
    send[29] = 0;
    QDateTime dateTime = QDateTime::currentDateTime();
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
    int nSendlen = 40;
    QString strIp = QString(pFromIP);
    _Sendto(strIp,send,nSendlen);
}

//设备报修
void CUDPRECV::_Repairs_A8(unsigned char *buf,char *pFromIP)
{
    char pMessage[64] = {0};
    char pAddr[20] = {0};
    char pType[20] = {0};
    QString strAddr,strType,strStime,strIntra;
    memcpy(pMessage,(char*)buf+8,20);   //地址

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

    strIntra = QString(pMessage);
    _IntraToAddrExplainA8(strIntra,pMessage);
    strAddr = QString(pMessage);
    qDebug()<<"设备报修："<<strAddr;

    memset(pMessage,0,64);
    memcpy(pMessage,(char*)buf+34,1);   //报修类型
    int RepairsType = pMessage[0];
    QSqlDatabase db = CCONNECTIONPOOLMYSQL::openConnection();
    QString SQL = "SELECT * FROM `repairs` ORDER BY Repairs_id";
    QSqlQuery query(db);
    if(query.exec(SQL))
    {
        for(int ti = 0; ti <= RepairsType; ti++)
        {
            query.next();
            strType = query.value(1).toString();//最后一条符合要求的记录的报修类型？？？
        }
    }
    CCONNECTIONPOOLMYSQL::closeConnection(db);
    QDateTime dateTime;
    dateTime = QDateTime::currentDateTime();
    strStime = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    int nFlag = 0;
    if(m_Mysql._UpdateRepairsRecord(nFlag,strAddr,strType,strStime,strIntra,QString(pType)))
    {
        emit SigRepairsRecord();
        _AskRepairs_A8(buf,pFromIP);
    }
}

//回复设备报修---8
void CUDPRECV::_AskRepairs_A8(unsigned char *buf, char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[36] = {0};
    memcpy(pSend,buf,36);
    pSend[7] = 2;
    int tSendLen = 36;
    _Sendto(strIp,pSend,tSendLen);
}

//取消设备报修---8
void CUDPRECV::_CancelRepairs_A8(unsigned char *buf, char *pFromIP)
{
    _CancelRepairs(buf, pFromIP);
}

//主机名（地址）查找---155
void CUDPRECV::_FindAddrIP(unsigned char *buf, char *pFromIP, int nRecvLen)
{
    QString strIp = QString(pFromIP);
    unsigned char pSend[57] = {0};
    memcpy(pSend,buf,32);
    pSend[7] = 2;
    pSend[32] = 1;  //地址个数

    char pAddr[20] = {0};
    char pIpAddr[20] = {0};
    memcpy(pAddr, (char*)buf+32, 20);   //要求查找IP地址
    memcpy(pSend+33, buf+32, 20);

    if(pAddr[0] == 'S')
    {
        CLIST::ItemTenementFindIp(pAddr,pIpAddr,m_TenementItem);
    }
    else
    {
        CLIST::ItemMiddleFindIp(pAddr,pIpAddr,m_MiddleItem);
    }

    QString str = QString(pIpAddr);
    QString str1 = "";

    for(int i = 0;i < 3;i++)        //解析IP地址
    {
        int j = str.indexOf(".");
        str1 = str.left(j);
        pSend[53+i] = str1.toInt();
        str = str.right(str.length() - j - 1);
        if(i == 2)
        {
            pSend[54+i] = str.toInt();
        }
    }
    _Sendto(strIp,pSend,nRecvLen+1);
}

//物业报修和报警防区---7
void CUDPRECV::_RepairsType_A8(unsigned char *buf, char *pFromIP)
{
    qDebug()<<"暂没实现2";
    unsigned char pSend[8100] = {0};
    memcpy(pSend, buf, 8);
    pSend[8] = 1;   //信息类型
    pSend[9] = 0;   //个数

    QString strIP = QString(pFromIP);
    _Sendto(strIP, pSend, 8100);
}

//读地址设置---41
void CUDPRECV::_ReadAddr_A8(unsigned char *buf, char *pFromIP)
{
    QString strIp = QString(pFromIP);
    unsigned char gSend[30] = {0};
    memcpy(gSend,buf,30);
    unsigned char buf1[8100] = {0};
    memcpy(buf1,buf,8100);
    emit SigRecvRead_A8(buf1,strIp);       //???
    int nSendlen = 30;
    _Sendto(strIp,gSend,nSendlen);
}

//单元门口机、围墙机、二次门口机发送呼叫照片->开始发送
void CUDPRECV::_StartUpCall_A8(unsigned char *buf, char *pFromIP, int nRecvLen)
{
    unsigned char pSend[1500] = {0};
    memcpy(pSend,m_gHead,6);
    pSend[6] = 60;
    pSend[7] = 2;
    memcpy(pSend+8,buf+8,nRecvLen-8);

    m_strCallName.clear();
    QDir dir;
    m_strCallName = dir.currentPath() + "/pic/" + QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz") + ".jpg";
    m_DateTime = QDateTime::currentDateTime();
    QFile file1;
    file1.setFileName(m_strCallName);
    if(file1.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        file1.close();
    }
    _Sendto(pFromIP,pSend,nRecvLen);
}

//单元门口机、围墙机、二次口机发送呼叫照片->发送数据
void CUDPRECV::_SendCall_A8(unsigned char *buf, char *pFromIP, int nRecvLen)
{
    unsigned char pSend[1500] = {0};
    memcpy(pSend,buf,nRecvLen);
    pSend[7] = 2;
    int npake = buf[52]+buf[53]*256;
    int nPack = buf[56];

    QFile file1;
    file1.setFileName(m_strCallName);
    if(file1.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        char data[1500] = {0};
        memcpy(data,(char*)buf+58,nRecvLen - 58);
        file1.write(data,npake);
        file1.close();
    }
    _Sendto(pFromIP,pSend,nRecvLen);
}

//单元门口机、围墙机、二次门口机发送呼叫照片->发送结束（成功）
void CUDPRECV::_SuccessCall_A8(unsigned char *buf, char *pFromIP, int nRecvLen)
{
    unsigned char pSend[1500] = {0};
    memcpy(pSend,buf,nRecvLen);
    pSend[7] = 2;
    _Sendto(pFromIP,pSend,nRecvLen);

    char gDialing[20] = {0};
    char gCalled[20]  = {0};
    memcpy(gDialing, buf+8, 20);
    memcpy(gCalled, buf+28, 20);
    QString strDialing, strCalled;
    strDialing = QString(gDialing);
    strCalled = QString(gCalled);
    CMYSQL CMysql;
    if(CMysql._InsertCallRecord(strDialing,strCalled,m_DateTime,m_strCallName))
    {
        emit SigCallSuccess();
    }
}
