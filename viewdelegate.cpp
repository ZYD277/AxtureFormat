#include "viewdelegate.h"

#include <QProgressBar>
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QDebug>
#include <QPen>
#include <QString>
#include <QPair>

ViewDelegate::ViewDelegate():m_processlList(NULL)
{

}
ViewDelegate::~ViewDelegate()
{
    m_processlList = NULL;
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
        QString t_fileName = index.data(Qt::UserRole + Global::COLUMN_STATE).toString();
        QStyleOptionProgressBar bar;
        bar.rect = option.rect;
        bar.state = QStyle::State_Enabled;
        bar.progress = index.data(Qt::DisplayRole).toInt(); //设置对应model列的值，需要自定义model
        bar.maximum =100;
        bar.minimum = 0;
        bar.textVisible = true;
        bool sig = getFileSwitchState(t_fileName);
        if(!sig)
        {
            painter->setBrush(QBrush(QColor(0xff,0,0)));
            QColor t_red(0xff,0,0);
            QPen t_pen(t_red);
            painter->setPen(t_pen);
            bar.text = QString(QStringLiteral("转换失败:%1%")).arg(bar.progress);
        }
        bar.text = QString(QStringLiteral("当前进度:%1%")).arg(bar.progress);
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

/**
 * @brief 获取进度条数据
 * @param processList数据列表
 */
void ViewDelegate::getProcessData(QList<Global::Stru_ProcessData> *processList)
{
    m_processlList = processList;
}

/**
 * @brief 获取文件转换状态(失败还是成功)
 * @param filename文件名
 * @return 返回true转换成功，失败返回false
 */
bool ViewDelegate::getFileSwitchState(QString filename) const
{
    //    for(int i = 0;i<m_processlList->size();i++)
    //    {
    //        Global::Stru_ProcessData t_process = m_processlList->at(i);
    //      if(filename == t_process.m_fileName)
    //      {
    //          if(t_process.m_succed)
    //          {
    //              return true;
    //          }
    //          else
    //          {
    //              return false;
    //          }
    //      }
    //    }
    return true;
}
