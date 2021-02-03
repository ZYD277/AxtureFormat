#include "myscrollbar.h"

#include <QPainter>
#include <QDebug>

MyScrollBar::MyScrollBar(QWidget *parent) : QWidget(parent),m_min(0),m_max(100)
  ,m_leftValue(0),m_rightValue(100),m_state(None),leftSliderCenterX(0),rightSliderCenterX(100)
  ,m_toolTipHight(0)
{

}

void MyScrollBar::setSelfStyleSheet(BidirectionalSlider styleStruct)
{
    m_scrollAllInfo = styleStruct;
    m_location = styleStruct.m_location;//初始化位置坐标

    if(!m_scrollAllInfo.m_leftToolTip.m_ID.isEmpty()){

        m_toolTipHight = m_scrollAllInfo.m_leftToolTip.m_textBox.m_location.m_height;

        setFixedSize(rect().width() + m_scrollAllInfo.m_leftToolTip.m_textBox.m_location.m_width,m_toolTipHight + m_scrollAllInfo.m_leftScrollBar.m_location.m_height);

        m_rightValue = m_scrollAllInfo.m_leftToolTip.m_textBox.m_textInfo.m_text.toInt();

    }

    //初始化滑块位置//两边距离文本显示值
    for(int i = 0; i < m_scrollAllInfo.m_rectangularsInfo.size(); i++){

        BaseInfo t_baseInfo  = m_scrollAllInfo.m_rectangularsInfo.at(i);
        if(t_baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("滑动槽"))){
            leftSliderCenterX = getRectLeft(t_baseInfo.m_location);
            rightSliderCenterX = leftSliderCenterX + t_baseInfo.m_location.m_width;

        }else if(t_baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("距离文字框左"))){
            m_leftValue = t_baseInfo.m_textInfo.m_text.toInt();
        }else if(t_baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("距离文字框右"))){
            m_rightValue = t_baseInfo.m_textInfo.m_text.toInt();
        }
    }

    m_min = m_leftValue;
    m_max = m_rightValue;
    startPointX = leftSliderCenterX;
    endPointX = rightSliderCenterX;
    rollingGrooveWidget = rightSliderCenterX - leftSliderCenterX;

}

void MyScrollBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    drawScrollBar(&painter);
}

void MyScrollBar::paintColoredRect(QRect rect, QColor color, QPainter *painter)
{
    painter->fillRect(rect,QBrush(color));
}

void MyScrollBar::drawScrollBar(QPainter *painter)
{

    //绘制基本固定控件
    for(int i = 0; i < m_scrollAllInfo.m_rectangularsInfo.size(); i++){
        BaseInfo t_baseInfo = m_scrollAllInfo.m_rectangularsInfo.at(i);
        drawRectangle(t_baseInfo,painter);
    }

    //绘制文字提示
    if(!m_scrollAllInfo.m_leftToolTip.m_ID.isEmpty()){

        m_scrollAllInfo.m_leftToolTip.m_textBox.m_dataLabel = QString::fromLocal8Bit("左文字提示");
        drawRectangle(m_scrollAllInfo.m_leftToolTip.m_textBox,painter);

        m_scrollAllInfo.m_rightToolTip.m_textBox.m_dataLabel = QString::fromLocal8Bit("右文字提示");
        drawRectangle(m_scrollAllInfo.m_rightToolTip.m_textBox,painter);

    }
    //绘制滚动条
    int scrollWidth = m_scrollAllInfo.m_leftScrollBar.m_location.m_width;
    int scrollHeight = m_scrollAllInfo.m_leftScrollBar.m_location.m_height;


    if(scrollWidth <= 0 || scrollHeight <=0){
        for(int i = 0; i < m_scrollAllInfo.m_leftScrollBar.m_srcImgs.size(); i++){
            BaseInfo t_baseInfo = m_scrollAllInfo.m_leftScrollBar.m_srcImgs.at(i);
            scrollWidth = t_baseInfo.m_location.m_width > scrollWidth ? t_baseInfo.m_location.m_width : scrollWidth;
            scrollHeight = t_baseInfo.m_location.m_height > scrollHeight ? t_baseInfo.m_location.m_height : scrollHeight;
        }
    }

    m_leftSliderRect = QRect(leftSliderCenterX - scrollWidth/2,m_toolTipHight > 0 ? m_toolTipHight : 0,scrollWidth,scrollHeight);
    m_rightSliderRect = QRect(rightSliderCenterX - scrollWidth/2 ,m_toolTipHight > 0 ? m_toolTipHight : 0,scrollWidth,scrollHeight);

    painter->drawRoundedRect(m_leftSliderRect,-1,-1);
    painter->drawRoundedRect(m_rightSliderRect,-1,-1);

    //绘制滚动条背景图片

    for(int i = 0; i < m_scrollAllInfo.m_leftScrollBar.m_srcImgs.size(); i++){
        BaseInfo baseInfo = m_scrollAllInfo.m_leftScrollBar.m_srcImgs.at(i);
        drawScrollBarImg(baseInfo,leftSliderCenterX,painter);
    }

    for(int i = 0; i < m_scrollAllInfo.m_rightScrollBar.m_srcImgs.size(); i++){
        BaseInfo baseInfo = m_scrollAllInfo.m_rightScrollBar.m_srcImgs.at(i);
        drawScrollBarImg(baseInfo,rightSliderCenterX,painter);
    }

}

