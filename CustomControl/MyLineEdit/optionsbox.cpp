#include "optionsbox.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QButtonGroup>
#include <QDebug>
#include <QCoreApplication>

#define buttPropertyName "buttnText"

OptionsBox::OptionsBox(QWidget *parent) : QWidget(parent),targetWidget(NULL)
{
    initView();

    setWindowFlags(Qt::Popup|Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_WindowPropagation);

    animation = new QPropertyAnimation(this,"size",this);

    connect(animation,SIGNAL(finished()),this,SLOT(setThisFocus()));
}

OptionsBox::~OptionsBox()
{
    delete m_mainWidget;
}

void OptionsBox::setThisFocus()
{
    this->setFocus(); //弹出后获得焦点

}

void OptionsBox::initView()
{
    m_mainWidget = new QWidget();
    m_functionBtnWidget = new QWidget(m_mainWidget);

    QHBoxLayout * mainlayout = new QHBoxLayout;
    mainlayout->setContentsMargins(0,0,0,0);
    mainlayout->addWidget(m_mainWidget);
    setLayout(mainlayout);
}

void OptionsBox::showPopup()
{
    if(animation->state()==QAbstractAnimation::Running)
        return;
    if(targetWidget){
        //设置目标widget的宽度
        this->resize(m_boxMaxwidth,1);
        //先show再move位置才是正确的
        show();
        //show之后取到的大小才是计算好的
        int view_height = m_boxMaxHeith;

        resize(m_boxMaxwidth,view_height>100?view_height:100);
        //复位
        move(0,0);
        //移动到目标widget上方或下方
        move(targetWidget->mapToGlobal(targetWidget->rect().bottomLeft()));
    }
    //弹出如果从0开始，点击窗口标题栏不会隐藏
    animation->setStartValue(QSize(m_boxMaxwidth,1));
    animation->setEndValue(QSize(m_boxMaxwidth,height()));
    animation->setDuration(250);

    animation->start();
}

/**
 * @brief 获取主题控件位置用于计算弹窗弹出位置
 * @param widget
 */
void OptionsBox::attachTarget(QWidget *widget)
{
    targetWidget = widget;
}

/**
 * @brief 标题为输入框时随时获取触发编辑框的文本
 * @param editBoxText触发编辑框文本
 */
void OptionsBox::getEditBoxText(QString editBoxText)
{
    m_editBoxText = editBoxText;
}

void OptionsBox::setTitletype(PopupWindowType type)
{
    m_titleType = type;
}

void OptionsBox::hidePopup()
{
    hide();
}

void OptionsBox::setOptionsBoxInfo(OptionPopUp optionsBoxInfo)
{
    m_optionsBoxInfo = optionsBoxInfo;

    m_boxMaxwidth = optionsBoxInfo.m_location.m_width;
    m_boxMaxHeith = optionsBoxInfo.m_location.m_height;

    //初始化弹窗
    m_mainWidget->setObjectName(m_optionsBoxInfo.m_ID);
    m_optionsWidget = new QWidget(m_mainWidget);
    m_optionsWidget->setObjectName(m_optionsBoxInfo.m_border.m_baseInfo.m_ID);
    m_optionsWidget->setGeometry(getControlLocation(m_optionsBoxInfo.m_location,m_optionsBoxInfo.m_border.m_baseInfo.m_location));


    //标题

    setTitleInfo(m_optionsWidget,m_optionsBoxInfo.m_titleInfo,m_optionsBoxInfo.m_border.m_baseInfo.m_location);

    //选项按钮组
    QWidget *optionsBtnGroup = new QWidget(m_optionsWidget);
    optionsBtnGroup->setObjectName(m_optionsBoxInfo.m_optionBtnGroup.m_ID);
    optionsBtnGroup->setGeometry(getControlLocation(m_optionsBoxInfo.m_border.m_baseInfo.m_location,m_optionsBoxInfo.m_optionBtnGroup.m_location));

    createBtnGroup(optionsBtnGroup,m_optionsBoxInfo.m_optionBtnGroup,true);

    //退出按钮组

    QWidget *exitBtnGroup = new QWidget(m_mainWidget);
    exitBtnGroup->setObjectName(m_optionsBoxInfo.m_exitBtnGroup.m_ID);
    exitBtnGroup->setGeometry(getControlLocation(m_optionsBoxInfo.m_location,m_optionsBoxInfo.m_exitBtnGroup.m_location));

    createBtnGroup(exitBtnGroup,m_optionsBoxInfo.m_exitBtnGroup,false);
}

