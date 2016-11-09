#ifndef LIST_H
#define LIST_H
#include "STRUCT.h"
class CLIST
{
public:
    CLIST();
    static QString nIDtoString();
    static int ItemTenementFindSendID(unsigned char *buf,NodeTenement *ItemTenement);
    static ItemTenement ItemTenementCreatH(STenementDevice data,NodeTenement *tItemTenement);
    static ItemMiddle ItemMiddleCreatH(SMiddleDevice data,NodeMiddle *tItemMiddle);
    static int ItemMiddleFindIp(char *Addr,char *Ip,NodeMiddle *tItemMiddle);
    static int ItemTenementFindIp(char *Addr,char *Ip,NodeTenement *tItemTenement);
    static int ItemMiddleUpdateCardToUp(char *Addr,NodeMiddle *tItemMiddle,int nState);
    static int ItemMiddleFind(SMiddleDevice *data,NodeMiddle *tItemMiddle);
    //住户设备上线查找
    static int ItemTenementFind(STenementDevice *data,NodeTenement *tItemTenement);
    //删除中间设备
    static ItemMiddle DeleteItemMiddle(SMiddleDevice data,NodeMiddle **tItemMiddle);
    //更新中间设备信息
    static ItemMiddle UpdateItemMiddle(SMiddleDevice data,NodeMiddle *tItemMiddle);
    //插入新的中间设备
    static ItemMiddle InsertItemMiddle(SMiddleDevice data,NodeMiddle *tItemMiddle);
    //删除指定住户设备
    static ItemTenement DeleteItemTenement(STenementDevice data,NodeTenement **tItemTenement);
    //更新住户设备信息
    static ItemTenement UpdateItemTenement(STenementDevice data,NodeTenement *tItemTenement);
    //插入住户设备
    static ItemTenement InsertItemTenement(STenementDevice data,NodeTenement *tItemTenement);
    //查找报警是否已存在链表
    static int FindAlarmItem(SAlarmDevice data,NodeAlarm *ItemAlarm);
    //更新住户设备链表的报警状态值
    static ItemTenement AlarmUpdateTenementItem(SAlarmDevice data,NodeTenement *ItemTenement);
    //查找设置是否存在报警链表
    static int AddrFindAlarmItem(SAlarmDevice data,NodeAlarm *ItemAlarm);
    //查找报警链表里是否存在该报警
    static int FindItemAlarm(SAlarmDevice data,NodeAlarm *ItemAlarm);
    //报警设备删除
    static ItemAlarm DeleteItemAlarm(SAlarmDevice *data,NodeAlarm **ItemAlarm);
    //更改设备的报警状态
    static int FindItemTenement(SAlarmDevice *data,NodeTenement *ItemTenement);
    //查找报警设备并获取信息
    static int FindItemMiddle(SAlarmDevice *data,NodeMiddle *ItemMiddle);
    //插入到报警链表
    static ItemAlarm InsertAlarmItem(SAlarmDevice data,NodeAlarm *ItemAlarm);
    //更新报警链表
    static int UpdataItemAlarm(SAlarmDevice data,NodeAlarm *ItemAlarm);

};

#endif // LIST_H
