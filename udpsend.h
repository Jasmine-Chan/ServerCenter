#ifndef UDPSEND_H
#define UDPSEND_H

#include <QThread>
#include <windows.h>
#include <Winsock2.h>
#include <WS2TCPIP.h>
#include <QtNetwork>
#include <QTimer>
#include "STRUCT.h"
#define  SEND_TIME 10*60
class CUDPSEND : public QThread
{
    Q_OBJECT

public:
    explicit CUDPSEND(SUser suser,QObject *parent = 0);
    void _SendIdToServer();

    SUser m_sUser;
    QDir *Dir;
    QString path;

protected:
    void run();

signals:
    void SigUdpOpenFail();

public slots:
    void SlotSendIdToServer();

private :
    SOCKET Fsocket;
    WSADATA wsd;
    struct sockaddr_in To;
    char m_gHead[6];
    int  _Read();
    void _Recv(unsigned char *buf,char *pFromIP,int nRecvLen);

    int _Sendto(QString strIp,unsigned char *send,int nSendlen);    //发送数据
    int _CRCvelocity(BYTE far *pucMsg,BYTE ucMsgLen);               //校验
};
#endif // UDPSEND_H
