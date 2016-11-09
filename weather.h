#ifndef WEATHER_H
#define WEATHER_H

#include <QDialog>

namespace Ui {
    class CWEATHER;
}

class CWEATHER : public QDialog
{
    Q_OBJECT

public:
    explicit CWEATHER(QWidget *parent = 0);
    ~CWEATHER();
    void _Update(QString strCity,int MinTemp,int MaxTemp,int Weather);
signals:
    void SigUpdateCity(QString strCityNum,QString strCity);
    void SigUpdateWeather(int MaxTemp,int MinTemp,int Weather);
private slots:
    void on_QpbtnUpdate_clicked();
    void on_QpbtnOK_clicked();
    void on_QpbtnClose_clicked();

private:
    Ui::CWEATHER *ui;
    QString m_strCityNum;
};

#endif // WEATHER_H
