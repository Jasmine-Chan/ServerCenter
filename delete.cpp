#include "delete.h"
#include "ui_delete.h"
#include "mysql.h"
#include "QMessageBox"

CDELETE::CDELETE(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDELETE)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    m_nPage = 0;
    ui->QlineAddr->setMaxLength(20);
}

CDELETE::~CDELETE()
{
    delete ui;
}

void CDELETE::_Update(int nPage)
{
    m_nPage = nPage;
    ui->QlineAddr->clear();
    switch(m_nPage)
    {
        case 7:
            setWindowTitle(tr("系统日志删除"));
            ui->label_3->setText("用户名：");
            break;
        case 3:
            setWindowTitle(tr("报警记录删除"));
            ui->label_3->setText("地址：");
            break;
        case 4:
            setWindowTitle(tr("报修记录删除"));
            ui->label_3->setText("地址：");
            break;
        case 8:
            setWindowTitle(tr("设备日志删除"));
            ui->label_3->setText("地址：");
            break;
        case 9:
            setWindowTitle(tr("刷卡记录删除"));
            ui->label_3->setText("地址：");
            break;
        case 14:
            setWindowTitle(tr("呼叫记录删除"));
            ui->label_3->setText("主叫地址：");
            break;
        default :
            break;
    }
    ui->QlineAddr->setFocus();
    QDateTime dateTime = QDateTime::currentDateTime();
    ui->QdateEtime->setDateTime(dateTime);      //setDate(dateTime.date());
    dateTime = dateTime.addDays(-5);
    ui->QdateStime->setDateTime(dateTime);      //setDate(dateTime.date());

}

void CDELETE::on_QpbtnDelete_clicked()
{
    CMYSQL CMysql;
    QDateTime DateSTime = ui->QdateStime->dateTime();
    QDateTime DateETime = ui->QdateEtime->dateTime();
    if(DateSTime < DateETime)
    {
        QString str;
        str.clear();
        if(ui->QlineAddr->text() != NULL)
        {
            str = "删除从'"+ui->QdateStime->text()+"'到'"+ui->QdateEtime->text()+"'时间段里'"+ui->QlineAddr->text()+"'设备的所有记录";
        }
        else
        {
            str = "删除从'"+ui->QdateStime->text()+"'到'"+ui->QdateEtime->text()+"'时间段里所有记录";
        }
        switch( QMessageBox::question( this, tr("提示"),str,tr("Yes"), tr("No"),0, 1 ) )
        {
            case 0:
              switch(m_nPage)
              {
                  case 3:       //报警记录删除
                      if(CMysql._DeleteAlarmRecord(ui->QlineAddr->text(),ui->QdateStime->text(),ui->QdateEtime->text()))
                      {
                          QMessageBox::information(this,tr("提示"),tr("删除成功"));
                          close();
                          emit SigDelete();
                          return;
                      }
                      else
                      {
                          QMessageBox::information(this,tr("提示"),tr("删除失败，请检查数据库连接情况"));
                          close();
                          return;
                      }
                      break;
                  case 4:       //报修记录删除
                      if(CMysql._DeleteRepairsRecord(ui->QlineAddr->text(),ui->QdateStime->text(),ui->QdateEtime->text()))
                      {
                          QMessageBox::information(this,tr("提示"),tr("删除成功"));
                          close();
                          emit SigDelete();
                          return;
                      }
                      else
                      {
                          QMessageBox::information(this,tr("提示"),tr("删除失败，请检查数据库连接情况"));
                          close();
                          return;
                      }
                      break;
                  case 7:       //系统日志删除
                      if(CMysql._DeleteSystemRecord(ui->QlineAddr->text(),ui->QdateStime->text(),ui->QdateEtime->text()))
                      {
                          QMessageBox::information(this,tr("提示"),tr("删除成功"));
                          close();
                          emit SigDelete();
                          return;
                      }
                      else
                      {
                          QMessageBox::information(this,tr("提示"),tr("删除失败，请检查数据库连接情况"));
                          close();
                          return;
                      }
                      break;
                  case 8:       //设备日志删除
                      if(CMysql._DeleteDeviceRecord(ui->QlineAddr->text(),ui->QdateStime->text(),ui->QdateEtime->text()))
                      {
                          QMessageBox::information(this,tr("提示"),tr("删除成功"));
                          close();
                          emit SigDelete();
                          return;
                      }
                      else
                      {
                          QMessageBox::information(this,tr("提示"),tr("删除失败，请检查数据库连接情况"));
                          close();
                          return;
                      }
                      break;
                  case 9:       //刷卡记录删除
                      if(CMysql._DeleteCardRecord(ui->QlineAddr->text(),ui->QdateStime->text(),ui->QdateEtime->text()))
                      {
                          QMessageBox::information(this,tr("提示"),tr("删除成功"));
                          close();
                          emit SigDelete();
                          return;
                      }
                      else
                      {
                          QMessageBox::information(this,tr("提示"),tr("删除失败，请检查数据库连接情况"));
                          close();
                          return;
                      }
                      break;
                  case 14:
                      if(CMysql._DeleteCallRecord(ui->QlineAddr->text(), ui->QdateStime->text(), ui->QdateEtime->text()))
                      {
                          QMessageBox::information(this, tr("提示"), tr("删除成功"));
                          close();
                          emit SigDelete();
                          return;
                      }
                      else
                      {
                          QMessageBox::information(this, tr("提示"), tr("删除失败，请检查数据库连接情况"));
                          close();
                          return;
                      }
                      break;
                  default :
                      break;
              }
                break;
            case 1:
            default:
                break;
        }
    }
    else
    {
        QMessageBox::information(this,tr("提示"),tr("开始时间不能大于结束时间"));
        ui->QdateStime->setFocus();
    }
}

void CDELETE::on_QpbtnClose_clicked()
{
    close();
}
