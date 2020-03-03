#include "viewdelegate.h"

#include <QProgressBar>
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QDebug>
#include <QPen>
#include <QString>

ViewDelegate::ViewDelegate(QObject*parent)
{

}
ViewDelegate::~ViewDelegate()
{
    delete g_process;
}
QWidget *ViewDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QItemDelegate::createEditor(parent,option,index);
}

/**
 * @brief 从moudel取出数据放到编辑器中
 * @param editor
 * @param index
 */
void ViewDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{

}

/**
 * @brief 将编辑器数据更新到moudel
 * @param editor
 * @param model
 * @param index
 */
void ViewDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{

}

/**
 * @brief 创建按钮
 * @param painter
 * @param option
 * @param index
 */
void ViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() == Global::COLUMN_OPEN)
    {
        QPixmap pixmap(QStringLiteral(":/icon/image/new_open.gif"));

        int height = (option.rect.height() - 22) / 2;
        QRect decorationRect = QRect(option.rect.left()+20 + option.rect.width()-90, option.rect.top() + height, 36, 22);
        painter->drawPixmap(decorationRect, pixmap);

    }
    else if(index.column() == Global::COLUMN_DELETE)
    {
        QPixmap pixmap(QStringLiteral(":/icon/image/new_delete.gif"));
        int height = (option.rect.height() - 22) / 2;
        QRect decorationRect = QRect(option.rect.left()+20 + option.rect.width()-90, option.rect.top() + height, 36, 22);
        painter->drawPixmap(decorationRect, pixmap);
    }
    else if(index.column() == Global::COLUMN_SWITCH)
    {

        QPixmap pixmap(QStringLiteral(":/icon/image/new_switch.png"));
        int height = (option.rect.height()-22) / 2;
        QRect decorationRect = QRect(option.rect.left()+20 + option.rect.width()-90, option.rect.top() + height, 36, 22);
        painter->drawPixmap(decorationRect, pixmap);
    }
    else if(index.column() == Global::COLUMN_STATE)
    {
        if(!index.data(Qt::UserRole + Global::SWITCH_RESULT).toBool())
        {
            QColor t_red(0xff,0,0);
            QPen t_pen(t_red);
            painter->setPen(t_pen);
        }

        QStyleOptionProgressBar bar;
        bar.rect = option.rect;
        bar.state = QStyle::State_Enabled;
        bar.progress = index.data(Qt::DisplayRole).toInt(); //设置对应model列的值，需要自定义model
        bar.maximum =100;
        bar.minimum = 0;
        bar.textVisible = true;
        bar.text = QString(QStringLiteral("已完成:%1%")).arg(bar.progress);
        bar.textAlignment = Qt::AlignCenter;
        QProgressBar pbar;
        pbar.setStyleSheet(QStringLiteral("QProgressBar{border:2px solid grey; border-radius: 5px; background-color: #FFFFFF;}QProgressBar::chunk { background-color: #05B8CC;width: 20px;}"));
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
            g_process = new QProcess;
            g_process->start("cmd.exe",arguments);  //开启新进程打开文件
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

void ViewDelegate::updateList(QString filename)
{
    emit deleteFileLine(filename);
}
