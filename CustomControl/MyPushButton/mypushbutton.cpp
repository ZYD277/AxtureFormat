#include "mypushbutton.h"

#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QDir>
#include <QStyle>
MyPushButton::MyPushButton(QWidget *parent) : QAbstractButton(parent),
    m_spacing(15),m_mouseEnter(false),isBtnOpenState(true)
{
    setIconSize(ICON_16);

	setCheckable(true); //用选中来表示弹框弹出

    connect(this,SIGNAL(toggled(bool)),this,SLOT(btnChecked(bool)));

    m_iMask = NORMAL;
    m_iMask |= NORMAL;
    m_iMask |= HOVER;
    m_iMask |= PRESSED;

    m_curState = NORMAL;

    initView();
}

void MyPushButton::btnChecked(bool checked)
{
    if(checked){
        m_optionDialog->showPopup();
    }else{
        m_optionDialog->hidePopup();
    }
}

void MyPushButton::initView()
{
    m_optionDialog = new OptionsBox();
    m_optionDialog->attachTarget(this);
    m_optionDialog->setTitletype(M_LABEL);

    connect(m_optionDialog,SIGNAL(setCurrentText(QString)),this,SLOT(changeText(QString)));
}

void MyPushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);

    painter.save();
    if(m_curState == NORMAL){
        //绘制背景颜色以及边框
        drawBorderBackground(&painter,m_currentBgStyle.m_default);
        //绘制背景图片
        drawImage(&painter,m_currentBgImg.m_normalPix);
        //绘制背景图标
        drawIcon(&painter,m_currentBgImg.m_normalIcon);
        //绘制文字
        drawText(&painter,m_currentFontStyle.m_default);

        painter.restore();
    }else if(m_curState == HOVER){
        //绘制背景颜色以及边框
        drawBorderBackground(&painter,m_currentBgStyle.m_hover);
        //绘制背景图片
        drawImage(&painter,m_currentBgImg.m_hoverPix);
        //绘制背景图标
        drawIcon(&painter,m_currentBgImg.m_hoverIcon);
        //绘制文字
        drawText(&painter,m_currentFontStyle.m_hover);
    }else if(m_curState == SELECTED){
        //绘制背景颜色以及边框
        drawBorderBackground(&painter,m_currentBgStyle.m_checked);
        //绘制背景图片
        drawImage(&painter,m_currentBgImg.m_selectedPix);
        //绘制背景图标
        drawIcon(&painter,m_currentBgImg.m_selectIcon);
        //绘制文字
        drawText(&painter,m_currentFontStyle.m_checked);
    }else if(m_curState == PRESSED){
        //绘制背景颜色以及边框
        drawBorderBackground(&painter,m_currentBgStyle.m_pressed);
        //绘制背景图片
        drawImage(&painter,m_currentBgImg.m_pressedPix);
        //绘制背景图标
        drawIcon(&painter,m_currentBgImg.m_pressIcon);
        //绘制文字
        drawText(&painter,m_currentFontStyle.m_pressed);
    }

}

/**
 * @brief 绘制边框以及背景颜色
 * @param painter
 */
void MyPushButton::drawBorderBackground(QPainter *painter,QString colorName)
{
    //背景
    if(!colorName.isEmpty())
        painter->setBrush(QBrush(QColor(colorName)));

    //边框
    QRect rec = rect();
    rec.adjust(1, 1, -1, -1);

    painter->drawRoundedRect(rec, 2, 2);
}

/**
 * @brief 绘制背景图片
 * @param painter
 */
void MyPushButton::drawImage(QPainter*painter,QPixmap pixmap)
{
    if(!pixmap.isNull())
        painter->drawPixmap(0,0,width(),height(),pixmap);
}

/**
 * @brief 绘制背景图标以及文字
 * @param painter
 * @param pixmap图标
 */
