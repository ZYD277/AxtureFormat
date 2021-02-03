#ifndef DROPDOWNBOX_H
#define DROPDOWNBOX_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QSize>
#include <QPushButton>
#include <QRect>
#include <QEvent>

#include "head.h"

class DropDownBox : public QWidget
{
    Q_OBJECT
public:
    explicit DropDownBox(QWidget *parent = 0);

    void showPopupBox();
    void hidePopupBox();

    void attachTarget(QWidget *targetWidget);

    void getPopupBoxInfo(LabelPopupWindow popupBoxInfo);

signals:

public slots:
    void setThisFocus();

protected:
    void enterEvent(QEvent *event);

private:

    QRect getControlRect(Location location);

private:
    //绑定下拉框主体，用于计算弹出位置
    QWidget *m_targetWidget;
    //弹出选项框
    QPropertyAnimation *animation;

    QSize m_mainWidgetSize;

    LabelPopupWindow m_popupBoxInfo;   /*!< 弹窗信息 */

    QWidget *m_mainWidget;
};

#endif // DROPDOWNBOX_H
