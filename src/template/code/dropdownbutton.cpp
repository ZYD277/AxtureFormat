#include "dropdownbutton.h"

#include "../cppgenerate.h"

namespace CXX{

DropdownButton::DropdownButton():AbstractCodeTemplate(CUSTOM_DROP_BUTTON)
{

}

void DropdownButton::prepareOutput(CppGenerate *generate)
{

    QString t_locationName = QString("t_location_%1").arg(m_sameTypeIndex);
    QString t_baseInfoName = QString("t_baseInfo_%1").arg(m_sameTypeIndex);
    QString t_textInfoName = QString("t_textInfo_%1").arg(m_sameTypeIndex);

    QString t_triggerBtnOpenName = QString("t_triggerBtnOpen_%1").arg(m_sameTypeIndex);
    QString t_triggerBtnCloseName = QString("t_triggerBtnClose_%1").arg(m_sameTypeIndex);
    QString t_optionPopUpName = QString("t_optionPopUp_%1").arg(m_sameTypeIndex);

    QString t_optionBtnGroupName = QString("t_optionBtnGroup_%1").arg(m_sameTypeIndex);
    QString t_exitBtnGroupName = QString("t_exitBtnGroup_%1").arg(m_sameTypeIndex);

    QString t_buttonInfoName= QString("t_buttonInfo_%1").arg(m_sameTypeIndex);

    QString t_dropDownBtnName = QString("t_dropDownBtn_%1").arg(m_sameTypeIndex);

    QString t_baseInfo = QString("         BaseInfo %1;").arg(t_baseInfoName);
    QString t_location = QString("         Location %1;").arg(t_locationName);
    QString t_textInfo = QString("         TextInfo %1;").arg(t_textInfoName);

    QString t_triggerBtnOpen = QString("         OptionTriggerButton %1;").arg(t_triggerBtnOpenName);
    QString t_triggerBtnClose = QString("         OptionTriggerButton %1;").arg(t_triggerBtnCloseName);
    QString t_optionPopUp = QString("         OptionPopUp %1;").arg(t_optionPopUpName);

    QString t_optionBtnGroup = QString("         ButtonGroup %1;").arg(t_optionBtnGroupName);
    QString t_exitBtnGroup = QString("         ButtonGroup %1;").arg(t_exitBtnGroupName);

    QString t_buttonInfo = QString("         ButtonInfo %1;").arg(t_buttonInfoName);

    QString t_dropDownBtn = QString("         DropDownButtonData %1;").arg(t_dropDownBtnName);

    QString customStyle;
    customStyle += ConcatNewLine(t_baseInfo);
    customStyle += ConcatNewLine(t_textInfo);
    customStyle += ConcatNewLine(t_location);

    customStyle += ConcatNewLine(t_triggerBtnOpen);
    customStyle += ConcatNewLine(t_triggerBtnClose);
    customStyle += ConcatNewLine(t_optionPopUp);

    customStyle += ConcatNewLine(t_optionBtnGroup);
    customStyle += ConcatNewLine(t_exitBtnGroup);
    customStyle += ConcatNewLine(t_buttonInfo);

    customStyle += ConcatNewLine(t_dropDownBtn);


    auto setLocation = [&](Location t_location){
        customStyle += ConcatNewLine(QString("         %1.m_width = %2;").arg(t_locationName).arg(t_location.m_width));
        customStyle += ConcatNewLine(QString("         %1.m_height = %2;").arg(t_locationName).arg(t_location.m_height));
        customStyle += ConcatNewLine(QString("         %1.m_top = %2;").arg(t_locationName).arg(t_location.m_top));
        customStyle += ConcatNewLine(QString("         %1.m_left = %2;").arg(t_locationName).arg(t_location.m_left));

    };
    auto setBaseInfo = [&](BaseInfo t_baseInfo){

        //获取单个控件位置信息

        setLocation(t_baseInfo.m_location);
        customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_baseInfoName).arg(t_locationName));

        //获取单个控件文本信息
        customStyle += ConcatNewLine(QString("         %1.m_text = QString::fromLocal8Bit(\"%2\");").arg(t_textInfoName).arg(t_baseInfo.m_textInfo.m_text));
        customStyle += ConcatNewLine(QString("         %1.m_color = QString::fromLocal8Bit(\"%2\");").arg(t_textInfoName).arg(t_baseInfo.m_textInfo.m_color));
        customStyle += ConcatNewLine(QString("         %1.m_font = QString::fromLocal8Bit(\"%2\");").arg(t_textInfoName).arg(t_baseInfo.m_textInfo.m_font));

        customStyle += ConcatNewLine(QString("         %1.m_textInfo = %2;").arg(t_baseInfoName).arg(t_textInfoName));

