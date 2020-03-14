#include "viewmodel.h"

#include <QSize>

ViewModel::ViewModel()
{

}

ViewModel::~ViewModel()
{

}

int	ViewModel::rowCount(const QModelIndex & parent) const
{
    return m_moudelList.size();
}

int	ViewModel::columnCount(const QModelIndex & parent) const
{
    return T_ColumnCount;
}

QVariant ViewModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();

    if( row >= 0 && row < m_moudelList.size())
    {
        AxturePage axurePage = m_moudelList.at(row);
        switch(role)
        {
            case Qt::TextAlignmentRole:
                return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
                break;
            case Qt::DisplayRole:{

                switch(static_cast<TColumn>(column)){
                    case T_Index: return row + 1;break;
                    case T_FileName: return axurePage.htmlFileName;break;
                    case T_Progress: return axurePage.processData.m_progress;break;
                    default:break;
                }
                break;
            }
            case Qt::UserRole + T_Open:
            case Qt::UserRole + T_Delete:
            case Qt::UserRole + T_Switch:{
                return axurePage.htmlFilePath;break;
            }
            case Qt::UserRole + T_OccurrError:{
                return axurePage.processData.error;
                break;
            }
            case Qt::UserRole + T_Description:{
                return axurePage.processData.textDescription;
                break;
            }
            case Qt::SizeHintRole:{
                switch(static_cast<TColumn>(column)){
                    case T_Index: return QSize(40,20);break;
                    case T_FileName: return QSize(100,20);break;
                    default:break;
                }
                break;
            }
            default:break;
        }
    }
    return QVariant();
}

Qt::ItemFlags ViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if(index.column() == T_Progress)
    {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    else if((index.column() == T_Open)||index.column() == T_Delete)
    {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    }
    else if(index.column() == T_Switch)
    {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return flags;
}

QVariant ViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case T_Index:
                return QStringLiteral("编号");
            case T_FileName:
                return QStringLiteral("文件名称");
            case T_Progress:
                return QStringLiteral("转换进度");
            case T_Open:
                return QStringLiteral("打开");
            case T_Delete:
                return QStringLiteral("删除");
            case T_Switch:
                return QStringLiteral("转换");
            default:break;
        }
    }
    return QVariant();
}

void ViewModel::refreshModel()
{
    beginResetModel();
    endResetModel();
}

void ViewModel::setModelData(const QList<AxtureProject> &dataList)
{
    m_moudelList.clear();

    std::for_each(dataList.begin(),dataList.end(),[&](const AxtureProject & proj){
        m_moudelList.append(proj.pages);
    });
}
