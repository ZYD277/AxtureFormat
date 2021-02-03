#ifndef MYLABEL_H
#define MYLABEL_H

#include <QWidget>
#include <QLabel>
#include <QEvent>

#include "head.h"

#include "dropdownbox.h"

class MyLabel : public QLabel
{
public:
    MyLabel(QWidget *parent);

    void initCustomLabel(CustomLabelData customLabelInfo);

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

private:
    CustomLabelData m_customLabelInfo;

    DropDownBox *m_dropDownBox;
};

#endif // MYLABEL_H
