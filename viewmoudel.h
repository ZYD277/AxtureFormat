#ifndef VIEWMOUDEL_H
#define VIEWMOUDEL_H

#include "global.h"
#include <QAbstractTableModel>
#include <QList>

#define COLUMN 6
class ViewMoudel : public QAbstractTableModel
{
public:
    ViewMoudel();
    ~ViewMoudel();
    //QAbstractTableModel 中3个必须重新实现的虚函数

    int	rowCount(const QModelIndex & = QModelIndex()) const;
    int	columnCount(const QModelIndex & = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
    void refrushModel();
    void setModalList(QList<Global::FileInfo> *recvList);//获取数据
private:
    QList<Global::FileInfo> *m_moudelList;
};

#endif // VIEWMOUDEL_H
