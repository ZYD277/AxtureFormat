#include "mylabel.h"

#include <QDebug>

MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
{
    m_dropDownBox = new DropDownBox();
    m_dropDownBox->attachTarget(this);
}

void MyLabel::initCustomLabel(CustomLabelData customLabelInfo)
{
    m_customLabelInfo = customLabelInfo;

    setObjectName(m_customLabelInfo.m_defaultInfo.m_ID);

    m_dropDownBox->getPopupBoxInfo(m_customLabelInfo.m_labelPopupWindow);
}

void MyLabel::enterEvent(QEvent *event)
{
    Q_UNUSED(event)

    m_dropDownBox->showPopupBox();

    update();

}

void MyLabel::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)

//    m_dropDownBox->hidePopupBox();

    update();

}
