#include "viewmoudel.h"

ViewMoudel::ViewMoudel():m_moudelList(NULL)
{

}
ViewMoudel::~ViewMoudel()
{
    m_moudelList = NULL;
}

/**
 * @brief moudel行数
 * @param parent
 * @return
 */
int	ViewMoudel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_moudelList->size();
}

/**
 * @brief moudel列数
 * @param parent
 * @return
 */
int	ViewMoudel::columnCount(const QModelIndex & /*parent*/) const
{
    return COLUMN;
}

/**
 * @brief 体现moudel显示值
 * @param index
 * @param role
 * @return
 */
QVariant ViewMoudel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();
    int t_Row = index.row();
    int t_Column = index.column();
    Global::FileInfo t_listFileInfo = m_moudelList->at(t_Row);
    switch(role)
    {
    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
        break;
    case Qt::DisplayRole:
        if (t_Column == Global::COLUMN_NUMBER)
            return t_Row+1;
        if (t_Column == Global::COLUMN_FILENAME)
            return t_listFileInfo.m_fileName;
        if (t_Column == Global::COLUMN_STATE)
        {
            return t_Row+1;
        }
        break;
    case Qt::UserRole + Global::COLUMN_OPEN:
        return t_listFileInfo.m_fielPath;
        break;
    case Qt::UserRole + Global::COLUMN_DELETE:
        return t_listFileInfo.m_fileName;
        break;
    case Qt::UserRole +Global::SWITCH_RESULT:
        return t_listFileInfo.m_resultSignal;
        break;
    default:
        return QVariant();
    }
    return QVariant();
}

/**
 * @brief moudel权限
 * @param index
 * @return
 */
Qt::ItemFlags ViewMoudel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if(index.column() == Global::COLUMN_STATE)
    {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    else if((index.column() == Global::COLUMN_OPEN)|index.column() == Global::COLUMN_DELETE)
    {
         flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    }
    else if(index.column() == Global::COLUMN_SWITCH)
    {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return flags;
}

/**
 * @brief 设置moudel数据
 * @param index
 * @param value
 * @param role
 * @return
 */
bool ViewMoudel::setData(const QModelIndex & index, const QVariant & value, int role)
{
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
    return true;
}

/**
 * @brief 设置表头
 * @param section列数
 * @param orientation方向
 * @param role
 * @return
 */
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
void ViewMoudel::refrushModel()
{
    beginResetModel();
    endResetModel();
}

/**
 * @brief 获取文件数据
 * @param recvList数据链表
 */
void ViewMoudel::setModalList(QList<Global::FileInfo> *recvList)
{
    m_moudelList = recvList;
}
