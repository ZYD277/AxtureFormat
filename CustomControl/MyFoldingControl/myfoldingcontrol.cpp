#include "myfoldingcontrol.h"

#include "foldingwidget.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QButtonGroup>

MyFoldingControl::MyFoldingControl(QWidget *parent) : QFrame(parent)
{

}

void MyFoldingControl::setSelfStyleSheet(FoldingControls foldingControl)
{
    m_foldingControl = foldingControl;

    if(m_foldingControl.m_addScrollBar){

        //滚动条
        m_scrollArea = new QScrollArea(this);
        m_scrollArea->setFrameShadow(QFrame::Sunken);

        m_scrollArea->setGeometry(rect());

        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//不显示滚动条

        m_scrollArea->setFrameStyle(QFrame::NoFrame);

        m_scrollArea->setStyleSheet("QScrollArea {background-color:transparent;}");
        m_scrollArea->viewport()->setStyleSheet("background-color:transparent;");


        m_mainWidget = new QWidget(m_scrollArea);

        m_mainWidget->setGeometry(0,0,m_foldingControl.m_location.m_width,setMainWidgetHight());

        m_scrollArea->setWidget(m_mainWidget);
    }else{

        m_mainWidget = new QWidget();

        QVBoxLayout *mainLaout = new QVBoxLayout();
        mainLaout->addWidget(m_mainWidget);
        setLayout(mainLaout);

    }
    initView();

}

int MyFoldingControl::setMainWidgetHight()
{
    int t_height = 0;
    for(int i = 0; i < m_foldingControl.m_informations.size(); i++){
        Information information  = m_foldingControl.m_informations.at(i);
        t_height += information.m_foldingInfo.m_location.m_height + information.m_unFoldInfo.m_location.m_height;
    }

    return t_height > rect().height() ? t_height : rect().height();
}

void MyFoldingControl::initView()
{
    QVBoxLayout *foldingInfoLaout = new QVBoxLayout();

    for(int i = 0; i < m_foldingControl.m_informations.size(); i++){

        Information information  = m_foldingControl.m_informations.at(i);
        //折叠窗口
        FoldingWidget *foldingWidget = new FoldingWidget(m_mainWidget);
        foldingWidget->setGeometry(getLocation(information.m_foldingInfo.m_location));
        foldingWidget->setFixedSize(information.m_foldingInfo.m_location.m_width,information.m_foldingInfo.m_location.m_height);
        foldingWidget->setObjectName(information.m_foldingInfo.m_ID);
        foldingWidget->setDrawData(information.m_foldingInfo.m_information,information.m_foldingInfo.m_location);

        QVBoxLayout *foldingLayout = new QVBoxLayout();
        foldingWidget->setLayout(foldingLayout);

        //展开窗口
        QWidget *unfoldWidget = new QWidget(m_mainWidget);
        unfoldWidget->setGeometry(getLocation(information.m_unFoldInfo.m_parentLocation));
        QVBoxLayout *unfoldLayout = new QVBoxLayout();
        unfoldWidget->setLayout(unfoldLayout);

        if(!information.m_unFoldInfo.m_autoSetControl){

            QButtonGroup * optionBtnsGroup = new QButtonGroup;

            optionBtnsGroup->setExclusive(true);

            for(int i = 0; i < information.m_unFoldInfo.m_information.size(); i++){
                BaseInfo t_baseInfo = information.m_unFoldInfo.m_information.at(i);
                if(t_baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("图标"))){
                    QPushButton *t_button = new QPushButton(unfoldWidget);
                    t_button->setGeometry(getLocation(t_baseInfo.m_location));
                    t_button->setFixedHeight(t_baseInfo.m_location.m_height);
                    t_button->setText(t_baseInfo.m_textInfo.m_text);
                    t_button->setObjectName(t_baseInfo.m_ID);

                    t_button->setCheckable(true);

                    optionBtnsGroup->addButton(t_button);
                    unfoldLayout->addWidget(t_button);
                }
            }
        }else{

            unfoldWidget->setFixedSize(information.m_unFoldInfo.m_parentLocation.m_width,information.m_unFoldInfo.m_parentLocation.m_height);

            QFrame *unfoldInfoBg = new QFrame(unfoldWidget);
            unfoldInfoBg->setGeometry(getLocation(information.m_unFoldInfo.m_location));

            for(int i = 0; i < information.m_unFoldInfo.m_information.size(); i ++){
                BaseInfo t_baseInfo = information.m_unFoldInfo.m_information.at(i);
                if(t_baseInfo.m_dataLabel.contains(QString::fromLocal8Bit("详情背景")))
                    unfoldInfoBg->setObjectName(t_baseInfo.m_ID);
                else {
                        QLabel *unfoldLabel = new QLabel(unfoldInfoBg);
                        unfoldLabel->setObjectName(t_baseInfo.m_ID);
                        unfoldLabel->setText(t_baseInfo.m_textInfo.m_text);
                        unfoldLabel->setFixedSize(t_baseInfo.m_location.m_width,t_baseInfo.m_location.m_height);
                        unfoldLabel->setWordWrap(true);
                }
            }
        }

        unfoldWidget->setHidden(true);

        connect(foldingWidget,SIGNAL(clicKed(bool)),unfoldWidget,SLOT(setHidden(bool)));

        foldingInfoLaout->addWidget(foldingWidget);
        foldingInfoLaout->addWidget(unfoldWidget);
    }
    foldingInfoLaout->addStretch();
    foldingInfoLaout->setMargin(0);
    foldingInfoLaout->setSpacing(0);

    m_mainWidget->setLayout(foldingInfoLaout);
}

QRect MyFoldingControl::getLocation(Location location)
{
    return QRect(location.m_left,location.m_top,location.m_width,location.m_height);
}
