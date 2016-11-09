//#ifndef ITEM_H
//#define ITEM_H
//#include "STRUCT.h"


///*========================================================================
//        Name:		。
//    ----------------------------------------------------------
//        returns:	返回城市的编码。
//    ----------------------------------------------------------
//        Remarks:
//    ----------------------------------------------------------
//        Params:         strCity 城市的名称
//==========================================================================*/
//static int ItemTenementFindSendID(unsigned char *buf,NodeTenement *ItemTenement)
//{
//    NodeTenement *q;
//    q = ItemTenement;
//    while(q->next)
//    {
//        if((memcmp(q->data.gcIntraAddr,(char *)buf + 8,20) == 0) && (q->data.nCheckedState == 1))
//        {
//            q->data.nAskTime = 0;
//            return q->data.nId;
//        }
//        q = q->next;
//    }
//    return -1;
//}

////创建链表，头插法插入节点
//static ItemTenement ItemTenementCreatH(STenementDevice data,NodeTenement *tItemTenement)
//{
//    NodeTenement *q;
//    q = (NodeTenement *)malloc(sizeof(NodeTenement));
//    q->data = data;
//    q->next = tItemTenement;
//    tItemTenement = q;
//    return tItemTenement;
//}
////创建链表
//static ItemMiddle ItemMiddleCreatH(SMiddleDevice data,NodeMiddle *tItemMiddle)
//{
//    NodeMiddle *q;
//    q = (NodeMiddle *)malloc(sizeof(NodeMiddle));
//    q->data = data;
//    q->next = tItemMiddle;
//    tItemMiddle = q;
//    return tItemMiddle;
//}
////通过内部地址查找IP地址
//static int ItemMiddleFindIp(char *Addr,char *Ip,NodeMiddle *tItemMiddle)
//{
//    NodeMiddle *q;
//    q = tItemMiddle;
//    while(q->next)
//    {
//        if(memcmp(q->data.gcIntraAddr,Addr,20) == 0)
//        {
//            memcpy(Ip,q->data.gcIpAddr,20);
//            if(q->data.nState == 0) return 0;
//            return 1;
//        }
//        q = q->next;
//    }
//    return 0;
//}
////2015 03 26
////修改IC卡状态
//static int ItemMiddleUpdateCardToUp(char *Addr,NodeMiddle *tItemMiddle,int nState)
//{
//    NodeMiddle *q;
//    q = tItemMiddle;
//    while(q->next)
//    {
//        if(memcmp(q->data.gcIntraAddr,Addr,20) == 0)
//        {
//            if(q->data.nCardState == 1)
//                q->data.nCardState = nState;
//            return 1;
//        }
//        q = q->next;
//    }
//    return 0;
//}

