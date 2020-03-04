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

//void ViewDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
//{

//}

//void ViewDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
//{

//}

void ViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() == Global::COLUMN_OPEN)
    {
        QPixmap pixmap(QStringLiteral(":/icon/image/new_open.gif"));
        int height = (option.rect.height()-35)/ 2;
        QRect decorationRect = QRect(option.rect.left()+20 + option.rect.width()-80, option.rect.top() + height, 36, 36);
        painter->drawPixmap(decorationRect, pixmap);
    }
    else if(index.column() == Global::COLUMN_DELETE)
    {
        QPixmap pixmap(QStringLiteral(":/icon/image/new_delete.gif"));
        int height = (option.rect.height() - 35) / 2;
        QRect decorationRect = QRect(option.rect.left()+20 + option.rect.width()-90, option.rect.top() + height, 36, 36);
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
        bool  t_sign = index.data(Qt::UserRole + Global::COLUMN_STATE).toInt();
        int t_finishValue = index.data(Qt::UserRole + Global::COLUMN_STATEFINISH).toInt();
        QStyleOptionProgressBar bar;
        double t_data = t_currentValue/t_finishValue;
        if(!t_sign)
        {
            painter->setBrush(QBrush(QColor(0xff,0,0)));
            QColor t_red(0xff,0,0);
            QPen t_pen(t_red);
            painter->setPen(t_pen);
            bar.text = QString(QStringLiteral("转换失败:%1")).arg(QString::number(t_data));
        }
        bar.maximum = t_finishValue;
        bar.minimum = 0;
        bar.progress = t_currentValue;
        bar.rect = option.rect;
        bar.state = QStyle::State_Enabled;
        bar.textVisible = true;
        bar.text = QString(QStringLiteral("当前进度:%1%")).arg(QString::number(t_data));
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
            QString t_fileName = index.data(Qt::UserRole + Global::COLUMN_DELETE).toString();
            updateList(t_fileName);
        }
    }
    return QItemDelegate::editorEvent(event,model,option,index);
}

void ViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index)const
{
    editor->setGeometry(option.rect);
}

/**
 * @brief 删除列表指定行数据
 * @param filename删除列表文件名
 */
void ViewDelegate::updateList(QString filename)
{
    emit deleteFileLine(filename);
}
