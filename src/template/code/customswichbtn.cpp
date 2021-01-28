#include "customswichbtn.h"

#include "../cppgenerate.h"

namespace CXX{

CustomSwichBtn::CustomSwichBtn() : AbstractCodeTemplate(CUSTOM_KEYBOARD_RFIELD)
{

}

void CustomSwichBtn::prepareOutput(CppGenerate *generate)
{
    QString t_locationName = QString("t_location_%1").arg(m_sameTypeIndex);
    QString t_baseInfoName = QString("t_baseInfo_%1").arg(m_sameTypeIndex);
    QString t_textInfoName = QString("t_textInfo_%1").arg(m_sameTypeIndex);


    QString t_customSwitchBtnName = QString("t_customSwitchBtn_%1").arg(m_sameTypeIndex);


    QString t_baseInfo = QString("         BaseInfo %1;").arg(t_baseInfoName);
    QString t_location = QString("         Location %1;").arg(t_locationName);
    QString t_textInfo = QString("         TextInfo %1;").arg(t_textInfoName);


    QString t_customSwitchBtn = QString("         customSwitchButton %1;").arg(t_customSwitchBtnName);


    QString customStyle;
    customStyle += ConcatNewLine(t_baseInfo);
    customStyle += ConcatNewLine(t_textInfo);
    customStyle += ConcatNewLine(t_location);

    customStyle += ConcatNewLine(t_customSwitchBtn);

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

    //获取开关按钮的状态信息
    setBaseInfo(m_customSwitchBtnInfo->m_closeState);
    customStyle += ConcatNewLine(QString("         %1.m_closeState = %2;").arg(t_customSwitchBtnName).arg(t_baseInfoName));

    setBaseInfo(m_customSwitchBtnInfo->m_openState);
    customStyle += ConcatNewLine(QString("         %1.m_openState = %2;").arg(t_customSwitchBtnName).arg(t_baseInfoName));

    for(BaseInfo t_baseinfo : m_customSwitchBtnInfo->m_textParagraphs){
        setBaseInfo(t_baseinfo);
        customStyle += ConcatNewLine(QString("         %1.m_textParagraphs.append(%2);").arg(t_customSwitchBtnName).arg(t_baseInfoName));
    }

    for(BaseInfo t_baseinfo : m_customSwitchBtnInfo->m_lineEdits){
        setBaseInfo(t_baseinfo);
        customStyle += ConcatNewLine(QString("         %1.m_lineEdits.append(%2);").arg(t_customSwitchBtnName).arg(t_baseInfoName));
    }

    QString code;
    code += ConcatNewLine("{");
    code += ConcatNewLine(customStyle);
    code += ConcatNewLine("         ui->%1->setSelfStyleSheet(%2);").arg(m_customSwitchBtnInfo->m_ID).arg(t_customSwitchBtnName);
    code += ConcatNewLine("    }");


    generate->addConstructInitCode(code);
}

/**
 * @brief 获取自制开关按钮必要参数
 * @param foldingControl
 */
void CustomSwichBtn::setCustomSwitchBtnData(customSwitchButton * customSwitchBtnInfo)
{
    m_customSwitchBtnInfo = customSwitchBtnInfo;
}

}