void MyPushButton::drawIcon(QPainter *painter,QPixmap pixmap)
{
    //计算图标位置
    QMargins margins = contentsMargins();
    m_startPoint.setX(margins.left());
    m_startPoint.setY(margins.top());

    //若控件尺寸大于最小绘制尺寸,则将其居中绘制
    QSize minSize = calcMiniumSize();
    if (width() > minSize.width())
        m_startPoint.setX(m_startPoint.x() + (width() - minSize.width()) / 2);

    if (height() > minSize.height())
        m_startPoint.setY(m_startPoint.y() + (height() - minSize.height()) / 2);

    if(!pixmap.isNull()){
        QPixmap pic = pixmap;
        int picY = (rect().height()) / 2;

        const QFont & ft = font();
        QFontMetrics fm(ft);
        int minWidth = fm.width(text());

        if(!m_optionDialog->isHidden()){
            QMatrix leftmatrix;
            leftmatrix.rotate(180);//旋转的角度
            pic = pic.transformed(leftmatrix,Qt::SmoothTransformation);
        }
        painter->drawPixmap(QPoint(m_startPoint.x() + minWidth + m_spacing, picY), pic);
    }
}

/**
 * @brief 绘制文本
 * @param painter
 * @param
 */
void MyPushButton::drawText(QPainter *painter,QString colorName)
{
    if (!text().isEmpty()) {
        const QFont & ft = font();

        QFontMetrics fm(ft);
        int minWidth = fm.width(text());
        int minHeight = fm.height();

        int txtY = (rect().height() - minHeight) / 2;
        QRect rect(m_startPoint.x(), txtY, minWidth, minHeight);

        painter->setFont(font());

        if(!colorName.isEmpty())
            painter->setPen(QColor(colorName));

        painter->drawText(rect, Qt::AlignLeft, text());

    }
}

/**
 * @brief 获取打开状态背景图片
 * @param openImgSrc打开背景图
 * @param openIconSrc打开图标
 */
void MyPushButton::setOpenStateImg()
{
    QString openImgSrc = m_customBtnInfo.m_triggerBtnOpen.m_buttonInfo.m_srcImg;
    QString openIconSrc = m_customBtnInfo.m_triggerBtnOpen.m_iconInfo.m_srcImg;
    if(!openImgSrc.isEmpty())
        openImgSrc = ":/" + openImgSrc;
    if(!openIconSrc.isEmpty())
        openIconSrc = ":/" + openIconSrc;

    updateStateImage(openImgSrc,openIconSrc,m_openBgImg);

    m_currentBgImg = m_openBgImg;

}

/**
 * @brief 获取关闭状态背景图片
 * @param closeImgSrc关闭背景图片
 * @param closeIconSrc关闭图标
 */
void MyPushButton::setCloseStateImg()
{
    QString closeImgSrc = m_customBtnInfo.m_triggerBtnClose.m_buttonInfo.m_srcImg;
    QString closeIconSrc = m_customBtnInfo.m_triggerBtnClose.m_iconInfo.m_srcImg;
    if(!closeImgSrc.isEmpty())
        closeImgSrc = ":/" + closeImgSrc;
    if(!closeIconSrc.isEmpty())
        closeIconSrc = ":/" + closeIconSrc;

    updateStateImage(closeImgSrc,closeIconSrc,m_closeBgImg);
}

/**
 * @brief 获取关闭状态样式
 * @param closeSytle关闭样式
 */
