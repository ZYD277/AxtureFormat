#include "myswitchbutton.h"

#include <QVBoxLayout>
#include <QDebug>

MySwitchButton::MySwitchButton(QWidget *parent) : QWidget(parent)
{

}

MySwitchButton::~MySwitchButton()
{
    for(int i = 0; i < m_lineEditList.size(); i++){
        QLineEdit *lineEdit =  m_lineEditList.at(i);
        delete lineEdit;
    }
    m_lineEditList.clear();
}

void MySwitchButton::setSelfStyleSheet(customSwitchButton customSwitchBtn)
{
    m_customSwitchBtn = customSwitchBtn;

    mainWidget = new QWidget();

    initView();

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(mainWidget);
    setLayout(mainLayout);
}

void MySwitchButton::initView()
{
    /**
     * @brief 初始化按钮参数
     */
    m_switchBtn = new QPushButton(mainWidget);

    m_switchBtn->setCheckable(true);

    m_switchBtn->setObjectName(m_customSwitchBtn.m_closeState.m_ID);
    m_switchBtn->setGeometry(getLocationInfo(m_customSwitchBtn.m_closeState.m_location));
    m_switchBtn->setText(m_customSwitchBtn.m_closeState.m_textInfo.m_text);

    m_switchBtn->setStyleSheet(QString("QPushButton {border-image: url(:/%1);}"
                                       "QPushButton:checked {border-image: url(:/%2);}").arg(m_customSwitchBtn.m_closeState.m_srcImg).
                               arg(m_customSwitchBtn.m_openState.m_srcImg));

    connect(m_switchBtn,SIGNAL(toggled(bool)),this,SLOT(setLineEditState(bool)));

    /*!< 初始化文本框 */
    for(int i = 0; i < m_customSwitchBtn.m_textParagraphs.size(); i++){
        BaseInfo t_baseIfo = m_customSwitchBtn.m_textParagraphs.at(i);
        QLabel *t_label = new QLabel(mainWidget);

        t_label->setObjectName(t_baseIfo.m_ID);
        t_label->setGeometry(getLocationInfo(t_baseIfo.m_location));
        t_label->setText(t_baseIfo.m_textInfo.m_text);

        if(!t_baseIfo.m_srcImg.isEmpty()){

          t_label->setStyleSheet(QString("#%1 {border-image: url(:/%2);}").arg(t_baseIfo.m_ID).
                                         arg(t_baseIfo.m_srcImg));
        }
    }

    /*!< 初始化行编辑框 */
    m_lineEditList.clear();

    for(int i = 0; i < m_customSwitchBtn.m_lineEdits.size(); i++){
        BaseInfo t_baseIfo = m_customSwitchBtn.m_lineEdits.at(i);
        QLineEdit *t_lineEdit = new QLineEdit(mainWidget);

        t_lineEdit->setGeometry(getLocationInfo(t_baseIfo.m_location));
        t_lineEdit->setObjectName(t_baseIfo.m_ID);

        t_lineEdit->setPlaceholderText(QString::fromLocal8Bit("请输入"));

        if(!t_baseIfo.m_srcImg.isEmpty()){

          t_lineEdit->setStyleSheet(QString("#%1 {border-image: url(:/%2);}").arg(t_baseIfo.m_ID).
                                         arg(t_baseIfo.m_srcImg));
        }

        m_lineEditList.append(t_lineEdit);
    }
}

QRect MySwitchButton::getLocationInfo(Location curLocation)
{
    return QRect(curLocation.m_left,curLocation.m_top,curLocation.m_width,curLocation.m_height);
}

/**
 * @brief 填充控件背景图片
 * @param controlID控件ID
 * @param srcImg背景图Name
 */
void MySwitchButton::setSrcImg(QString controlID,QString srcImg)
{
    if(!srcImg.isEmpty())
        setStyleSheet(QString("#%1 {border-image: url(:/%2);}").arg(controlID).
                                arg(srcImg));
}

/**
 * @brief 设置当前编辑框的状态
 * @param checked
 */
void MySwitchButton::setLineEditState(bool checked)
{
    for(int i = 0; i < m_lineEditList.size(); i++){
        QLineEdit *lineEdit = m_lineEditList.at(i);
        if(checked){
            if(lineEdit->text().isEmpty()){
                lineEdit->setText(QString::fromLocal8Bit("无"));
                lineEdit->setReadOnly(true);
            }
        }else{
            if(lineEdit->text() == QString::fromLocal8Bit("无")){
                lineEdit->setText(NULL);
                lineEdit->setReadOnly(false);
            }
        }
    }
}
