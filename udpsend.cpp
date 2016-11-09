#include "udpsend.h"

CUDPSEND::CUDPSEND(SUser sUser,QObject *parent) :
    QThread(parent)
{
    memcpy(m_gHead,"XXXCID",6);

    m_sUser = sUser;

    QTimer *Timer1 = new QTimer();
    connect(Timer1,SIGNAL(timeout()), this, SLOT(SlotSendIdToServer()) );
    Timer1->start(SEND_TIME*1000);
}

//发送数据到0.19服务器
void CUDPSEND::_SendIdToServer()
{
    unsigned char gSend[14] = {0};
    memcpy(gSend,m_gHead,6);
    gSend[6] = 90;
    gSend[7] = 1;
    gSend[8] = (NID>>8)&0xFF;
    gSend[9] =  NID&0xFF;
    int nSendlen = 14;
    qDebug()<<NID;
    gSend[10] = (nSendlen>>8)&0xFF;
    gSend[11] = nSendlen&0xFF;
    int CRC = _CRCvelocity(gSend,nSendlen-2);
    gSend[12] = (CRC>>8)&0xFF;
    gSend[13] = CRC&0xFF;
    QString strIp = "192.168.0.19";
    _Sendto(strIp,gSend,nSendlen);
}

//信号槽：定时器
void CUDPSEND::SlotSendIdToServer()
{
    _SendIdToServer();
}

//校验
int CUDPSEND::_CRCvelocity(BYTE far *pucMsg,BYTE ucMsgLen)
{
    int wCRC = 0xffff;
    int nPos=0;
    while( ucMsgLen-- )
    {
//          qDebug()<<*pucMsg<<ucMsgLen<<wCRC;
        wCRC ^= *pucMsg++;
        for(nPos=0;nPos<8;nPos++)
        {
            if((wCRC & 0x01) == 1)
            {
                wCRC = wCRC >>1;
                wCRC ^= 0xA001;
            }
            else
                wCRC = wCRC >>1;
        }
    }
    // 该算法计算出的效验和说明书上的正好高低字节相反
//  qDebug()<<wCRC;
    wCRC = (wCRC%0x100)*0x100+(wCRC-wCRC%0x100)/0x100;
    return (wCRC);
}

//启动线程
void CUDPSEND::run()
{
    //进行版本协商，加载特定版本的socket动态链接库
    int retVal;
//    int port = 8300;
    int port = 8301;
    if((retVal = WSAStartup(MAKEWORD(2,2), &wsd)) != 0)
    {
        perror("socket1");
//        qDebug() << "WSAStartup() failed";
        return ;
    }
    Fsocket = socket(PF_INET,SOCK_DGRAM,IPPROTO_IP);
    if(Fsocket == INVALID_SOCKET)
    {
        emit SigUdpOpenFail();
        perror("socket2");
//        qDebug()<<WSAGetLastError();
        return;
    }
    bool bOptVal=FALSE;
    retVal = setsockopt(Fsocket,SOL_SOCKET,SO_BROADCAST,(char*)&bOptVal,sizeof(bool));//可以发生广播
    struct sockaddr_in FAddr;
    memset(&FAddr,0,sizeof(FAddr));
    FAddr.sin_family = AF_INET;
//    FAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    char buf[15];
    Dir = new QDir();
    path = Dir->currentPath() + "/DBSet.ini";
    ::GetPrivateProfileStringA("IpAdress", "IPSend", "000", buf, 15, path.toAscii().data());
    FAddr.sin_addr.S_un.S_addr = inet_addr((char *)buf);
    FAddr.sin_port = htons(port);
    if(bind(Fsocket,(sockaddr*)&FAddr,sizeof(FAddr))==SOCKET_ERROR)
    {
        emit SigUdpOpenFail();
        perror("socket3");
//        qDebug() <<WSAGetLastError();
        return;
    }

    while(1)
    {
        qDebug ()<<"udpSend";
        _Read();
    }
}

/*********    数据发送    *********/
int CUDPSEND::_Sendto(QString strIp,unsigned char *pSend,int nSendlen)
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

//接收到的数据
int CUDPSEND::_Read()
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

//接收数据的分析分流
void CUDPSEND::_Recv(unsigned char *buf,char *pFromIP,int nRecvLen)
{
//    qDebug()<<nRecvLen<<"接收"<<memcmp(buf,m_gHead,6);
    if((memcmp(buf,m_gHead,6) == 0) && (nRecvLen == (buf[nRecvLen-4]*256 + buf[nRecvLen-3]))&&
         (_CRCvelocity(buf,nRecvLen-2) == (buf[nRecvLen-2]*256 + buf[nRecvLen-1])))
    {
        switch(buf[7])
        {
        case 1:         //上行
            switch(buf[6])
            {
            case 1:
                qDebug()<<"室内机报警";
//                _DevcieAlarm(buf, pFromIP);
                break;
            case 2:
                qDebug()<<"取消报警";
                break;
            case 3:
                qDebug()<<"普通信息";
                break;
            case 4:
                qDebug()<<"设备在线报告";
                break;
            case 30:
                qDebug()<<"复位密码";
                break;
            case 41:
                qDebug()<<"读取设备地址设置";
                break;
            case 155:
                qDebug()<<"主机名（地址）查找";
                break;
            case 54:
                qDebug()<<"写IC卡";
                break;
            case 55:
                qDebug()<<"读IC卡";
                break;
            case 56:
                qDebug()<<"刷IC卡";
                break;
            case 5:
                qDebug()<<"查询设备状态";
                break;
            case 8:
                qDebug()<<"住户报修";
                break;
            }
            break;
        case 2:         //下行
            switch(buf[6])
            {
                case 90:
                qDebug()<<"hello"<<nRecvLen;
                    break;
            }
            break;
        default:
            break;
        }
    }
}
