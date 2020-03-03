#ifndef VIEWDELEGATE_H
#define VIEWDELEGATE_H

#include "global.h"
#include "viewmoudel.h"

#include <QObject>
#include <QItemDelegate>
#include <QProcess>

class ViewDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ViewDelegate(QObject *parent = 0);
    ~ViewDelegate();

    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    void setEditorData(QWidget * editor, const QModelIndex & index) const;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index)const;
    void updateList(QString filename);
signals:
    void deleteFileLine(QString filename);
private:
    QProcess *g_process;
};

#endif // VIEWDELEGATE_H