void MyScrollBar::drawScrollBarImg(BaseInfo baseInfo,int pointX,QPainter *painter)
{
    QRect rect;

    QPixmap pixmap;

    Location t_location = baseInfo.m_location;

    rect = QRect(pointX - t_location.m_width/2 + t_location.m_left,t_location.m_top + m_toolTipHight,t_location.m_width,t_location.m_height);

    if(baseInfo.m_srcImg.contains("images")){
         pixmap.load(":/" + baseInfo.m_srcImg);
        painter->drawPixmap(rect,pixmap);
    }
}

void MyScrollBar::drawRectangle(BaseInfo baseInfo,QPainter *painter)
{
    QRect rect;

    QPixmap pixmap;

    Location t_location = baseInfo.m_location;

    painter->setPen(Qt::NoPen);

    painter->setBrush(Qt::NoBrush);

    if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("高亮条"))){
        rect = QRect(leftSliderCenterX,getRectTop(t_location) + m_toolTipHight,rightSliderCenterX - leftSliderCenterX,t_location.m_height);
        painter->setBrush(QBrush(QColor(switchRgbStyle(baseInfo.m_style))));

    }else if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("滑动槽"))){


        if(!baseInfo.m_style.isEmpty()){
            painter->setBrush(QBrush(QColor(switchRgbStyle(baseInfo.m_style))));
        }
        rect = QRect(getRectLeft(t_location),getRectTop(t_location) + m_toolTipHight,t_location.m_width,t_location.m_height);

    }else{
        rect = QRect(getRectLeft(t_location),getRectTop(t_location) + m_toolTipHight,t_location.m_width,t_location.m_height);
    }



    painter->drawRoundedRect(rect,-1,-1);

    if(baseInfo.m_srcImg.contains("images")){

        if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("左文字提示")) && m_state == LeftHandle){

            rect = QRect(m_leftSliderRect.left(),0,t_location.m_width,t_location.m_height);

            pixmap.load(":/" + baseInfo.m_srcImg);
            painter->drawPixmap(rect,pixmap);

            QString leftValueString = QString::number(m_leftValue);
            QFontMetrics metrics = painter->fontMetrics();
            int mintextWidth = metrics.width(leftValueString);
            int textHeight = metrics.height();

            painter->setPen(QColor(baseInfo.m_textInfo.m_color));
            painter->drawText(QRect(rect.left() + t_location.m_width/2 - mintextWidth/2,rect.top() + t_location.m_height/2 - textHeight/2,
                                    mintextWidth,textHeight),leftValueString);

        }else if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("右文字提示")) && m_state == RightHandle){

            rect = QRect(m_rightSliderRect.left(),0,t_location.m_width,t_location.m_height);

            pixmap.load(":/" + baseInfo.m_srcImg);
            painter->drawPixmap(rect,pixmap);

            QString leftValueString = QString::number(m_rightValue);
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

    if(baseInfo.m_className.contains(QString::fromLocal8Bit("文本段落"))){

        QString textString;
        if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("距离文字框左"))){

            QString leftValueString = QString::number(m_leftValue);
            QFontMetrics metrics = painter->fontMetrics();
            int mintextWidth = metrics.width(leftValueString);
            int textHeight = metrics.height();

            painter->setPen(QColor(baseInfo.m_textInfo.m_color));
            painter->drawText(QRect(getRectLeft(t_location),getRectTop(t_location),mintextWidth,textHeight),leftValueString);

        }else if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("距离文字框右"))){

            QString rightValueString = QString::number(m_rightValue);
            QFontMetrics metrics = painter->fontMetrics();
            int mintextWidth = metrics.width(rightValueString);
            int textHeight = metrics.height();

            painter->setPen(QColor(baseInfo.m_textInfo.m_color));
            painter->drawText(QRect(getRectLeft(t_location),getRectTop(t_location),mintextWidth,textHeight),rightValueString);

        }else{
            textString = baseInfo.m_textInfo.m_text;
            painter->setPen(QColor(baseInfo.m_textInfo.m_color));
            painter->drawText(rect,textString);
        }

    }

}
/**
 * @brief 转换RGBA颜色样式格式为16进制
 * @param srcStyle原始rgba格式
 * @return
 */
