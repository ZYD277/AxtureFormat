#ifndef MYFOLDINGCONTROL_H
#define MYFOLDINGCONTROL_H

#include <QWidget>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>

#include "head.h"

class MyFoldingControl : public QFrame
{
    Q_OBJECT
public:
    explicit MyFoldingControl(QWidget *parent = 0);

    void setSelfStyleSheet(FoldingControls foldingControl);

signals:

public slots:

private:
    void initView();

    int setMainWidgetHight();

    QRect getLocation(Location location);

private:
    FoldingControls m_foldingControl;

    QWidget *m_mainWidget;

    QWidget *m_foldingWidget;

    QWidget *m_unFoldWidget;

    QScrollArea *m_scrollArea;
};

#endif // MYFOLDINGCONTROL_H
