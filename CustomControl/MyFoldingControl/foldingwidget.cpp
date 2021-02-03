#include "foldingwidget.h"

#include <QDebug>

FoldingWidget::FoldingWidget(QWidget *parent) : QWidget(parent)
  ,isHover(false),isChecked(false)
{

}

void FoldingWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    for(int i = 0; i < m_drawInfoData.size(); i++){
        BaseInfo t_baseInfo  = m_drawInfoData.at(i);

        drawRectangle(t_baseInfo,&painter);
    }
}

void FoldingWidget::drawRectangle(BaseInfo baseInfo,QPainter* painter)
{
    QRect rec;
    QPixmap pixmap;

    painter->setPen(Qt::NoPen);

    rec = QRect(getLocation(baseInfo.m_location));

    if(!baseInfo.m_style.isEmpty()){
        painter->setBrush(QBrush(QColor(switchRgbStyle(baseInfo.m_style))));
        painter->setBrush(Qt::NoBrush);
    }

    painter->drawRoundedRect(rec,-1,-1);

    if(baseInfo.m_srcImg.contains("images")){

        if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("悬浮显示"))){
            if(isHover){
                QStringList srcList = baseInfo.m_srcImg.split(".");
                QString t_hoverStyle;
                if(srcList.size() > 1)
                    t_hoverStyle = srcList.at(0) + "_mouseOver." + srcList.at(1);
                pixmap.load(":/" + t_hoverStyle);
                painter->drawPixmap(rec,pixmap);
            }

        }else{
            pixmap.load(":/" + baseInfo.m_srcImg);

            if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("下拉三角"))){
                QMatrix leftmatrix;
                if(!isChecked)
                    leftmatrix.rotate(90);//旋转的角度
                else
                    leftmatrix.rotate(180);//旋转的角度
                pixmap = pixmap.transformed(leftmatrix,Qt::SmoothTransformation);
            }

            else if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("箭头"))){
                rec = QRect(baseInfo.m_location.m_left - baseInfo.m_location.m_width,baseInfo.m_location.m_top,
                            baseInfo.m_location.m_width,baseInfo.m_location.m_height);

                QMatrix leftmatrix;

                if(!isChecked){
                    leftmatrix.rotate(90);//旋转的角度
                    rec = QRect(baseInfo.m_location.m_left - baseInfo.m_location.m_width,baseInfo.m_location.m_top,
                                baseInfo.m_location.m_height,baseInfo.m_location.m_width);           /*!< 宽高对掉 */
                }

                pixmap = pixmap.transformed(leftmatrix,Qt::SmoothTransformation);
            }else if(baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("二级菜单图标"))){

                QMatrix leftmatrix;
                if(isChecked){
                    leftmatrix.rotate(90);//旋转的角度
                    rec = QRect(rec.left(),rec.top(),baseInfo.m_location.m_height,baseInfo.m_location.m_width);
                }

                pixmap = pixmap.transformed(leftmatrix,Qt::SmoothTransformation);
            }

            painter->drawPixmap(rec,pixmap);

        }
    }

    if(!baseInfo.m_textInfo.m_text.isEmpty()){
		
		QString rightValueString;
        if(baseInfo.m_textInfo.m_text.contains("?"))
			rightValueString = baseInfo.m_textInfo.m_text.remove("?");
		else
			rightValueString = baseInfo.m_textInfo.m_text;

        QFontMetrics metrics = painter->fontMetrics();
        int mintextWidth = metrics.width(rightValueString);
        int textHeight = metrics.height();

        if(baseInfo.m_textInfo.m_color.contains("rgba"))
            painter->setPen(QColor(switchRgbStyle(baseInfo.m_textInfo.m_color)));
        else
            painter->setPen(QColor(baseInfo.m_textInfo.m_color));
        painter->drawText(QRect(rec.left(),rec.top() + rec.height()/2 - textHeight/2,mintextWidth,textHeight),rightValueString);
    }
}

void FoldingWidget::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);

   setCursor(Qt::PointingHandCursor);

   isHover = true;

   update();
}

void FoldingWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    isHover = false;

    update();
}

void FoldingWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);

}

void FoldingWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);

    emit clicKed(isChecked);

    if(isChecked)
        isChecked = false;
    else
        isChecked = true;

    update();
}

QRect FoldingWidget::getLocation(Location location)
{

    return QRect(location.m_left - m_location.m_left,location.m_top - m_location.m_top,location.m_width,location.m_height);
}

/**
 * @brief 转换RGBA颜色样式格式为16进制
 * @param srcStyle原始rgba格式
 * @return
 */
QString FoldingWidget::switchRgbStyle(QString srcStyle)
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
 * @brief 设置需要绘制的数据
 * @param drawDatas数据列表
 */
void FoldingWidget::setDrawData(QList<BaseInfo> drawInfoData,Location location)
{
    m_drawInfoData = drawInfoData;

//    qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<"\n"
//           <<
//           <<"\n";
    m_location = location;
}