/**
 * @brief 填充弹窗标题
 * @param titleText文本信息
 * @param titleID标题objName
 */
void OptionsBox::setTitleInfo(QWidget *parentWidget,TitleInfo titleInfo,Location pLocation)
{
    //标题填充
    switch (m_titleType) {
    case M_LABEL:
    {
        QLabel *titleLabel = new QLabel(parentWidget);
        titleLabel->setObjectName(titleInfo.m_baseInfo.m_ID);
        titleLabel->setText(titleInfo.m_baseInfo.m_textInfo.m_text);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setGeometry(getControlLocation(pLocation,titleInfo.m_baseInfo.m_location));
    }
        break;
    case M_LINEEDIT:
    {

    }
        break;
    default:
        break;
    }

}

/**
 * @brief 创建按钮组
 * @param parentWidget父在窗口
 * @param btnGroupInfo按钮组信息
 * @param createBtnChecked是否设置可选中
 */
void OptionsBox::createBtnGroup(QWidget *parentWidget,ButtonGroup btnGroupInfo,bool createBtnChecked)
{
    QButtonGroup * optionBtnsGroup = new QButtonGroup;
    optionBtnsGroup->setExclusive(true);

    for(int i = 0; i < btnGroupInfo.m_buttons.size(); i++){
        ButtonInfo t_btnInfo = btnGroupInfo.m_buttons.at(i);
        QPushButton * butt = new QPushButton(parentWidget);
        butt->setGeometry(getControlLocation(btnGroupInfo.m_location,t_btnInfo.m_baseInfo.m_location));
        butt->setObjectName(t_btnInfo.m_baseInfo.m_ID);
        butt->setText(t_btnInfo.m_baseInfo.m_textInfo.m_text);

        if(createBtnChecked){
            butt->setCheckable(true);
            optionBtnsGroup->addButton(butt);
        }

        if(t_btnInfo.deleteBtn)
            butt->setProperty(buttPropertyName,QString("delete"));
        else
            butt->setProperty(buttPropertyName,t_btnInfo.m_baseInfo.m_textInfo.m_text);

        connect(butt,SIGNAL(pressed()),this,SLOT(buttonText()));

        if(t_btnInfo.m_baseInfo.m_srcImg.contains("images")){

            butt->setStyleSheet(setControlStyleSheet(t_btnInfo.m_baseInfo.m_srcImg));
        }
    }
}

void OptionsBox::buttonText()
{
    QString t_buttonText = QObject::sender()->property(buttPropertyName).toString();

    if(t_buttonText.contains(QString::fromLocal8Bit("确定")) || t_buttonText.contains(QString::fromLocal8Bit("取消")))
        changeCurrentText(t_buttonText);
    else {
        switch (m_titleType) {
        case M_LABEL:
        {
            m_checkedText = t_buttonText;
        }
            break;
        case M_LINEEDIT:
        {
            if(t_buttonText.contains(QString("delete"))){
                m_titleLine->setText(m_titleLine->text().left(m_titleLine->text().size() - 1));
            }else
                m_titleLine->setText(m_titleLine->text() +t_buttonText);
        }
            break;
        default:
            break;
        }

    }
}

