#ifndef VIEWDELEGATE_H
#define VIEWDELEGATE_H

#include <QItemDelegate>

class ViewDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ViewDelegate();
    ~ViewDelegate();

    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    void viewFile(QString filePath);
    void deleteFile(QString filePath);
    void switchSingleFile(QString filePath);

private:
    QSize m_iconSize;       /*!< 图标固定大小 */
};

#endif // VIEWDELEGATE_H
