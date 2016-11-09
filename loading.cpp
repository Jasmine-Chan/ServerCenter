#include "loading.h"
#include "ui_loading.h"
#include "mysql.h"
#include "STRUCT.h"
CLOADING::CLOADING(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CLOADING)
{
    ui->setupUi(this);
    setWindowTitle("登陆系统");
    ui->QlineCity->setText("深圳");
    setWindowFlags(Qt::FramelessWindowHint );
    ui->QlinePass->setEchoMode(QLineEdit::Password);
//    ui->QpbtnLoad->setShortcut( QKeySequence::InsertParagraphSeparator );
//    ui->QpbtnClose->setShortcut(Qt::Key_Enter);
    setWindowModality(Qt::ApplicationModal);
    m_nFlag = 0;

    _Interface();       //界面处理
    ui->QlinePass->setMaxLength(6);
    ui->QlineCity->setMaxLength(10);
    connect(this,SIGNAL(SigQuit()), qApp, SLOT(quit()));

//    QHostInfo::lookupHost("www.baidu.com", this, SLOT(lookedUp(QHostInfo)));

//    QTimer *timer = new QTimer();      //为了能实时检测网络连接状态（行不通）
//    timer->start(3000);             //意外断网会保持连接一段时间！！！
//    connect(timer, SIGNAL(timeout()), this, SLOT(time()));

    QDir dir;
    QString path = dir.currentPath() + "/DBSet.ini";
    char buf[2] = {0};
    ::GetPrivateProfileStringA("Protocol", "protocol", "3", buf, 2, path.toAscii().data());
    protocol = QString(buf).toInt();
}

void CLOADING::time()
{
    QHostInfo::lookupHost("www.baidu.com", this, SLOT(lookedUp(QHostInfo)));
}

void CLOADING::lookedUp(const QHostInfo &host)   //测试网络是否畅通
{
    QDir dir;
    QString path = dir.currentPath()+"/DBSet.ini";
    if(host.error() == QHostInfo::NoError)
    {
        qDebug()<<"host:"<<host.addresses().first().toString();
        ::WritePrivateProfileStringA("Network", "net", "Yes", path.toAscii().data());
    }
    else
    {
        qDebug()<<"不通";
        ::WritePrivateProfileStringA("Network", "net", "No", path.toAscii().data());
    }
}

//界面处理
void CLOADING::_Interface()
{
    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);

    p.setRenderHint(QPainter::Antialiasing);
    int arcR = 10;
    QRect rect = this->rect();
    QPainterPath path;
    //逆时针
    path.moveTo(arcR, 0);
    path.arcTo(0, 0, arcR * 2, arcR * 2, 90.0f, 90.0f);

    path.lineTo(0, rect.height()-arcR);
    path.arcTo(0, rect.height()-arcR*2, arcR * 2, arcR * 2, 180.0f, 90.0f);

    path.lineTo(rect.width()-arcR, rect.height());
    path.arcTo(rect.width()-arcR*2, rect.height()-arcR*2, arcR * 2, arcR * 2, 270.0f, 90.0f);

    path.lineTo(rect.width(), arcR);
    path.arcTo(rect.width() - arcR * 2, 0, arcR * 2, arcR * 2, 0.0f, 90.0f);
    path.lineTo(arcR, 0);

    p.drawPath(path);
    p.fillPath(path, QBrush(Qt::red)); //arm和windows平台没有这行代码将显示一个透明的空空的框
    setMask(bmp);//
}

void CLOADING::paintEvent(QPaintEvent *event)
{
   QPainter painter(this);
}

void CLOADING::mousePressEvent(QMouseEvent *e)
{
    pos=e->pos();
}

void CLOADING::mouseMoveEvent(QMouseEvent *e)
{
    move(e->globalPos()-pos);
}

CLOADING::~CLOADING()
{
    delete ui;
}

//刷新界面——下拉框
void CLOADING::_Update()
{
    CMYSQL MySql;
    ui->QcomUser->clear();
    T.clear();
    if(MySql._GetAllUser(&T))
    {
        for(int i = 0;i < T.size();i++)
        {
            ui->QcomUser->addItem(T.at(i));
        }
    }
    ui->QlinePass->setFocus();
}
/*登陆
  m_nFlag :为0时代表的是第一次登陆
          :不为0时代表的是注销后登陆
*/
void CLOADING::on_QpbtnLoad_clicked()
{
    SUser sUser;
    sUser.strPass = ui->QlinePass->text();
    sUser.strUser = ui->QcomUser->currentText();
    sUser.strCity = ui->QlineCity->text();
    sUser.nGrade = 0;
    CMYSQL MySql;
    if(MySql._GetUser(sUser))
    {
        if(sUser.nGrade)
        {
            if(sUser.strCityNum != NULL)
            {
                MySql._InsertSystem(1,sUser.strUser,sUser.strName);
                sUser.protocol = protocol;     //0:A8    1:A9
                if(m_nFlag == 0)
                {
                    ServerCenter = new CSERVERCENTER(sUser);
                    ServerCenter->move((QApplication::desktop()->width() - ServerCenter->width())/2, (QApplication::desktop()->height() - ServerCenter->height())/2);
                    connect(ServerCenter,SIGNAL(SigLoading()), this, SLOT(SlotLoading()));
                    this->hide();
                    ServerCenter->show();
                    m_nFlag = 1;
                }
                else
                {
                    ServerCenter->_UpdateLoading(sUser);
                    this->hide();
                    ServerCenter->show();
                    m_nFlag = 1;
                }
            }
            else
            {
                QMessageBox::warning(NULL,tr("提示"),tr("请输入正确的归属地"));
                ui->QlineCity->selectAll();
                ui->QlineCity->setFocus();
            }
        }
    }
    else
    {
        QMessageBox::warning(NULL,tr("提示"),tr("账号或密码错误"));
        ui->QlinePass->selectAll();
        ui->QlinePass->setFocus();
    }
}

void CLOADING::on_QpbtnClose_clicked()
{
    close();
}

void CLOADING::on_QtbtnClose_clicked()
{
    close();
}

void CLOADING::on_QtbtnMin_clicked()
{
    this->showMinimized();
}

//关闭提示函数
void CLOADING::closeEvent( QCloseEvent * event )
{
    switch( QMessageBox::information( this, tr("提示"),tr("确定要关闭?"),tr("Yes"), tr("No"),0, 1 ) )
    {
        case 0:
            emit SigQuit();//系统退出信号发送
            break;
        case 1:

        default:
            event->ignore();
            break;
    }
}
//系统注销，槽函数响应
void CLOADING::SlotLoading()
{
    _Update();
    ui->QlinePass->clear();
    this->show();
}