///*========================================================================
//        Name:		查找中间设备的状况。
//    ----------------------------------------------------------
//        returns:	-1：新设备
//                        -2：没有更改
//                        -3：地址改变
//                        其它：非地址的其它改变
//    ----------------------------------------------------------
//        Remarks:
//    ----------------------------------------------------------
//        Params:         data 中间设备的详细数据
//                        tItemMiddle 中间设备的信息链表
//==========================================================================*/
//static int ItemMiddleFind(SMiddleDevice *data,NodeMiddle *tItemMiddle)
//{
//    NodeMiddle *q;
//    q = tItemMiddle;
//    while(q->next)
//    {
//        int ti = memcmp(q->data.gcMacAddr,(*data).gcMacAddr,20);
//        if( ti == 0)
//        {
//            (*data).nCheckedState = q->data.nCheckedState;
//            (*data).nAskTime = q->data.nAskTime;
//            (*data).nId = q->data.nId;
//            (*data).nState = q->data.nState;
//            if(memcmp(q->data.gcIntraAddr,(*data).gcIntraAddr,20) != 0 )
//            {
//                memcpy((*data).gcOldAddr,q->data.gcIntraAddr,20);
//                return -3;
//            }else{
//                     if(memcmp(q->data.gcIpAddr,(*data).gcIpAddr,20) || (q->data.nState <=0) )
//                        {
//                            return q->data.nId;
//                        }else {
//                         q->data.nState = ONLINE_TIME;
//                         return -2;
//                     }
//            }
//        }
//        q = q->next;
//    }
//    return -1;
//}
////住户设备上线查找
//static int ItemTenementFind(STenementDevice *data,NodeTenement *tItemTenement)
//{
//    NodeTenement *q;
//    q = tItemTenement;
//    while(q->next)
//    {
//        if(memcmp(q->data.gcMacAddr,(*data).gcMacAddr,20) ==0)
//        {
//            (*data).nCheckedState = q->data.nCheckedState;
//            (*data).nAskTime = q->data.nAskTime;
//            (*data).nId = q->data.nId;
//            (*data).nAlarmState = q->data.nAlarmState;
//            (*data).nState = q->data.nState;
//            (*data).nOldFenceState = q->data.nFenceState;
//            if(memcmp(q->data.gcIntraAddr,(*data).gcIntraAddr,20) != 0 )
//            {
//                return -3;
//            }else{
//                     if(memcmp(q->data.gcName,(*data).gcName,64)||
//                        memcmp(q->data.gcPhone1,(*data).gcPhone1,20)||
//                        memcmp(q->data.gcPhone2,(*data).gcPhone2,20)||
//                        memcmp(q->data.gcIpAddr,(*data).gcIpAddr,20)||
//                        (q->data.nFenceState != (*data).nFenceState)||
//                        (q->data.nState <=0) )
//                     {
//                         return q->data.nId;
//                        }else {
//                         q->data.nState = ONLINE_TIME;
//                         return -2;
//                     }
//            }
//        }
//        q = q->next;
//    }
//    return -1;
//}


////删除中间设备
//static ItemMiddle DeleteItemMiddle(SMiddleDevice data,NodeMiddle **tItemMiddle)
//{
//    NodeMiddle *p,*q;
//    q = (NodeMiddle *)malloc(sizeof(NodeMiddle));
//    p = *tItemMiddle;
//    if(memcmp(p->data.gcMacAddr,data.gcMacAddr,20) == 0)
//    {
//        q = p;
//        p = p->next;
//        delete(q);
//        return p;
//    }
//    while(p->next != NULL)              //查找值为x的元素
//    {
//        if(memcmp(p->next->data.gcMacAddr,data.gcMacAddr,17) == 0)
//        {
//            q = p->next;         //删除操作，将其前驱next指向其后继。
//            p->next = q->next;
//            delete(q);
//            return *tItemMiddle;
//        }
//        q = p;
//        p = p->next;
//    }
//    return *tItemMiddle;
//}
////更新中间设备信息
//static ItemMiddle UpdateItemMiddle(SMiddleDevice data,NodeMiddle *tItemMiddle)
//{
//    NodeMiddle *q;
//    q = tItemMiddle;
//    while(q)
//    {
//        if(memcmp(q->data.gcMacAddr,data.gcMacAddr,20) ==0)
//        {
//            memcpy(q->data.gcAddr,data.gcAddr,20);
//            memcpy(q->data.gcAddrExplain,data.gcAddrExplain,40);
//            memcpy(q->data.gcIntraAddr,data.gcIntraAddr,20);
//            memcpy(q->data.gcIpAddr,data.gcIpAddr,20);
//            q->data.nState = ONLINE_TIME;
//            return tItemMiddle;
//        }
//        q = q->next;
//    }
//    return tItemMiddle;
//}
////插入新的中间设备
//static ItemMiddle InsertItemMiddle(SMiddleDevice data,NodeMiddle *tItemMiddle)
//{
//    NodeMiddle *p;
//    p = tItemMiddle;
//    data.nState = ONLINE_TIME;
//    if(p->next == NULL)
//    {
//        NodeMiddle *q;
//        q = (NodeMiddle *)malloc(sizeof(NodeMiddle));
//        q->data = data;
//        q->next = p;
//        p = q;
//        tItemMiddle = p;
//        return tItemMiddle;
//    }
//    if(strncmp(p->data.gcIntraAddr,data.gcIntraAddr,20) > 0)
//    {
//        NodeMiddle *q;
//        q = (NodeMiddle *)malloc(sizeof(NodeMiddle));
//        q->data = data;
//        q->next = p;
//        p = q;
//        tItemMiddle = p;
//        return tItemMiddle;
//    }
//    while(p->next)
//    {
//       if(strncmp(p->next->data.gcIntraAddr,data.gcIntraAddr,20) > 0)
//        {
//            NodeMiddle *q;
//            q = (NodeMiddle *)malloc(sizeof(NodeMiddle));
//            q->data = data;
//            q->next = p->next;
//            p->next = q;
//            return tItemMiddle;
//        }else
//           p = p->next;
//    }
//    NodeMiddle *q;
//    q = (NodeMiddle *)malloc(sizeof(NodeMiddle));
//    q->data = p->data;
//    q->next = NULL;
//    p->data = data;
//    p->next = q;
//    return tItemMiddle;
//}


