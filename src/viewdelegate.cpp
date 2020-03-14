#include "viewdelegate.h"

#include <QProgressBar>
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QDebug>
#include <QPen>
#include <QString>
#include <QPair>

ViewDelegate::ViewDelegate()
{

}
ViewDelegate::~ViewDelegate()
{

}

QWidget *ViewDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QItemDelegate::createEditor(parent,option,index);
}

void ViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() == Global::COLUMN_OPEN)
    {
        QPixmap pixmap(QStringLiteral(":/icon/image/new_open.png"));
        int height = (option.rect.height()-35)/ 2;
        QRect decorationRect = QRect(option.rect.left()+20 + option.rect.width()-80, option.rect.top() + height, 36, 36);
        painter->drawPixmap(decorationRect, pixmap);
    }
    else if(index.column() == Global::COLUMN_DELETE)
    {
        QPixmap pixmap(QStringLiteral(":/icon/image/new_delete.png"));
        int height = (option.rect.height() - 35) / 2;
        QRect decorationRect = QRect(option.rect.left()+20 + option.rect.width()-90, option.rect.top() + height, 35, 35);
        painter->drawPixmap(decorationRect, pixmap);
    }
    else if(index.column() == Global::COLUMN_SWITCH)
    {

        QPixmap pixmap(QStringLiteral(":/icon/image/new_switch.png"));
        int height = (option.rect.height()-30) / 2;
        QRect decorationRect = QRect(option.rect.left()+20 + option.rect.width()-90, option.rect.top() + height, 40, 40);
        painter->drawPixmap(decorationRect, pixmap);
    }
    else if(index.column() == Global::COLUMN_STATE)
    {
        int t_currentValue = index.data(Qt::DisplayRole).toInt();
        bool  t_sign = index.data(Qt::UserRole + Global::COLUMN_STATE).toBool();
        int t_finishValue = index.data(Qt::UserRole + Global::COLUMN_STATEFINISH).toInt();
        QString t_processText = index.data(Qt::UserRole + Global::COLUMN_PROCESSTEXT).toString();

        QStyleOptionProgressBar bar;
//        float t_data = (float)t_currentValue/(float)t_finishValue;
        if(!t_sign)
        {
            painter->setBrush(QBrush(QColor(0xff,0,0)));
            QColor t_red(0xff,0,0);
            QPen t_pen(t_red);
            painter->setPen(t_pen);
//            bar.text = QString(QStringLiteral("转换失败:%1%")).arg(QString::number(t_data*100));
        }
        bar.maximum = t_finishValue;
        bar.minimum = 0;
        bar.progress = t_currentValue;
        bar.rect = option.rect;
        bar.state = QStyle::State_Enabled;
        bar.textVisible = true;
        bar.text = QString(QStringLiteral("%1")).arg(t_processText);
        bar.textAlignment = Qt::AlignCenter;
        QProgressBar pbar;
        QApplication::style()->drawControl(QStyle::CE_ProgressBar,&bar,painter,&pbar);
    }
    else QItemDelegate::paint(painter,option,index);
}

bool ViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    //    if(event->type() == QEvent::MouseButtonDblClick)//禁止双击编辑
    //        return true;
    QRect decorationRect = option.rect;
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    if (event->type() == QEvent::MouseButtonPress && decorationRect.contains(mouseEvent->pos()))
    {
        if (index.column() == Global::COLUMN_OPEN)
        {
            QString t_filePath = index.data(Qt::UserRole + Global::COLUMN_OPEN).toString();
            QStringList arguments;
            arguments << "/c" << t_filePath;
            QProcess g_process;
            g_process.start("cmd.exe",arguments);  //开启新进程打开文件
            g_process.waitForStarted();
            g_process.waitForFinished();//等待回收进程
        }
        if (index.column() == Global::COLUMN_DELETE)
        {
            QString t_filePath = index.data(Qt::UserRole + Global::COLUMN_DELETE).toString();
            emit deleteFileLine(t_filePath);
        }
        if(index.column() == Global::COLUMN_SWITCH)
        {
            emit switchSingleFile((index.data(Qt::UserRole + Global::COLUMN_SWITCH).toString()));
        }
    }
    return QItemDelegate::editorEvent(event,model,option,index);
}