void OptionsBox::changeCurrentText(QString btnText)
{
    switch (m_titleType) {
    case M_LABEL:
    {
        if(btnText.contains(QString::fromLocal8Bit("确定"))){
            emit setCurrentText(m_checkedText);
            hidePopup();
        }else if(btnText.contains(QString::fromLocal8Bit("取消"))){
            hidePopup();
        }
    }
        break;
    case M_LINEEDIT:
    {
        if(btnText.contains(QString::fromLocal8Bit("确定"))){
            emit setCurrentText(m_titleLine->text());
            hidePopup();
        }else if(btnText.contains(QString::fromLocal8Bit("取消"))){
            m_titleLine->setText(m_editBoxText);
            hidePopup();
        }
    }
        break;
    default:
        break;
    }
}

/**
 * @brief 获取控件真实位置
 * @param plocation父窗口位置信息
 * @param slocation当前窗口位置信息
 * @return返回一个QRect
 */
QRect OptionsBox::getControlLocation(Location plocation,Location slocation)
{
    return QRect(slocation.m_left - plocation.m_left,slocation.m_top - plocation.m_top,slocation.m_width,slocation.m_height);
}

/**
 * @brief 设置背景图片
 * @param srcImage 原始背景图片
 * @return返回处理后的样式
 */
QString OptionsBox::setControlStyleSheet(QString srcImage)
{
    QStringList srcList = srcImage.split(".");
    if(srcList.size() > 1){
        QString t_pressStyle = srcList.at(0) + "_mouseDown." + srcList.at(1);
        QString t_hoverStyle = srcList.at(0) + "_mouseOver." + srcList.at(1);

        srcImage = QString(""
                           "QPushButton {border-image: url(:/%1);}"
                           "QPushButton:hover {border-image: url(:/%2);}"
                           "QPushButton:pressed  {border-image: url(:/%3);}"
                           "").arg(srcImage).arg(t_hoverStyle).arg(t_pressStyle);
    }
    return srcImage;
}

/*************************************************************************************/
/*****************************自制输入框带有虚拟键盘的输入框***********************************/
/************************************************************************************/
/**
 * @brief 设置键盘区样式
 * @param keyBoardStyle样式
 */
void OptionsBox::setControlStyle(VirtualKeyboard keyBoardStyle)
{
    m_keyBoardStyle = keyBoardStyle;
    m_boxMaxwidth = m_keyBoardStyle.m_location.m_width;
    m_boxMaxHeith = m_keyBoardStyle.m_location.m_height;

    m_mainWidget->setObjectName(m_keyBoardStyle.m_ID);

    m_optionsWidget = new QWidget(m_mainWidget);
    m_optionsWidget->setObjectName(m_keyBoardStyle.m_baseAtrribute.m_ID);
    m_optionsWidget->setGeometry(getControlLocation(m_keyBoardStyle.m_location,m_keyBoardStyle.m_baseAtrribute.m_location));

//    auto setBtnGroup = [&](QList<BUTTON> buttons,QWidget *parentWidget,Location pLocation){

//    };
    setBtnGroup(m_keyBoardStyle.m_keyGroup.m_keyBtns,m_optionsWidget,m_keyBoardStyle.m_baseAtrribute.m_location);

    m_titleLine = new QLineEdit(m_mainWidget);
    m_titleLine->setObjectName(m_keyBoardStyle.m_title.m_baseAtrribute.m_ID);
    m_titleLine->setText(m_keyBoardStyle.m_title.m_baseAtrribute.m_text);
    m_titleLine->setGeometry(getControlLocation(m_keyBoardStyle.m_location,m_keyBoardStyle.m_title.m_location));

    if(!m_keyBoardStyle.m_title.m_baseAtrribute.m_srcImage.isEmpty()){
        setSrcImg(m_keyBoardStyle.m_title.m_baseAtrribute.m_ID,m_keyBoardStyle.m_title.m_baseAtrribute.m_srcImage);
    }

    m_functionBtnWidget = new QWidget(m_mainWidget);
    m_functionBtnWidget->setObjectName(m_keyBoardStyle.m_tailGroup.m_baseAtrribute.m_ID);
    m_functionBtnWidget->setGeometry(getControlLocation(m_keyBoardStyle.m_location,m_keyBoardStyle.m_tailGroup.m_location));

    setBtnGroup(m_keyBoardStyle.m_tailGroup.m_quitBtns,m_functionBtnWidget,m_keyBoardStyle.m_tailGroup.m_location);


}

