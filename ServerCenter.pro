#-------------------------------------------------
#
# Project created by QtCreator 2015-12-18T09:27:57
#
#-------------------------------------------------

QT       += core gui sql network script xml phonon

LIBS += -lwsock32
TARGET = ServerCenter
TEMPLATE = app



SOURCES += main.cpp\
        servercenter.cpp \
    udpsend.cpp \
    connectionpoolmysql.cpp \
    connectionpoolmssql.cpp \
    syncmssql.cpp \
    weather.cpp \
    user.cpp \
    update.cpp \
    smartcontrol.cpp \
    setodbc.cpp \
    repairs.cpp \
    phone.cpp \
    mytablewidget.cpp \
    mysql.cpp \
    log.cpp \
    lock.cpp \
    loading.cpp \
    list.cpp \
    findcard.cpp \
    find.cpp \
    delete.cpp \
    dealrepairs.cpp \
    dealalarm.cpp \
    card.cpp \
    about.cpp \
    udprecv.cpp \
    taesclass.cpp \
    finddevice.cpp \
    pic.cpp

HEADERS  += servercenter.h \
    udpsend.h \
    connectionpoolmysql.h \
    connectionpoolmssql.h \
    syncmssql.h \
    STRUCT.h \
    weather.h \
    user.h \
    update.h \
    smartcontrol.h \
    setodbc.h \
    repairs.h \
    phone.h \
    mytablewidget.h \
    mysql.h \
    log.h \
    lock.h \
    loading.h \
    list.h \
    findcard.h \
    find.h \
    delete.h \
    dealrepairs.h \
    dealalarm.h \
    card.h \
    about.h \
    udprecv.h \
    taesclass.h \
    finddevice.h \
    pic.h

FORMS    += \
    weather.ui \
    user.ui \
    update.ui \
    smartcontrol.ui \
    setodbc.ui \
    repairs.ui \
    phone.ui \
    lock.ui \
    loading.ui \
    findcard.ui \
    find.ui \
    delete.ui \
    dealrepairs.ui \
    dealalarm.ui \
    card.ui \
    about.ui \
    servercenter.ui \
    finddevice.ui \
    pic.ui

OTHER_FILES += \
    tu.rc \
    button.qss

RC_FILE = tu.rc

RESOURCES += \
    pic.qrc