QString MyScrollBar::switchRgbStyle(QString srcStyle)
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

/**
 * @brief 获取x
 * @param location
 */
int MyScrollBar::getRectLeft(Location location)
{
    return (location.m_left - m_location.m_left);
}

/**
 * @brief 获取y
 * @param location
 */
int MyScrollBar::getRectTop(Location location)
{
    return (location.m_top - m_location.m_top);
}

void MyScrollBar::mousePressEvent(QMouseEvent *event)
{
    if(m_leftSliderRect.contains(event->pos())){
        m_state = LeftHandle;
    }else if(m_rightSliderRect.contains(event->pos())){
        m_state = RightHandle;
    }else{
        m_state = None;
    }

}

void MyScrollBar::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);

    m_state = None;

    update();
}

inline float getValidValue(float val, float min, float max)
{
    float tmp = std::max(val, min);
    return std::min(tmp, max);
}

void MyScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton){
        int move = (event->x() - startPointX)*m_max/rollingGrooveWidget;

        switch(m_state){
            case LeftHandle:{
                leftSliderCenterX = event->x();
                if(leftSliderCenterX < startPointX){
                    leftSliderCenterX = startPointX;
                }else if(leftSliderCenterX > rightSliderCenterX - m_scrollAllInfo.m_leftScrollBar.m_location.m_width){
                    leftSliderCenterX = rightSliderCenterX - m_scrollAllInfo.m_leftScrollBar.m_location.m_width;
                }


                float val = getValidValue(move,m_min,m_rightValue);
                if(val > (rightSliderCenterX - m_scrollAllInfo.m_leftScrollBar.m_location.m_width - startPointX)*m_max/rollingGrooveWidget)
                    val = (rightSliderCenterX - m_scrollAllInfo.m_leftScrollBar.m_location.m_width - startPointX)*m_max/rollingGrooveWidget;
                setMinValue(val);
                break;
            }

            case RightHandle:{
                rightSliderCenterX = event->x();
                if(rightSliderCenterX < leftSliderCenterX + m_scrollAllInfo.m_leftScrollBar.m_location.m_width){
                    rightSliderCenterX = leftSliderCenterX + m_scrollAllInfo.m_leftScrollBar.m_location.m_width;
                }else if(rightSliderCenterX > endPointX){
                    rightSliderCenterX = endPointX;
                }
                float val = getValidValue(move,m_leftValue,m_max);
                if(val < (leftSliderCenterX + m_scrollAllInfo.m_leftScrollBar.m_location.m_width - startPointX)*m_max/rollingGrooveWidget)
                    val = (leftSliderCenterX + m_scrollAllInfo.m_leftScrollBar.m_location.m_width - startPointX)*m_max/rollingGrooveWidget;
                setMaxValue(val);
                break;
            }
            case None:default: break;
        }
    }
    update();
}

void MyScrollBar::setMinValue(float val)
{
    m_leftValue = val;
}

void MyScrollBar::setMaxValue(float val)
{
    m_rightValue = val;
}
