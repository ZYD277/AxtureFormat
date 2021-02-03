#include "dropdownbox.h"

#include <QVBoxLayout>
#include <QDebug>

DropDownBox::DropDownBox(QWidget *parent) : QWidget(parent)
  ,m_mainWidgetSize(100,200)
{
    setWindowFlags(Qt::Popup|Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_WindowPropagation);

    animation = new QPropertyAnimation(this,"size",this);

    connect(animation,SIGNAL(finished()),this,SLOT(setThisFocus()));

}

void DropDownBox::setThisFocus()
{
    this->setFocus(); //弹出后获得焦点

}

/**
 * @brief 提取弹窗信息
 * @param popupBoxInfo弹窗信息
 */
void DropDownBox::getPopupBoxInfo(LabelPopupWindow popupBoxInfo)
{
    m_popupBoxInfo = popupBoxInfo;

    m_mainWidget = new QWidget();
    m_mainWidget->setObjectName(m_popupBoxInfo.m_ID);

    m_mainWidgetSize.setWidth(m_popupBoxInfo.m_location.m_width);
    m_mainWidgetSize.setHeight(m_popupBoxInfo.m_location.m_height);

    QVBoxLayout *buttonLayout = new QVBoxLayout();

    for(int i = 0; i < m_popupBoxInfo.m_optionsInfo.size(); i++){

        BaseInfo t_baseInfo = m_popupBoxInfo.m_optionsInfo.at(i);

        QPushButton *button = new QPushButton(m_mainWidget);
        button->setGeometry(getControlRect(t_baseInfo.m_location));
        button->setObjectName(t_baseInfo.m_ID);
        button->setText(t_baseInfo.m_textInfo.m_text);

        connect(button,SIGNAL(pressed()),this,SLOT(hide()));

        buttonLayout->addWidget(button);
    }

    m_mainWidget->setLayout(buttonLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(m_mainWidget);

    setLayout(mainLayout);
}

QRect DropDownBox::getControlRect(Location location)
{
    return QRect(location.m_left,location.m_top - m_popupBoxInfo.m_location.m_top,location.m_width,location.m_height);
}

void DropDownBox::attachTarget(QWidget *targetWidget)
{
    m_targetWidget = targetWidget;
}

void DropDownBox::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    update();
}

void DropDownBox::showPopupBox()
{
    if(animation->state()==QAbstractAnimation::Running)
        return;
    if(m_targetWidget){
        //设置目标widget的宽度
        this->resize(m_mainWidgetSize.width(),1);
        //先show再move位置才是正确的
        show();
        //show之后取到的大小才是计算好的
        resize(m_mainWidgetSize.width(),m_mainWidgetSize.height());
        //复位
        move(0,0);
        //移动到目标widget上方或下方
        move(m_targetWidget->mapToGlobal(m_targetWidget->rect().bottomLeft()));
    }
    //弹出如果从0开始，点击窗口标题栏不会隐藏
    animation->setStartValue(QSize(m_mainWidgetSize.width(),1));
    animation->setEndValue(QSize(m_mainWidgetSize.width(),height()));
    animation->setDuration(250);

    animation->start();
}

void DropDownBox::hidePopupBox()
{
    hide();
}
