#include "viewdelegate.h"

#include <QProgressBar>
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QLineEdit>
#include <QDebug>

#include "head.h"

ViewDelegate::ViewDelegate():m_iconSize(24,24)
{

}

ViewDelegate::~ViewDelegate()
{

}

QWidget *ViewDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if(index.column() == T_ClassName){
        QLineEdit * lineEdit = new QLineEdit(parent);
        return lineEdit;
    }
    return QItemDelegate::createEditor(parent,option,index);
}

void ViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(index.column() == T_ClassName){
        QLineEdit * edit = dynamic_cast<QLineEdit *>(editor);
        if(edit){
            edit->setText(index.model()->data(index).toString());
        }
    }
}

void ViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(index.column() == T_ClassName){
        QLineEdit * edit = dynamic_cast<QLineEdit *>(editor);
        if(edit){
            model->setData(index,edit->text());
        }
    }
}

void ViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() == T_ClassName){
        QLineEdit * edit = dynamic_cast<QLineEdit *>(editor);
        if(edit){
            edit->setGeometry(option.rect);
        }
    }
}

void ViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QItemDelegate::paint(painter,option,index);

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
        //绘制初始背景
        {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->drawRect(option.rect);
            painter->restore();
        }

        int progress = index.data(Qt::UserRole + T_Progress).toInt();

        if(progress > 0){
            bool error = index.data(Qt::UserRole + T_OccurrError).toBool();

            painter->save();
            painter->setPen(Qt::NoPen);
            if(error){
                painter->setBrush(QColor("#ED683C"));
            }else{
                painter->setBrush(QColor("#22F06C"));

            }
            painter->drawRect(QRectF(option.rect.topLeft(),QSizeF(option.rect.width() / (double)100 * progress,option.rect.height())));
            painter->restore();
        }

        painter->drawText(option.rect,Qt::AlignCenter,index.data(Qt::UserRole + T_Description).toString());
    }
}

bool ViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    switch(event->type()){
        case QEvent::MouseButtonPress:{
            QMouseEvent * mouseEvent = static_cast<QMouseEvent*>(event);
            if(mouseEvent && mouseEvent->button() == Qt::LeftButton && option.rect.contains(mouseEvent->pos())){
                switch(index.column()){
                    case T_Open:{
                        emit viewFile(index.data(Qt::UserRole + T_Open).toString());
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
            break;
        }
        default:break;
    }
    return QItemDelegate::editorEvent(event,model,option,index);
}
