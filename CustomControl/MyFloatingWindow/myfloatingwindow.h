#ifndef MYFLOATINGWINDOW_H
#define MYFLOATINGWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

#include "head.h"
class MyFloatingWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MyFloatingWindow(QWidget *parent = 0);

    void setSelfStyleSheet(FloatingWindow floatintWindow);
signals:

public slots:

private:

    void initFloatingWindow();

    void drawFloatingBtn(QPainter *painter);

    QRect getLocationInfo(Location locationInfo);

    void createButt(SwitchButton switchButtn);

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void enterEvent(QEvent * event);


private:
    FloatingWindow m_floatintWindow;

    QWidget *mainWidget;

    QWidget *floatingWindow;

    QRect m_floatingBtnRect;      /*!< 悬浮按钮区域 */

};

#endif // MYFLOATINGWINDOW_H