void MyPushButton::setCloseStateStytle()
{
     m_closeFontStyle.m_default = m_customBtnInfo.m_triggerBtnClose.m_buttonInfo.m_textInfo.m_color;

     m_closeBgStyle.m_default = getBackgroundStyle(m_customBtnInfo.m_triggerBtnClose.m_buttonInfo.m_style);

     //当其他状态没为空时使用默认样式
     m_closeFontStyle.m_hover = firltrateStyle(m_closeFontStyle.m_hover,m_closeFontStyle.m_hover);
     m_closeFontStyle.m_pressed = firltrateStyle(m_closeFontStyle.m_pressed,m_closeFontStyle.m_hover);
     m_closeFontStyle.m_checked = firltrateStyle(m_closeFontStyle.m_checked,m_closeFontStyle.m_hover);

     m_closeBgStyle.m_hover = firltrateStyle(m_closeBgStyle.m_hover,m_closeBgStyle.m_hover);
     m_closeBgStyle.m_pressed = firltrateStyle(m_closeBgStyle.m_pressed,m_closeBgStyle.m_hover);
     m_closeBgStyle.m_checked = firltrateStyle(m_closeBgStyle.m_checked,m_closeBgStyle.m_hover);

}

/**
 * @brief 获取按钮打开状态样式
 * @param openStyleColors样式map
 */
void MyPushButton::setOpenStateStytle()
{

     m_openFontStyle.m_default = m_customBtnInfo.m_triggerBtnOpen.m_buttonInfo.m_textInfo.m_color;

     m_openBgStyle.m_default = getBackgroundStyle(m_customBtnInfo.m_triggerBtnOpen.m_buttonInfo.m_style);

     //当其他状态没为空时使用默认样式
     m_openFontStyle.m_hover = firltrateStyle(m_openFontStyle.m_hover,m_openFontStyle.m_hover);
     m_openFontStyle.m_pressed = firltrateStyle(m_openFontStyle.m_pressed,m_openFontStyle.m_hover);
     m_openFontStyle.m_checked = firltrateStyle(m_openFontStyle.m_checked,m_openFontStyle.m_hover);

     m_openBgStyle.m_hover = firltrateStyle(m_openBgStyle.m_hover,m_openBgStyle.m_hover);
     m_openBgStyle.m_pressed = firltrateStyle(m_openBgStyle.m_pressed,m_openBgStyle.m_hover);
     m_openBgStyle.m_checked = firltrateStyle(m_openBgStyle.m_checked,m_openBgStyle.m_hover);


     m_currentBgStyle = m_openBgStyle;
     m_currentFontStyle = m_openFontStyle;

}

QString MyPushButton::firltrateStyle(QString currentStyle,QString defauleStyle)
{
    return (currentStyle.isEmpty() ? defauleStyle : currentStyle);
}

/**
 * @brief 处理按钮背景样式
 */
QString MyPushButton::getBackgroundStyle(QString srcStyle)
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

void MyPushButton::updateStateImage(QString srcCurrentImg,QString srcCurentIcon,BackgroundImg &backgroundImg)
{
    QStringList t_imglist = srcCurrentImg.split(".");

    QStringList t_iconlist = srcCurentIcon.split(".");

    backgroundImg.m_normalPix.load(srcCurrentImg);
    backgroundImg.m_normalIcon.load(srcCurentIcon);

    if(t_imglist.size() > 1){

        backgroundImg.m_hoverPix.load(t_imglist.at(0) + "_mouseOver." + t_imglist.at(1));
        if(backgroundImg.m_hoverPix.isNull())
            backgroundImg.m_hoverPix = backgroundImg.m_normalPix;

        backgroundImg.m_pressedPix.load(t_imglist.at(0) + "_mouseDown." + t_imglist.at(1));
        if(backgroundImg.m_pressedPix.isNull())
            backgroundImg.m_pressedPix = backgroundImg.m_normalPix;

        backgroundImg.m_selectedPix.load(t_imglist.at(0) + "_selected." + t_imglist.at(1));
        if(backgroundImg.m_selectedPix.isNull())
            backgroundImg.m_selectedPix = backgroundImg.m_normalPix;

    }else{
        backgroundImg.m_hoverPix = backgroundImg.m_pressedPix = backgroundImg.m_selectedPix = backgroundImg.m_normalPix;
    }

    if(t_iconlist.size() > 1){

        backgroundImg.m_hoverIcon.load(t_iconlist.at(0) + "_mouseOver." + t_iconlist.at(1));
        if(backgroundImg.m_hoverIcon.isNull())
            backgroundImg.m_hoverIcon = backgroundImg.m_normalIcon;

        backgroundImg.m_pressIcon.load(t_iconlist.at(0) + "_mouseDown." + t_iconlist.at(1));
        if(backgroundImg.m_pressIcon.isNull())
            backgroundImg.m_pressIcon = backgroundImg.m_normalIcon;

        backgroundImg.m_selectIcon.load(t_iconlist.at(0) + "_selected." + t_iconlist.at(1));
        if(backgroundImg.m_selectIcon.isNull())
            backgroundImg.m_selectIcon = backgroundImg.m_normalIcon;

    }else{
        backgroundImg.m_hoverIcon = backgroundImg.m_pressIcon = backgroundImg.m_selectIcon = backgroundImg.m_normalIcon;
    }

}

