#ifndef VIEWDELEGATE_H
#define VIEWDELEGATE_H

#include "global.h"
#include "viewmoudel.h"

#include <QObject>
#include <QItemDelegate>
#include <QProcess>
#include <QList>

class ViewDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ViewDelegate();
    ~ViewDelegate();

    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

signals:
    void deleteFileLine(QString filePath);
    void switchSingleFile(QString parge);
private:
};

#endif // VIEWDELEGATE_H
