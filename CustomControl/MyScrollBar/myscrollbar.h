/*!
 *  @brief     自定义双向滚动条
 *  @details   使用画图事件将进条的每部分作为一个小矩形按照位置信息画出
 *             跟踪两个滑块的位置信息，逻辑判断达到双向拉动的效果
 *  @author    zyd
 *  @version   1.0
 *  @date      2022.1.15
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef MYSCROLLBAR_H
#define MYSCROLLBAR_H

#include <QWidget>
#include <QMouseEvent>

#include "head.h"

class MyScrollBar : public QWidget
{
    Q_OBJECT
public:
    explicit MyScrollBar(QWidget *parent = 0);

    enum State{ LeftHandle,RightHandle,None};

    void setSelfStyleSheet(BidirectionalSlider styleStruct);

public slots:
    void setMaxValue(float val);
    void setMinValue(float val);

signals:
    void minValueChanged(float);
    void maxValueChanged(float);

private:

    int m_min;
    int m_max;
    int m_leftValue;    /*!< 左文本值 */
    int m_rightValue;    /*!< 右文本值 */

    int leftSliderCenterX;   /*!< 左测距点 */
    int rightSliderCenterX; /*!< 右测距点 */

    int startPointX;     /*!< 滚动条起点 */
    int endPointX;       /*!< 滚动条终点 */

    State m_state;           /*!< 左右滑块类型 */

    QRect m_leftSliderRect;      /*!< 左滑块起点坐标 */
    QRect m_rightSliderRect;     /*!< 右滑块起点坐标 */

    int rollingGrooveWidget;     /*!< 滚动槽宽度 */

    BidirectionalSlider m_scrollAllInfo;     /*!< 滚动条信息 */

    Location m_location;             /*!< 滚动条基准位置 */

    int m_toolTipHight;        /*!< 有些包含文字提示的滚动条,位置坐标不准确，需要手动设置 */

private:
    void drawScrollBar(QPainter *painter);

    int getRectLeft(Location location);
    int getRectTop(Location location);
    QString switchRgbStyle(QString srcStyle);

    void drawRectangle(BaseInfo baseInfo, QPainter *painter);

    void drawScrollBarImg(BaseInfo baseInfo, int pointX, QPainter *painter);

protected:
    void paintEvent(QPaintEvent* event);
    void paintColoredRect(QRect rect, QColor color, QPainter* painter);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
};
#endif // MYSCROLLBAR_H