////删除指定住户设备
//static ItemTenement DeleteItemTenement(STenementDevice data,NodeTenement **tItemTenement)
//{
//    NodeTenement *p,*q;

//    q = (NodeTenement *)malloc(sizeof(NodeTenement));
//    p = *tItemTenement;
//    if(memcmp(p->data.gcMacAddr,data.gcMacAddr,20) == 0)
//    {
//        q = p;
//        p = p->next;
//        delete(q);
//        return p;
//    }
//    while(p->next != NULL)              //查找值为x的元素
//    {
//        if(memcmp(p->next->data.gcMacAddr,data.gcMacAddr,20) == 0)
//        {
//            q = p->next;         //删除操作，将其前驱next指向其后继。
//            p->next = q->next;
//            delete(q);
//            return *tItemTenement;
//        }
//        q = p;
//        p = p->next;
//    }
//    return *tItemTenement;
//}
////更新住户设备信息
//static ItemTenement UpdateItemTenement(STenementDevice data,NodeTenement *tItemTenement)
//{
//    NodeTenement *q;
//    q = tItemTenement;
//    while(q)
//    {
//        if(memcmp(q->data.gcMacAddr,data.gcMacAddr,20) ==0)
//        {
//            memcpy(q->data.gcAddr,data.gcAddr,20);
//            memcpy(q->data.gcAddrExplain,data.gcAddrExplain,40);
//            memcpy(q->data.gcIntraAddr,data.gcIntraAddr,20);
//            memcpy(q->data.gcIpAddr,data.gcIpAddr,20);
//            memcpy(q->data.gcName,data.gcName,64);
//            memcpy(q->data.gcPhone1,data.gcPhone1,20);
//            memcpy(q->data.gcPhone2,data.gcPhone2,20);
//            q->data.nFenceState = data.nFenceState;
//            q->data.nState = ONLINE_TIME;
//            return tItemTenement;
//        }
//        q = q->next;
//    }
//    return tItemTenement;
//}
////插入住户设备
//static ItemTenement InsertItemTenement(STenementDevice data,NodeTenement *tItemTenement)
//{
//    NodeTenement *p;
//    p = tItemTenement;
//    data.nState = ONLINE_TIME;
//    if(p->next == NULL)
//    {
//        NodeTenement *q;
//        q = (NodeTenement *)malloc(sizeof(NodeTenement));
//        q->data = data;
//        q->next = p;
//        p = q;
//        tItemTenement = p;
//        return tItemTenement;
//    }
//    if(strncmp(p->data.gcIntraAddr,data.gcIntraAddr,20) > 0)
//    {
//        NodeTenement *q;
//        q = (NodeTenement *)malloc(sizeof(NodeTenement));
//        q->data = data;
//        q->next = p;
//        p = q;
//        tItemTenement = p;
//        return tItemTenement;
//    }
//    while(p->next)
//    {
//       if(strncmp(p->next->data.gcIntraAddr,data.gcIntraAddr,20) > 0)
//        {
//            NodeTenement *q;
//            q = (NodeTenement *)malloc(sizeof(NodeTenement));
//            q->data = data;
//            q->next = p->next;
//            p->next = q;
//            return tItemTenement;
//        }else
//           p = p->next;
//    }
//    NodeTenement *q;
//    q = (NodeTenement *)malloc(sizeof(NodeTenement));
//    q->data = p->data;
//    q->next = NULL;
//    p->data = data;
//    p->next = q;
//    return tItemTenement;
//}
////查找报警是否已存在链表
//static int FindAlarmItem(SAlarmDevice data,NodeAlarm *ItemAlarm)
//{
//    NodeAlarm *t;
//    t = ItemAlarm;
//    while(t->next)
//    {
//        if(memcmp(t->data.gcIntraAddr,data.gcIntraAddr,20) ==0 &&
//                memcmp(t->data.gcMacAddr,data.gcMacAddr,20) ==0 &&
//                t->data.nFenceId == data.nFenceId &&
//                t->data.nAlarmType == data.nAlarmType)
//        {
//            return 0;
//        }else
//            t = t->next;
//    }
//    return 1;
//}
////更新住户设备链表的报警状态值
//static ItemTenement AlarmUpdateTenementItem(SAlarmDevice data,NodeTenement *ItemTenement)
//{
//    NodeTenement *q;
//    q = ItemTenement;
//    while(q)
//    {
//        if(memcmp(q->data.gcIntraAddr,data.gcIntraAddr,20) ==0 &&
//                memcmp(q->data.gcMacAddr,data.gcMacAddr,20)== 0)
//        {
//            q->data.nAlarmState = 0;
//            return ItemTenement;
//        }
//        q = q->next;
//    }
//    return ItemTenement;
//}
////查找设置是否存在报警链表
//static int AddrFindAlarmItem(SAlarmDevice data,NodeAlarm *ItemAlarm)
//{
//    NodeAlarm *t;
//    t = ItemAlarm;
//    while(t->next)
//    {
//        if(memcmp(t->data.gcIntraAddr,data.gcIntraAddr,20) ==0 &&
//                memcmp(t->data.gcMacAddr,data.gcMacAddr,20) ==0
//                )
//        {
//            return 0;
//        }else
//            t = t->next;
//    }
//    return 1;
//}

