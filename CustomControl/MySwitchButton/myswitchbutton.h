#ifndef MYSWITCHBUTTON_H
#define MYSWITCHBUTTON_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

#include "head.h"

class MySwitchButton : public QWidget
{
    Q_OBJECT
public:
    explicit MySwitchButton(QWidget *parent = 0);
    ~MySwitchButton();

    void setSelfStyleSheet(customSwitchButton customSwitchBtn);

signals:

public slots:
    void setLineEditState(bool checked);

private:

    void initView();

    QRect getLocationInfo(Location curLocation);

    void setSrcImg(QString controlID,QString srcImg);

private:
    customSwitchButton m_customSwitchBtn;

    QWidget *mainWidget;

    QPushButton *m_switchBtn;

    QList<QLineEdit*> m_lineEditList;
};

#endif // MYSWITCHBUTTON_H