        //获取背景图片
        customStyle += ConcatNewLine(QString("         %1.m_srcImg = QString::fromLocal8Bit(\"%2\");").arg(t_baseInfoName).arg(t_baseInfo.m_srcImg));
        //获取背景样式
        customStyle += ConcatNewLine(QString("         %1.m_style = QString::fromLocal8Bit(\"%2\");").arg(t_baseInfoName).arg(t_baseInfo.m_style));
        //获取类型标志
        customStyle += ConcatNewLine(QString("         %1.m_className = QString::fromLocal8Bit(\"%2\");").arg(t_baseInfoName).arg(t_baseInfo.m_className));
        //获取数据标签
        customStyle += ConcatNewLine(QString("         %1.m_dataLabel = QString::fromLocal8Bit(\"%2\");").arg(t_baseInfoName).arg(t_baseInfo.m_dataLabel));
        //获取控件ID
        customStyle += ConcatNewLine(QString("         %1.m_ID = QString::fromLocal8Bit(\"%2\");").arg(t_baseInfoName).arg(t_baseInfo.m_ID));

    };

    //获取自定义下拉框的触发弹窗按钮属性
    auto setmTriggerBtnState = [&](OptionTriggerButton triggerBtn,QString triggerBtnName){

        customStyle += ConcatNewLine(QString("         %1.m_ID = QString::fromLocal8Bit(\"%2\");").arg(triggerBtnName).arg(triggerBtn.m_ID));

        setLocation(triggerBtn.m_location);
        customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(triggerBtnName).arg(t_locationName));

        setBaseInfo(triggerBtn.m_iconInfo);
        customStyle += ConcatNewLine(QString("         %1.m_iconInfo = %2;").arg(triggerBtnName).arg(t_baseInfoName));

        setBaseInfo(triggerBtn.m_buttonInfo);
        customStyle += ConcatNewLine(QString("         %1.m_buttonInfo = %2;").arg(triggerBtnName).arg(t_baseInfoName));
    };

    //打开状态
    setmTriggerBtnState(m_customButton->m_triggerBtnOpen,t_triggerBtnOpenName);
    customStyle += ConcatNewLine(QString("         %1.m_triggerBtnOpen = %2;").arg(t_dropDownBtnName).arg(t_triggerBtnOpenName));

    //关闭状态
    setmTriggerBtnState(m_customButton->m_triggerBtnClose,t_triggerBtnCloseName);
    customStyle += ConcatNewLine(QString("         %1.m_triggerBtnClose = %2;").arg(t_dropDownBtnName).arg(t_triggerBtnCloseName));

    //选项弹出框
    //基本信息
    customStyle += ConcatNewLine(QString("         %1.m_ID = QString::fromLocal8Bit(\"%2\");").arg(t_optionPopUpName).arg(m_customButton->m_optionPopUp.m_ID));

    setLocation(m_customButton->m_optionPopUp.m_location);
    customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_optionPopUpName).arg(t_locationName));

    //边框
    setBaseInfo(m_customButton->m_optionPopUp.m_border.m_baseInfo);
    customStyle += ConcatNewLine(QString("         %1.m_border.m_baseInfo = %2;").arg(t_optionPopUpName).arg(t_baseInfoName));

    //标题
    setBaseInfo(m_customButton->m_optionPopUp.m_titleInfo.m_baseInfo);
    customStyle += ConcatNewLine(QString("         %1.m_titleInfo.m_baseInfo = %2;").arg(t_optionPopUpName).arg(t_baseInfoName));

    //选项按钮组
    auto setBtnGroupInfo = [&](ButtonGroup curBtnGroup,QString curBtnGroupName){
        customStyle += ConcatNewLine(QString("         %1.m_ID = QString::fromLocal8Bit(\"%2\");").arg(curBtnGroupName).arg(curBtnGroup.m_ID));

        setLocation(curBtnGroup.m_location);
        customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(curBtnGroupName).arg(t_locationName));

        for(ButtonInfo button : curBtnGroup.m_buttons){
            customStyle += ConcatNewLine(QString("         %1.deleteBtn = %2;").arg(t_buttonInfoName).arg(button.deleteBtn));
            setBaseInfo(button.m_baseInfo);
            customStyle += ConcatNewLine(QString("         %1.m_baseInfo = %2;").arg(t_buttonInfoName).arg(t_baseInfoName));

            customStyle += ConcatNewLine(QString("         %1.m_buttons.append(%2);").arg(curBtnGroupName).arg(t_buttonInfoName));

        }
    };

    //选项按钮组
    setBtnGroupInfo(m_customButton->m_optionPopUp.m_optionBtnGroup,t_optionBtnGroupName);
    customStyle += ConcatNewLine(QString("         %1.m_optionBtnGroup = %2;").arg(t_optionPopUpName).arg(t_optionBtnGroupName));

    //退出按钮组
    setBtnGroupInfo(m_customButton->m_optionPopUp.m_exitBtnGroup,t_exitBtnGroupName);
    customStyle += ConcatNewLine(QString("         %1.m_exitBtnGroup = %2;").arg(t_optionPopUpName).arg(t_exitBtnGroupName));

    customStyle += ConcatNewLine(QString("         %1.m_optionPopUp = %2;").arg(t_dropDownBtnName).arg(t_optionPopUpName));

    QString code;
    code += ConcatNewLine("{");
    code += ConcatNewLine(customStyle);
    code += ConcatNewLine("         ui->%1->setSelfStyleSheet(%2);").arg(m_customButton->m_ID).arg(t_dropDownBtnName);
    code += ConcatNewLine("    }");


    generate->addConstructInitCode(code);

}

void DropdownButton::setCustomButtonStyle(DropDownButtonData *customButton)
{
    m_customButton = customButton;
}

} //namespace CXX