////查找报警链表里是否存在该报警
//static int FindItemAlarm(SAlarmDevice data,NodeAlarm *ItemAlarm)
//{
//    NodeAlarm *p;
//    p = ItemAlarm;
//    while(p->next != NULL)              //查找值为x的元素
//    {
//        if(memcmp(p->data.gcStime, data.gcStime,20) == 0)
//        {
//            return 1;
//        }
//        p = p->next;
//    }
//    return 0;
//}

////报警设备删除
//static ItemAlarm DeleteItemAlarm(SAlarmDevice *data,NodeAlarm **ItemAlarm)
//{
//    NodeAlarm *p,*q;
//    q = (NodeAlarm *)malloc(sizeof(NodeAlarm));
//    p = *ItemAlarm;
//    if(memcmp(p->data.gcStime,(*data).gcStime,20) == 0)
//    {
//        memcpy((*data).gcIntraAddr,p->data.gcIntraAddr,20);
//        memcpy((*data).gcIpAddr,p->data.gcIpAddr,20);
//        memcpy((*data).gcAddr,p->data.gcAddr,64);
//        memcpy((*data).gcMacAddr,p->data.gcMacAddr,20);
//        (*data).nTenementId = p->data.nTenementId;
//        (*data).nAlarmType = p->data.nAlarmType;
//        (*data).nFenceId = p->data.nFenceId;
//        q = p;
//        p = p->next;
//        delete(q);
//        return p;
//    }
//    while(p->next != NULL)              //查找值为x的元素
//    {
//        if(memcmp(p->data.gcStime, (*data).gcStime,20) == 0)
//        {
//            memcpy((*data).gcIntraAddr,p->data.gcIntraAddr,20);
//            memcpy((*data).gcIpAddr,p->data.gcIpAddr,20);
//            memcpy((*data).gcAddr,p->data.gcAddr,64);
//            memcpy((*data).gcMacAddr,p->data.gcMacAddr,20);
//            (*data).nTenementId = p->data.nTenementId;
//            (*data).nAlarmType = p->data.nAlarmType;
//            (*data).nFenceId = p->data.nFenceId;
//            q = p->next;         //删除操作，将其前驱next指向其后继。
//            p->next = q->next;
//            delete(q);
//            return *ItemAlarm;
//        }
//        q = p;
//        p = p->next;
//    }
//    return *ItemAlarm;
//}
////更改设备的报警状态
//static int FindItemTenement(SAlarmDevice *data,NodeTenement *ItemTenement)
//{
//    NodeTenement *q;
//    q = ItemTenement;

