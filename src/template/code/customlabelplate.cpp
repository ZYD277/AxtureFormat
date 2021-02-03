#include "customlabelplate.h"

#include "../cppgenerate.h"

namespace CXX{

CustomLabelPlate::CustomLabelPlate() : AbstractCodeTemplate(CUSTOM_LABEL)
{

}

void CustomLabelPlate::prepareOutput(CppGenerate *generate)
{
    //定义私有函数
    QString t_privateFunName = QString("customLabel_%1()").arg(m_sameTypeIndex);
    generate->addPrivateFun(QString("void %1;").arg(t_privateFunName));

    generate->addConstructInitCode(NEW_EMPTY);
    generate->addConstructInitCode(t_privateFunName + ";");

    //弹出框部分

    QString t_locationName = QString("t_location_%1").arg(m_sameTypeIndex);
    QString t_baseInfoName = QString("t_baseInfo_%1").arg(m_sameTypeIndex);
    QString t_textInfoName = QString("t_textInfo_%1").arg(m_sameTypeIndex);

    QString t_customLabelInfoName = QString("t_customLabelInfo_%1").arg(m_sameTypeIndex);


    QString t_baseInfo = QString("         BaseInfo %1;").arg(t_baseInfoName);
    QString t_location = QString("         Location %1;").arg(t_locationName);
    QString t_textInfo = QString("         TextInfo %1;").arg(t_textInfoName);

    QString t_customLabelInfo = QString("         CustomLabelData %1;").arg(t_customLabelInfoName);


    QString customStyle;
    customStyle += ConcatNewLine(t_baseInfo);
    customStyle += ConcatNewLine(t_textInfo);
    customStyle += ConcatNewLine(t_location);
    customStyle += ConcatNewLine(t_customLabelInfo);

    auto setLocation = [&](Location location){
        //获取单个控件位置信息
        customStyle += ConcatNewLine(QString("         %1.m_width = %2;").arg(t_locationName).arg(location.m_width));
        customStyle += ConcatNewLine(QString("         %1.m_height = %2;").arg(t_locationName).arg(location.m_height));
        customStyle += ConcatNewLine(QString("         %1.m_top = %2;").arg(t_locationName).arg(location.m_top));
        customStyle += ConcatNewLine(QString("         %1.m_left = %2;").arg(t_locationName).arg(location.m_left));
    };
    auto setBaseInfo = [&](BaseInfo t_baseInfo){

        setLocation(t_baseInfo.m_location);
        //获取单个控件位置信息
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
        //获取控件id
        customStyle += ConcatNewLine(QString("         %1.m_ID = QString::fromLocal8Bit(\"%2\");").arg(t_baseInfoName).arg(t_baseInfo.m_ID));

    };

    m_customLabelInfo;
    //设置默认状态标签信息
    setBaseInfo(m_customLabelInfo->m_defaultInfo);
    customStyle += ConcatNewLine("         %1.m_defaultInfo = %2;").arg(t_customLabelInfoName).arg(t_baseInfoName);

    //设置鼠标进入状态下标签样式
    setBaseInfo(m_customLabelInfo->m_mouseEnter);
    customStyle += ConcatNewLine("         %1.m_mouseEnter = %2;").arg(t_customLabelInfoName).arg(t_baseInfoName);

    //设置弹窗样式
    m_customLabelInfo->m_labelPopupWindow.m_optionsInfo;
    customStyle += ConcatNewLine("         %1.m_labelPopupWindow.m_ID = QString::fromLocal8Bit(\"%2\");").arg(t_customLabelInfoName).arg(m_customLabelInfo->m_labelPopupWindow.m_ID);
    setLocation(m_customLabelInfo->m_labelPopupWindow.m_location);
    customStyle += ConcatNewLine("         %1.m_labelPopupWindow.m_location = %2;").arg(t_customLabelInfoName).arg(t_locationName);

    for(BaseInfo t_baseInfo : m_customLabelInfo->m_labelPopupWindow.m_optionsInfo){
        setBaseInfo(t_baseInfo);
        customStyle += ConcatNewLine("         %1.m_labelPopupWindow.m_optionsInfo.append(%2);").arg(t_customLabelInfoName).arg(t_baseInfoName);
    }

    QString code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_privateFunName));
    code += ConcatNewLine("{");
    code += ConcatNewLine(customStyle);
    code += ConcatNewLine("         ui->%1->initCustomLabel(%2);").arg(m_customLabelInfo->m_ID).arg(t_customLabelInfoName);
    code += ConcatNewLine("}");

    generate->addFunImplement(code);
}

void CustomLabelPlate::setCustomLabelInfo(CustomLabelData *customLabelInfo)
{
    m_customLabelInfo = customLabelInfo;
}

}

