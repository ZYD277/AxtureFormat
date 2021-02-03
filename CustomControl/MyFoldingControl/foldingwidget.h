#ifndef FOLDINGWIDGET_H
#define FOLDINGWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QEvent>

#include "head.h"

class FoldingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FoldingWidget(QWidget *parent = 0);

    void setDrawData(QList<BaseInfo> drawInfoData,Location location);

signals:
    void clicKed(bool);

public slots:

protected:
    void paintEvent(QPaintEvent* event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent * event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent* event);

private:
    QRect getLocation(Location location);
    QString switchRgbStyle(QString srcStyle);

    void drawRectangle(BaseInfo baseInfo, QPainter *painter);

private:
    QList<BaseInfo> m_drawInfoData;

    Location m_location;

    bool isHover;

    QString m_hoverImg;

    bool isChecked;

};

#endif // FOLDINGWIDGET_H
