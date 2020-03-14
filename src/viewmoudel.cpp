#include "viewmoudel.h"
#include <QSize>

ViewMoudel::ViewMoudel()
{

}
ViewMoudel::~ViewMoudel()
{

}

int	ViewMoudel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_moudelList.size();
}

int	ViewMoudel::columnCount(const QModelIndex & /*parent*/) const
{
    return COLUMN;
}

QVariant ViewMoudel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();
    int t_Row = index.row();
    int t_Column = index.column();
    if((t_Row >= 0)&&(t_Row < m_moudelList.size()))
    {
        AxturePage t_listFileInfo = m_moudelList.at(t_Row);
        switch(role)
        {
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
            break;
        case Qt::DisplayRole:
            if (t_Column == Global::COLUMN_NUMBER)
                return t_Row+1;
            if (t_Column == Global::COLUMN_FILENAME)
                return t_listFileInfo.htmlFileName;
            if (t_Column == Global::COLUMN_STATE)
            {
                return t_listFileInfo.processData.currentTime;
            }
            break;
        case Qt::UserRole + Global::COLUMN_OPEN:
            return t_listFileInfo.htmlFilePath;
            break;
        case Qt::UserRole + Global::COLUMN_DELETE:
            return t_listFileInfo.htmlFilePath;
            break;
        case Qt::UserRole + Global::COLUMN_STATE:
            return t_listFileInfo.processData.switchState;
            break;
        case Qt::UserRole + Global::COLUMN_STATEFINISH:
            return t_listFileInfo.processData.finishedTime;
            break;
        case Qt::UserRole + Global::COLUMN_PROCESSTEXT:
            return t_listFileInfo.processData.textDescription;
            break;
        case Qt::UserRole + Global::COLUMN_SWITCH:
            return t_listFileInfo.htmlFilePath;
            break;
        case Qt::SizeHintRole:
            if(t_Column == Global::COLUMN_NUMBER)
            {
                return QSize(40,20);
            }
            if(t_Column == Global::COLUMN_FILENAME)
            {
                return QSize(100,20);
            }
        default:
            return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags ViewMoudel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if(index.column() == Global::COLUMN_STATE)
    {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    else if((index.column() == Global::COLUMN_OPEN)||index.column() == Global::COLUMN_DELETE)
    {
         flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    }
    else if(index.column() == Global::COLUMN_SWITCH)
    {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return flags;
}

//bool ViewMoudel::setData(const QModelIndex & index, const QVariant & value, int role)
//{
//    if (!index.isValid())
//        return false;
//    int nRow = index.row();
//    Global::FileInfo t_fileinfo = m_moudelList->at(nRow);
//    switch(role)
//    {
//    case Qt::UserRole + Global::COLUMN_DELETE:
//        t_fileinfo.m_fileName = value.toString();
//        emit dataChanged(index,index);
//        break;
//    case Qt::UserRole + Global::COLUMN_FILENAME:
//        t_fileinfo.m_fileName =value.toString();
//        emit dataChanged(index,index);
//        break;
//    case Qt::UserRole + Global::COLUMN_NUMBER:
//        emit dataChanged(index,index);
//        break;
//    case Qt::UserRole + Global::COLUMN_OPEN:
//        t_fileinfo.m_fielPath = value.toString();
//        emit dataChanged(index,index);
//        break;
//    }
//    return true;
//}

QVariant ViewMoudel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case Global::COLUMN_NUMBER:
                return QStringLiteral("编号");
            case Global::COLUMN_FILENAME:
                return QStringLiteral("文件名称");
            case Global::COLUMN_STATE:
                return QStringLiteral("状态");
            case Global::COLUMN_OPEN:
                return QStringLiteral("打开");
            case Global::COLUMN_DELETE:
                return QStringLiteral("删除");
            case Global::COLUMN_SWITCH:
                return QStringLiteral("转换");
            }
        }
    }
    return QVariant();
}

/**
 * @brief 更新moudel
 */
void ViewMoudel::refrushModel()
{
    beginResetModel();
    endResetModel();
}

/**
 * @brief moudel获取数据
 * @param recvList数据链表
 */
void ViewMoudel::setModalList(QList<AxtureProject> *recvList)
{
    m_moudelList.clear();
    QList<AxtureProject> *list = recvList;
    for(int i = 0;i<list->size();i++)
    {
        AxtureProject project = list->at(i);
        for(int j = 0;j<project.pages.size();j++)
        {
            AxturePage t_fileInfo = project.pages.at(j);
            m_moudelList.append(t_fileInfo);
        }
    }
}
