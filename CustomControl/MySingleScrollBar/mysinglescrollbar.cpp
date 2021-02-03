#include "mysinglescrollbar.h"

#include <QPainter>
#include <QDebug>
#include <QVBoxLayout>

MySingleScrollBar::MySingleScrollBar(QWidget *parent) : QWidget(parent)
  ,leftSliderCenterX(0),m_pressedSlider(false)
{

}

void MySingleScrollBar::initSingleSlider(SingleSlidingBlockData singleSliderInfo)
{
    m_singleSliderInfo = singleSliderInfo;

    m_top = m_singleSliderInfo.m_letfToolTip.m_textBox.m_location.m_height;
    m_location = singleSliderInfo.m_location;//初始化位置坐标

    setFixedSize(rect().width() + m_singleSliderInfo.m_letfToolTip.m_textBox.m_location.m_width,m_top + m_singleSliderInfo.m_leftScrollBar.m_location.m_height);

    for(int i = 0; i < m_singleSliderInfo.m_rectangularsInfo.size(); i++){

        BaseInfo t_baseInfo  = m_singleSliderInfo.m_rectangularsInfo.at(i);
        if(t_baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("滑动槽"))){
            leftSliderCenterX = getRectLeft(t_baseInfo.m_location);
            m_rollingGrooveWidth = t_baseInfo.m_location.m_width;
        }
    }

    m_currentValue = m_singleSliderInfo.m_letfToolTip.m_textBox.m_textInfo.m_text;
    m_mxValue = m_currentValue.toInt();
    m_startPointX = leftSliderCenterX;

}

void MySingleScrollBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    drawScrollBar(&painter);
}

void MySingleScrollBar::paintColoredRect(QRect rect, QColor color, QPainter *painter)
{
    painter->fillRect(rect,QBrush(color));
}

void MySingleScrollBar::drawScrollBar(QPainter *painter)
{

    //绘制基本部件

    for(int i = 0; i < m_singleSliderInfo.m_rectangularsInfo.size(); i++){
        BaseInfo t_baseInfo = m_singleSliderInfo.m_rectangularsInfo.at(i);
        drawRectangle(t_baseInfo,painter);
    }

    //绘制文字提示
    drawRectangle(m_singleSliderInfo.m_letfToolTip.m_textBox,painter);

    //绘制输入框
    BaseInfo inputInfo;
    for(int i = 0; i < m_singleSliderInfo.m_inputBox.m_inputBoxInfos.size(); i++){
        BaseInfo t_baseInfo = m_singleSliderInfo.m_inputBox.m_inputBoxInfos.at(i);
        t_baseInfo.m_dataLabel = QString::fromLocal8Bit("输入框");
        if(t_baseInfo.m_className.contains("box_1"))
            inputInfo = t_baseInfo;
        if(t_baseInfo.m_className.contains("text_field")){
            m_inputTextStyle = t_baseInfo.m_textInfo.m_color;
        }
    }
     drawRectangle(inputInfo,painter);

    //绘制滑块
    //绘制滚动条背景图片
    for(int i = 0; i < m_singleSliderInfo.m_leftScrollBar.m_srcImgs.size(); i++){
        BaseInfo baseInfo = m_singleSliderInfo.m_leftScrollBar.m_srcImgs.at(i);
        drawScrollBarImg(baseInfo,leftSliderCenterX,painter);
    }
}

void MySingleScrollBar::drawScrollBarImg(BaseInfo baseInfo,int pointX,QPainter *painter)
{
    QPixmap pixmap;

    Location t_location = baseInfo.m_location;

    m_leftSliderRect = QRect(pointX + t_location.m_left - t_location.m_width/2,m_top,t_location.m_width,t_location.m_height);

    if(baseInfo.m_srcImg.contains("images")){
         pixmap.load(":/" + baseInfo.m_srcImg);
        painter->drawPixmap(m_leftSliderRect,pixmap);
    }
}

