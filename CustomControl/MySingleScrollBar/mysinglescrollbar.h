#ifndef MYSINGLESCROLLBAR_H
#define MYSINGLESCROLLBAR_H

#include <QWidget>
#include <QMouseEvent>
#include <QLineEdit>

#include "head.h"

class MySingleScrollBar : public QWidget
{
    Q_OBJECT
public:
    explicit MySingleScrollBar(QWidget *parent = 0);

    void initSingleSlider(SingleSlidingBlockData singleSliderInfo);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    void paintColoredRect(QRect rect, QColor color, QPainter *painter);
    void drawScrollBar(QPainter *painter);
    void drawScrollBarImg(BaseInfo baseInfo,int pointX,QPainter *painter);
    void drawRectangle(BaseInfo baseInfo,QPainter *painter);
    int getRectLeft(Location location);
    int getRectTop(Location location);
    QString switchRgbStyle(QString srcStyle);

private:
    SingleSlidingBlockData m_singleSliderInfo;

    QRect m_leftSliderRect;      /*!< 左滑块起点坐标 */
    int leftSliderCenterX;   /*!< 左测距点 */
    int m_startPointX;        /*!< 起始值 */

    bool m_pressedSlider;    /*!< 鼠标按下滑块 */

    Location m_location;

    int m_top;           /*!< axure9版本组合控件下有隐藏控件时html文件提供的位置信息是错的，需要手动设置 */

    int m_mxValue;

    int m_rollingGrooveWidth;    /*!< 滚动槽宽度 */

    QString m_currentValue;      /*!< 当前刻度值 */

    QString m_inputTextStyle;         /*!< 输入框文字样式 */

};

#endif // MYSINGLESCROLLBAR_H
