#ifndef MYTABLEWIDGET_H
#define MYTABLEWIDGET_H

#include <QTableWidget>

class MyTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit MyTableWidget(QWidget *parent = 0);
signals:

public slots:
    void mycellEntered(int row, int column );
 private:
    QColor defaultBkColor;   //默认背景色
    int previousColorRow;
};

#endif // MYTABLEWIDGET_H