void MyPushButton::updateCurrentStyle(bool isOpen)
{
    if(isOpen){
       m_currentBgStyle = m_openBgStyle;
       m_currentFontStyle = m_openFontStyle;
       m_currentBgImg = m_openBgImg;
    }else{
        m_currentBgStyle = m_closeBgStyle;
        m_currentFontStyle = m_closeFontStyle;
        m_currentBgImg = m_closeBgImg;
    }
}

void MyPushButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event)

//    SetBtnState(ControlState::HOVER);


    update();
}

void MyPushButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)

    m_curState = NORMAL;

    update();
}

/*!
     * @brief 设置ICON类型
     * @details 若为自定义类型,还需要提供ICON的尺寸
     */
void MyPushButton::setIconSize(MyPushButton::IconSize type, QSize size)
{
    switch (type) {
    case ICON_16: m_iconSize = QSize(16, 16); break;
    case ICON_24: m_iconSize = QSize(24, 24); break;
    case ICON_32: m_iconSize = QSize(32, 32); break;
    case ICON_48: m_iconSize = QSize(48, 48); break;
    case ICON_64: m_iconSize = QSize(64, 64); break;
    case ICON_custom: {
        if (size.isValid())
            m_iconSize = size;
        else
            m_iconSize = QSize(16, 16);
    }
        break;
    default:break;
    }
}

void MyPushButton::SetBtnState(ControlState state)
{
    m_curState = state;
}

QSize MyPushButton::sizeHint() const
{
    return calcMiniumSize();
}

QSize MyPushButton::minimumSizeHint() const
{
    return calcMiniumSize();
}

QSize MyPushButton::calcMiniumSize() const
{
    QMargins margins = contentsMargins();

    QPoint startPoint(margins.left(), margins.top());
    QIcon qic = icon();

    int m_maxY = 0;

    if (!qic.isNull()) {
        m_maxY = m_iconSize.height();
        startPoint += QPoint(m_iconSize.width(), 0);
    }

    if (!qic.isNull() && !text().isEmpty()) {
        startPoint.setX(startPoint.x() + m_spacing);
    }

    if (!text().isEmpty()) {
        const QFont & ft = font();

        QFontMetrics fm(ft);

        if (fm.height() > m_maxY)
            m_maxY = fm.height();

        startPoint += QPoint(fm.width(text()), 0);
    }

    return QSize(startPoint.x() + margins.right(), m_maxY + margins.top() + margins.bottom());
}


void MyPushButton::changeText(QString text)
{
    if(!text.isEmpty())
        setText(text);

    if(text.contains(QString::fromLocal8Bit("关闭")))
        isBtnOpenState = false;
    else
        isBtnOpenState = true;
    updateCurrentStyle(isBtnOpenState);
}

void MyPushButton::setSelfStyleSheet(DropDownButtonData customBtnInfo)
{
    m_customBtnInfo = customBtnInfo;

    setCloseStateStytle();
    setOpenStateStytle();

    setOpenStateImg();
    setCloseStateImg();

    m_optionDialog->setOptionsBoxInfo(m_customBtnInfo.m_optionPopUp);
}
