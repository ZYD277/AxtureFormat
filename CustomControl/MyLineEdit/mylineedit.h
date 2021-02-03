#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QWidget>
#include <QLineEdit>

#include "optionsbox.h"
#include "head.h"

class MyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEdit(QWidget *parent = 0);

    void setControlStyle(ControlStyles controlStyles);

signals:

public slots:
    void changeText(QString text);

private:
    void initView();

protected:
//    void paintEvent(QPaintEvent * event);
    void enterEvent(QEvent * event);
    void leaveEvent(QEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void focusOutEvent(QFocusEvent *event);

private:
    OptionsBox *m_keyboardBox;    /*!< 虚拟键盘框 */
    PopupWindowType m_popupWindowType;  /*!< 弹框类型 */
};

#endif // MYLINEEDIT_H