//    while(q->next)
//    {
//        if(memcmp(q->data.gcIntraAddr,(*data).gcIntraAddr,20) ==0 &&
//           memcmp(q->data.gcMacAddr,(*data).gcMacAddr,20) ==0 &&
//           (q->data.nState != 0))
//        {
//            q->data.nAlarmState = 1;
//            memcpy((*data).gcName,q->data.gcName,64);
//            memcpy((*data).gcAddr,q->data.gcAddrExplain,64);
//            memcpy((*data).gcPhone1,q->data.gcPhone1,20);
//            memcpy((*data).gcType,q->data.gcType,20);
//            (*data).nTenementId = q->data.nId;
//            return 0;
//        }
//        q = q->next;
//    }
//    return 1;
//}
////查找报警设备并获取信息
//static int FindItemMiddle(SAlarmDevice *data,NodeMiddle *ItemMiddle)
//{
//    NodeMiddle *q;
//    q = ItemMiddle;

//    while(q->next)
//    {
////        qDebug()<<QString((*data).gcIntraAddr)<<QString(q->data.gcIntraAddr);
////        qDebug()<<QString((*data).gcMacAddr)<<QString(q->data.gcMacAddr);
////        qDebug()<<q->data.nState;
//        if(memcmp(q->data.gcIntraAddr,(*data).gcIntraAddr,20) ==0 &&
//           memcmp(q->data.gcMacAddr,(*data).gcMacAddr,20) ==0 &&
//           (q->data.nState != 0))
//        {
//            memcpy((*data).gcAddr,q->data.gcAddrExplain,64);
//            memcpy((*data).gcType,q->data.gcType,20);
//            return 0;
//        }
//        q = q->next;
//    }
//    return 1;
//}
////插入到报警链表
//static ItemAlarm InsertAlarmItem(SAlarmDevice data,NodeAlarm *ItemAlarm)//头插法
//{
//    NodeAlarm *q;
//    q = (NodeAlarm *)malloc(sizeof(NodeAlarm));
//    q->data = data;
//    q->next = ItemAlarm;
//    ItemAlarm = q;
//    return ItemAlarm;
//}
////更新报警链表
//static int UpdataItemAlarm(SAlarmDevice data,NodeAlarm *ItemAlarm)
//{
//    NodeAlarm *q;
//    q = ItemAlarm;
//    int ret = 0;
//    while(q->next)
//    {
//        if(memcmp(q->data.gcIntraAddr,data.gcIntraAddr,20) ==0 &&
//           memcmp(q->data.gcIpAddr,data.gcIpAddr,20) ==0)
//        {
//            memcpy(q->data.gcEtime,data.gcEtime,20);
//            ret++;
//        }
//        q = q->next;
//    }
//    return ret;
//}
//#endif // ITEM_H
