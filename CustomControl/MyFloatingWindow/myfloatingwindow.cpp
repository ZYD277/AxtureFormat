#include "myfloatingwindow.h"

#include <QHBoxLayout>
#include <QDebug>

MyFloatingWindow::MyFloatingWindow(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
}

void MyFloatingWindow::setSelfStyleSheet(FloatingWindow floatintWindow)
{
    m_floatintWindow = floatintWindow;

    mainWidget = new QWidget();

    mainWidget->setGeometry(getLocationInfo(m_floatintWindow.m_floatButton.m_location));

    //设置悬浮窗口位置布局以及背景
    initFloatingWindow();

    //布局
    QHBoxLayout *hBoxlayout = new QHBoxLayout();
    hBoxlayout->addWidget(mainWidget);
    hBoxlayout->addWidget(floatingWindow);
    hBoxlayout->setContentsMargins(0,0,0,0);

    setLayout(hBoxlayout);
}

void MyFloatingWindow::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    drawFloatingBtn(&painter);
}

void MyFloatingWindow::drawFloatingBtn(QPainter *painter)
{
    QPixmap pixmap;


    if(floatingWindow->isHidden()){
        int left = m_floatintWindow.m_location.m_width - m_floatintWindow.m_floatButton.m_location.m_width;
        m_floatingBtnRect = QRect(left,m_floatintWindow.m_floatButton.m_location.m_top,m_floatintWindow.m_floatButton.m_location.m_width,m_floatintWindow.m_floatButton.m_location.m_height);

    }
    else
        m_floatingBtnRect = getLocationInfo(m_floatintWindow.m_floatButton.m_location);

    for(int i = 0; i < m_floatintWindow.m_floatButton.m_srcImgs.size(); i ++){

        BaseInfo t_imgInfo  = m_floatintWindow.m_floatButton.m_srcImgs.at(i);

        if(t_imgInfo.m_srcImg.contains("images")){
            QRect rect = QRect(t_imgInfo.m_location.m_left + m_floatingBtnRect.left(),t_imgInfo.m_location.m_top + m_floatingBtnRect.top(),t_imgInfo.m_location.m_width,t_imgInfo.m_location.m_height);

            pixmap.load(":/" + t_imgInfo.m_srcImg);
            painter->drawPixmap(rect,pixmap);
        }
    }
}

void MyFloatingWindow::mousePressEvent(QMouseEvent *event)
{
    if(m_floatingBtnRect.contains(event->pos())){
        if(floatingWindow->isHidden())
        {
            floatingWindow->show();
        }else{
            floatingWindow->hide();
        }
    }
}

void MyFloatingWindow::enterEvent(QEvent * event)
{

}

void MyFloatingWindow::initFloatingWindow()
{
    floatingWindow = new QWidget();

    floatingWindow->setObjectName(m_floatintWindow.m_mainWidget.m_id);

    floatingWindow->setGeometry(getLocationInfo(m_floatintWindow.m_mainWidget.m_location));

    floatingWindow->setFixedSize(m_floatintWindow.m_mainWidget.m_location.m_width,m_floatintWindow.m_mainWidget.m_location.m_height);

    if(m_floatintWindow.m_mainWidget.m_srcImg.contains("images")){
        setStyleSheet(QString("#%1 {border-image: url(:/%2);}").arg(m_floatintWindow.m_mainWidget.m_id).
                                arg(m_floatintWindow.m_mainWidget.m_srcImg));
    }

    for(int i = 0; i < m_floatintWindow.m_mainWidget.m_switchButtons.size(); i++){
        SwitchButton t_switchBtn  = m_floatintWindow.m_mainWidget.m_switchButtons.at(i);
        createButt(t_switchBtn);
    }
}

void MyFloatingWindow::createButt(SwitchButton switchButtn)
{
    QPushButton * butt = new QPushButton(floatingWindow);
    butt->setCheckable(true);
    butt->setObjectName(switchButtn.m_openState.m_ID);
    butt->setText(switchButtn.m_openState.m_textInfo.m_text);

    Location t_location = switchButtn.m_openState.m_location;
    QRect buttonRect(switchButtn.m_location.m_left - m_floatintWindow.m_mainWidget.m_location.m_left,switchButtn.m_location.m_top,t_location.m_width,t_location.m_height);

    butt->setGeometry(buttonRect);

    if(switchButtn.m_openState.m_srcImg.contains("images")){
        QStringList srcList = switchButtn.m_openState.m_srcImg.split(".");

        QString t_pressStyle;

        QString t_checkedStyle;

        if(srcList.size() > 1)
            t_pressStyle  = srcList.at(0) + "_mouseDown." + srcList.at(1);


        if(switchButtn.m_closeState.m_srcImg.contains("images"))
            t_checkedStyle  = switchButtn.m_closeState.m_srcImg;


        butt->setStyleSheet(QString(""
                                    "QPushButton {border-image: url(:/%1);}"
                                    "QPushButton:checked {border-image: url(:/%2);}"
                                    "QPushButton:pressed  {border-image: url(:/%3);}"
                                    "").arg(switchButtn.m_openState.m_srcImg).arg(t_checkedStyle).arg(t_pressStyle));
    }
}

QRect MyFloatingWindow::getLocationInfo(Location locationInfo)
{
    return QRect(locationInfo.m_left,locationInfo.m_top,locationInfo.m_width,locationInfo.m_height);
}
