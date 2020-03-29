#include "viewmodel.h"

#include <QSize>
#include <QRegExp>
#include <QMessageBox>
#include <QDebug>

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
        AxurePage axurePage = m_moudelList.at(row);
        switch(role)
        {
            case Qt::TextAlignmentRole:
                return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
                break;
            case Qt::DisplayRole:{

                switch(static_cast<TColumn>(column)){
                    case T_Index: return row + 1;break;
                    case T_FileName: return axurePage.htmlFileName;break;
                    case T_ClassName:return axurePage.switchClassName;break;
                    default:break;
                }
                break;
            }
            case Qt::ToolTipRole:{
                switch(static_cast<TColumn>(column)){
                    case T_Open: {
                        if(axurePage.processData.m_progress < 100)
                            return QStringLiteral("查看原文件");
                        else
                            return QStringLiteral("查看转换文件");
                        break;
                    }
                    case T_Delete: return QStringLiteral("删除记录");break;
                    case T_Switch: return QStringLiteral("单条转换");break;
                    default:break;
                }
                break;
            }
            case Qt::UserRole + T_Progress:{
                return axurePage.processData.m_progress;
                break;
            }
            case Qt::UserRole + T_Open:{
                //NOTE 20200322 若未转换则打开html，若转换成功，则打开转换的文件夹
                if(axurePage.processData.m_progress < 100){
                    return axurePage.htmlFilePath;
                }else{
                    return axurePage.outputDir;
                }
                break;
            }
            case Qt::UserRole + T_Delete:
            case Qt::UserRole + T_Switch:{
                return axurePage.id;
                break;
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
    if(index.column() == T_ClassName){
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool ViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.isValid() && role == Qt::EditRole){
        if(index.column() >= 0 && index.column() < m_moudelList.size()){
            QRegExp classValidator("^[a-zA-Z]\\w+$");
            if(classValidator.exactMatch(value.toString())){
                emit updateItemClassName(index.row(),value.toString());
                return true;
            }else{
                emit updateItemError();
            }
        }
    }
    return false;
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
            case T_ClassName:
                return QStringLiteral("转换后类名");
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

void ViewModel::setModelData(const AxturePages &dataList)
{
    m_moudelList.clear();

    m_moudelList = dataList;
}
