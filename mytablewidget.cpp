#include "mytablewidget.h"
#include<QDebug>
MyTableWidget::MyTableWidget(QWidget *parent) :
    QTableWidget(parent)
{
    defaultBkColor = QColor(255,255,255);   //默认背景色
    previousColorRow = -1;
    this->setMouseTracking(true);
    connect(this,SIGNAL(cellEntered(int,int)),this,SLOT(mycellEntered(int,int)));
}

void MyTableWidget::mycellEntered(int row, int column )
{
    QTableWidgetItem *item = 0;

    item = this->item(previousColorRow, 0);
    if (item != 0)
    {
        if(this->objectName() =="QTenement")
        {
            if(this->item(previousColorRow, 10)->text() == "报警")
            {
                for (int col=0; col<this->columnCount(); col++)
                {
                    QTableWidgetItem *item = this->item(previousColorRow, col);
                    item->setBackgroundColor(Qt::red);
                }
            }else{
                for (int col=0; col<this->columnCount(); col++)
                {
                    QTableWidgetItem *item = this->item(previousColorRow, col);
                    item->setBackgroundColor(QColor(0,125,0,0));
                }
            }
        }else{
            for (int col=0; col<this->columnCount(); col++)
            {
                QTableWidgetItem *item = this->item(previousColorRow, col);
                item->setBackgroundColor(QColor(0,125,0,0));
            }
        }
    }
    //设置当前行的颜色
    item = this->item(row, column);
    if (item != 0 && !item->isSelected())
    {
        for (int col=0; col<this->columnCount(); col++)
        {
            QTableWidgetItem *item = this->item(row, col);
            item->setBackgroundColor(QColor(240,200,255));
        }
    }
    //设置行的索引
    previousColorRow = row;
}
