#include "mylineedit.h"

#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QDir>
#include <QStyle>

MyLineEdit::MyLineEdit(QWidget *parent) : QLineEdit(parent),m_popupWindowType(M_LINEEDIT)
{
    initView();

    setPlaceholderText(QString::fromLocal8Bit("请输入"));
}

void MyLineEdit::initView()
{
    m_keyboardBox = new OptionsBox();
    m_keyboardBox->attachTarget(this);
    m_keyboardBox->setTitletype(m_popupWindowType);

    connect(m_keyboardBox,SIGNAL(setCurrentText(QString)),this,SLOT(changeText(QString)));
    connect(this,SIGNAL(textChanged(QString)),m_keyboardBox,SLOT(getEditBoxText(QString)));
}

void MyLineEdit::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton){
      if(m_keyboardBox->isHidden())
          m_keyboardBox->showPopup();
      else
          m_keyboardBox->hidePopup();
    }
    update();
}

void MyLineEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
}

void MyLineEdit::enterEvent(QEvent * event)
{
    Q_UNUSED(event)

    update();
}
void MyLineEdit::leaveEvent(QEvent * event)
{
    Q_UNUSED(event)

    update();
}

void MyLineEdit::changeText(QString text)
{
    if(!text.isEmpty())
        setText(text);
}

void MyLineEdit::setControlStyle(ControlStyles controlStyles)
{
    m_keyboardBox->setControlStyle(controlStyles.m_virtualKeyGroup);
    setObjectName(controlStyles.m_triggerInputBox.m_baseAtrribute.m_ID);
}
