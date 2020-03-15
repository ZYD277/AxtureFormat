#include "viewdelegate.h"

#include <QProgressBar>
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QDebug>

#include "head.h"

ViewDelegate::ViewDelegate():m_iconSize(25,25)
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
    if(index.column() == T_Open || index.column() == T_Delete || index.column() == T_Switch)
    {
        QString pixPath;
        switch(index.column()){
            case T_Open:pixPath = QString(":/icon/image/new_open.png");break;
            case T_Delete:pixPath = QString(":/icon/image/new_delete.png");break;
            case T_Switch:pixPath = QString(":/icon/image/new_switch.png");break;
            default:break;
        }

        QPixmap pixmap = QPixmap(pixPath).scaled(m_iconSize);
        QRect drawRect = QRect(QPoint(option.rect.left() + (Table_Icon_Width - pixmap.width())/2,option.rect.top() + (option.rect.height() - pixmap.height())/2), m_iconSize);
        painter->drawPixmap(drawRect,pixmap);
    }
    else if(index.column() == T_Progress)
    {
        bool error = index.data(Qt::UserRole + T_OccurrError).toBool();
        if(error)
        {
            painter->setBrush(QBrush(QColor(0xff,0,0)));
            QColor t_red(0xff,0,0);
            QPen t_pen(t_red);
            painter->setPen(t_pen);
        }

        QStyleOptionProgressBar bar;
        bar.maximum = 100;
        bar.minimum = 0;
        bar.progress = index.data(Qt::DisplayRole).toInt();
        bar.rect = option.rect;
        bar.state = QStyle::State_Enabled;
        bar.textVisible = true;
        bar.text = index.data(Qt::UserRole + T_Description).toString();
        bar.textAlignment = Qt::AlignCenter;
        painter->fillRect(option.rect,Qt::yellow);

        QProgressBar pbar;

        QApplication::style()->drawControl(QStyle::CE_ProgressBar,&bar,painter,&pbar);
    }
    else
    {
        QItemDelegate::paint(painter,option,index);
    }
}

bool ViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    if(event->type() == QEvent::MouseButtonPress && option.rect.contains(mouseEvent->pos())){
        switch(index.column()){
            case T_Open:{
                emit viewFile(index.data(Qt::UserRole+T_Open).toString());
                break;
            }
            case T_Delete:{
                emit deleteFile(index.data(Qt::UserRole + T_Delete).toString());
                break;
            }
            case T_Switch:{
                emit switchSingleFile(index.data(Qt::UserRole + T_Switch).toString());
                break;
            }
            default:break;
        }
    }
    return QItemDelegate::editorEvent(event,model,option,index);
}