void OptionsBox::setBtnGroup(QList<BUTTON> buttons,QWidget *parentWidget,Location pLocation)
{
    for(int i = 0; i < buttons.size(); i++){
        BUTTON t_buttton = buttons.at(i);
        QPushButton * butt = new QPushButton(parentWidget);
        butt->setGeometry(getControlLocation(pLocation,t_buttton.m_location));
        butt->setObjectName(t_buttton.m_baseAtrribute.m_ID);
        butt->setText(t_buttton.m_baseAtrribute.m_text);

        if(t_buttton.deleteBtn)
            butt->setProperty(buttPropertyName,QString("delete"));
        else
            butt->setProperty(buttPropertyName,t_buttton.m_baseAtrribute.m_text);

        connect(butt,SIGNAL(pressed()),this,SLOT(buttonText()));

        if(!t_buttton.m_baseAtrribute.m_iconInfo.m_srcIcon.isEmpty()){
            QIcon btn_icon;
            QSize t_size;
            btn_icon.addFile(":/" + t_buttton.m_baseAtrribute.m_iconInfo.m_srcIcon);
            if(t_buttton.m_baseAtrribute.m_iconInfo.m_location.m_width !=0 &&
                    t_buttton.m_baseAtrribute.m_iconInfo.m_location.m_height != 0){
                t_size = QSize(t_buttton.m_baseAtrribute.m_iconInfo.m_location.m_width,
                               t_buttton.m_baseAtrribute.m_iconInfo.m_location.m_height);
            }else
                t_size = QSize(16,16);

            butt->setIcon(btn_icon);
            butt->setIconSize(t_size);
        }

        if(t_buttton.m_baseAtrribute.m_srcImage.contains("images")){
            butt->setStyleSheet(setControlStyleSheet(t_buttton.m_baseAtrribute.m_srcImage));
        }

    }
}

/**
 * @brief 标题区布局
 */
void OptionsBox::setTitleLaout()
{
    QString titleID = m_keyBoardStyle.m_title.m_baseAtrribute.m_ID;
    QString titleText = m_keyBoardStyle.m_title.m_baseAtrribute.m_text;
    QString srcImag = m_keyBoardStyle.m_title.m_baseAtrribute.m_srcImage;

    QVBoxLayout *titleLaout = new QVBoxLayout();
    m_titleWidget->setLayout(titleLaout);

    switch (m_titleType) {
    case M_LABEL:
    {
        QLabel *titleLable = new QLabel();
        titleLable->setObjectName(titleID);
        titleLable->setText(titleText);
        titleLable->setAlignment(Qt::AlignHCenter);

        setSrcImg(titleID,srcImag);

        titleLaout->addWidget(titleLable);
    }
        break;
    case M_LINEEDIT:
    {
        m_titleLine = new QLineEdit();
        m_titleLine->setPlaceholderText(QString::fromLocal8Bit("请输入"));
        m_titleLine->setObjectName(titleID);
        m_titleLine->setText(titleText);
        m_titleLine->setAlignment(Qt::AlignLeft);

        setSrcImg(titleID,srcImag);//需要根据不同状态处理

        titleLaout->addWidget(m_titleLine);
    }
        break;
    default:
        break;
    }
}

/**
 * @brief 填充控件背景图片
 * @param controlID控件ID
 * @param srcImg背景图Name
 */
void OptionsBox::setSrcImg(QString controlID,QString srcImg)
{
    if(!srcImg.isEmpty())
        setStyleSheet(QString("#%1 {border-image: url(:/%2);}").arg(controlID).
                                arg(srcImg));
}
