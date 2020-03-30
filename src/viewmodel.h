#ifndef VIEWMOUDEL_H
#define VIEWMOUDEL_H

#include "global.h"
#include "head.h"

#include <QAbstractTableModel>
#include <QList>

class ViewModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ViewModel();
    ~ViewModel();

    int	rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int	columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex & index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void refreshModel();
    void setModelData(const AxturePages &dataList);

signals:
    void updateItemClassName(int row,QString newValue);
    void updateItemError();

private:
    AxturePages m_moudelList;
};

#endif // VIEWMOUDEL_H