void MySingleScrollBar::drawRectangle(BaseInfo baseInfo,QPainter *painter)
{
    QRect rect;

    QPixmap pixmap;

    Location t_location = baseInfo.m_location;

    painter->setPen(Qt::NoPen);

    painter->setBrush(Qt::NoBrush);

    if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("高亮条"))){
        rect = QRect(m_startPointX,m_top,leftSliderCenterX,t_location.m_height);
        painter->setBrush(QBrush(QColor(switchRgbStyle(baseInfo.m_style))));

    }else if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("输入框"))){

        rect = QRect(getRectLeft(m_singleSliderInfo.m_inputBox.m_location),getRectTop(m_singleSliderInfo.m_inputBox.m_location) + m_top,
                     t_location.m_width,t_location.m_height);

    }else{
        rect = QRect(getRectLeft(t_location),m_top,t_location.m_width,t_location.m_height);
    }

    if(!baseInfo.m_style.isEmpty()){
        painter->setBrush(QBrush(QColor(switchRgbStyle(baseInfo.m_style))));
    }

    painter->drawRoundedRect(rect,-1,-1);

    if(baseInfo.m_srcImg.contains("images")){

        if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("文字提示")) && m_pressedSlider){

            rect = QRect(leftSliderCenterX - t_location.m_width/2,m_startPointX,t_location.m_width,t_location.m_height);

            pixmap.load(":/" + baseInfo.m_srcImg);
            painter->drawPixmap(rect,pixmap);

            QString leftValueString = m_currentValue;
            QFontMetrics metrics = painter->fontMetrics();
            int mintextWidth = metrics.width(leftValueString);
            int textHeight = metrics.height();

            painter->setPen(QColor(baseInfo.m_textInfo.m_color));
            painter->drawText(QRect(rect.left() + t_location.m_width/2 - mintextWidth/2,rect.top() + t_location.m_height/2 - textHeight/2,
                                    mintextWidth,textHeight),leftValueString);
        }else{

            pixmap.load(":/" + baseInfo.m_srcImg);
            painter->drawPixmap(rect,pixmap);
        }
    }

    if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("输入框"))){

        QString leftValueString = m_currentValue;
        QFontMetrics metrics = painter->fontMetrics();
        int mintextWidth = metrics.width(leftValueString);
        int textHeight = metrics.height();

        painter->setPen(QColor(m_inputTextStyle));
        painter->drawText(QRect(rect.left() + t_location.m_width/2 - mintextWidth/2,rect.top() + t_location.m_height/2 - textHeight/2,
                                mintextWidth,textHeight),leftValueString);
    }
}

void MySingleScrollBar::mousePressEvent(QMouseEvent *event)
{
    if(m_leftSliderRect.contains(event->pos())){
        m_pressedSlider = true;
    }else{
        m_pressedSlider = false;
    }

    update();
}

void MySingleScrollBar::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    m_pressedSlider = false;

    update();
}

void MySingleScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton){
        int move = (event->x() - m_startPointX)*m_mxValue/m_rollingGrooveWidth;

        if(m_pressedSlider){
            leftSliderCenterX = event->x();
            if(leftSliderCenterX <= m_startPointX){
                leftSliderCenterX = m_startPointX;
            }else if(leftSliderCenterX >= m_rollingGrooveWidth + m_startPointX){
                leftSliderCenterX = m_rollingGrooveWidth;
            }
            if(move > m_mxValue)
                move = m_mxValue;
            else if(move < 0)
                move = 0;
            m_currentValue = QString::number(move);
        }
    }
    update();
}
/**
 * @brief 获取x
 * @param location
 */
int MySingleScrollBar::getRectLeft(Location location)
{
    return (location.m_left - m_location.m_left);
}

/**
 * @brief 获取y
 * @param location
 */
int MySingleScrollBar::getRectTop(Location location)
{
    return (location.m_top - m_location.m_top);
}

/**
 * @brief 转换RGBA颜色样式格式为16进制
 * @param srcStyle原始rgba格式
 * @return
 */
QString MySingleScrollBar::switchRgbStyle(QString srcStyle)
{
    QString resultStyle;
    if(srcStyle.contains("gradient")){
        QStringList t_rgbList = srcStyle.split(QRegExp("\\s+"));

        QString rgbStyle;
        QString rgbStart("rgba");//渐变色开始
        QString rgbEnd("rgba"); //渐变色结束rgb

        for(int i = 0; i < t_rgbList.size(); i++){
            QString rgbInfo = t_rgbList.at(i);
            if(!rgbInfo.contains("gradient") && !rgbInfo.contains("%")){
                rgbStyle += rgbInfo;

            }
        }

        QStringList rgbStyleS = rgbStyle.split("rgba");

        if(rgbStyleS.size() > 2){
            rgbStart += rgbStyleS.at(1);
            rgbEnd += rgbStyleS.at(2);
        }

        srcStyle = rgbEnd;//目前在渐变样式里随便取一个作为当前样式，后期可做成渐变样式

    }

    QStringList t_rgbList = srcStyle.split(",");
    if(t_rgbList.size() > 3){
        int rgb_red;
        int rgb_green;
        int rgb_blue;

        resultStyle += "#";
        QString strRed = t_rgbList.at(0);
        QStringList rgb_RList = strRed.split("(");
        if(rgb_RList.size() > 1){
            rgb_red = rgb_RList.at(1).toInt();
        }
        resultStyle += QString::number(rgb_red,16);

        rgb_green = t_rgbList.at(1).toInt();
        resultStyle += QString::number(rgb_green,16);

        rgb_blue = t_rgbList.at(2).toInt();
        resultStyle += QString::number(rgb_blue,16);
    }
    return resultStyle;
}
