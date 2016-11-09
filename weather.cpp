#include "weather.h"
#include "ui_weather.h"
#include "mysql.h"
CWEATHER::CWEATHER(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWEATHER)
{
    ui->setupUi(this);
}

CWEATHER::~CWEATHER()
{
    delete ui;
}

void CWEATHER::_Update(QString strCity,int MinTemp,int MaxTemp,int Weather)
{
    m_strCityNum.clear();
    ui->QlineCity->setText(strCity);
    ui->QlineMaxTemp->setText(QString::number(MaxTemp));
    ui->QlineMinTemp->setText(QString::number(MinTemp));
    ui->QcomWeather->setCurrentIndex(Weather);
}

void CWEATHER::on_QpbtnUpdate_clicked()
{
    CMYSQL MySql;
    m_strCityNum = MySql._GetCityNum(ui->QlineCity->text());
    if(m_strCityNum != NULL)
    {
        emit SigUpdateCity(m_strCityNum,ui->QlineCity->text());
    }
    else
    {
        QMessageBox::warning(NULL,tr("提示"),tr("请输入正确的归属地"));
        ui->QlineCity->selectAll();
        ui->QlineCity->setFocus();
    }
}

void CWEATHER::on_QpbtnOK_clicked()
{
    emit SigUpdateWeather(ui->QlineMaxTemp->text().toInt(),ui->QlineMinTemp->text().toInt(),ui->QcomWeather->currentIndex());
}

void CWEATHER::on_QpbtnClose_clicked()
{
    close();
}
