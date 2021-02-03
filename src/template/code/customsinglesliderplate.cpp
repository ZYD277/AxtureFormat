#include "customsinglesliderplate.h"

#include "../cppgenerate.h"

namespace CXX{

CustomSingleSliderPlate::CustomSingleSliderPlate() :AbstractCodeTemplate(CUSTOM_SIGNAL_SLIDING)
{

}

void CustomSingleSliderPlate::prepareOutput(CppGenerate *generate)
{
    //定义私有函数
    QString t_privateFunName = QString("singleSlider_%1()").arg(m_sameTypeIndex);
    generate->addPrivateFun(QString("void %1;").arg(t_privateFunName));

    generate->addConstructInitCode(NEW_EMPTY);
    generate->addConstructInitCode(t_privateFunName + ";");
    //弹出框部分

    QString t_locationName = QString("t_location_%1").arg(m_sameTypeIndex);
    QString t_baseInfoName = QString("t_baseInfo_%1").arg(m_sameTypeIndex);
    QString t_textInfoName = QString("t_textInfo_%1").arg(m_sameTypeIndex);

    QString t_scrollBarInfoName = QString("t_singleSliderInfo_%1").arg(m_sameTypeIndex);


    QString t_baseInfo = QString("         BaseInfo %1;").arg(t_baseInfoName);
    QString t_location = QString("         Location %1;").arg(t_locationName);
    QString t_textInfo = QString("         TextInfo %1;").arg(t_textInfoName);


    QString t_scrollBarInfo = QString("         SingleSlidingBlockData %1;").arg(t_scrollBarInfoName);


    QString customStyle;
    customStyle += ConcatNewLine(t_baseInfo);
    customStyle += ConcatNewLine(t_textInfo);
    customStyle += ConcatNewLine(t_location);
    customStyle += ConcatNewLine(t_scrollBarInfo);

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

    };

    //设置单一滚动条内部组成矩形框信息
    for(BaseInfo t_baseInfo : m_srollBarData->m_rectangularsInfo){
        setBaseInfo(t_baseInfo);
        customStyle += ConcatNewLine("         %1.m_rectangularsInfo.append(%2);").arg(t_scrollBarInfoName).arg(t_baseInfoName);
    }

    //左滑块
    customStyle += ConcatNewLine("         %1.m_leftScrollBar.m_id = QString::fromLocal8Bit(\"%2\");").arg(t_scrollBarInfoName).arg(m_srollBarData->m_leftScrollBar.m_id);
    setLocation(m_srollBarData->m_leftScrollBar.m_location);
    customStyle += ConcatNewLine("         %1.m_leftScrollBar.m_location = %2;").arg(t_scrollBarInfoName).arg(t_locationName);

    for(BaseInfo t_baseInfo : m_srollBarData->m_leftScrollBar.m_srcImgs){
        setBaseInfo(t_baseInfo);
        customStyle += ConcatNewLine("         %1.m_leftScrollBar.m_srcImgs.append(%2);").arg(t_scrollBarInfoName).arg(t_baseInfoName);
    }

    //左文字提示

    customStyle += ConcatNewLine("         %1.m_letfToolTip.m_ID = QString::fromLocal8Bit(\"%2\");").arg(t_scrollBarInfoName).arg(m_srollBarData->m_letfToolTip.m_ID);
    setLocation(m_srollBarData->m_letfToolTip.m_location);
    customStyle += ConcatNewLine("         %1.m_letfToolTip.m_location = %2;").arg(t_scrollBarInfoName).arg(t_locationName);

    setBaseInfo(m_srollBarData->m_letfToolTip.m_textBox);
    customStyle += ConcatNewLine("         %1.m_letfToolTip.m_textBox = %2;").arg(t_scrollBarInfoName).arg(t_baseInfoName);

    //文字输入框
    m_srollBarData->m_inputBox.m_ID;
    m_srollBarData->m_inputBox.m_inputBoxInfos;
    m_srollBarData->m_inputBox.m_location;

    customStyle += ConcatNewLine("         %1.m_inputBox.m_ID = QString::fromLocal8Bit(\"%2\");").arg(t_scrollBarInfoName).arg(m_srollBarData->m_inputBox.m_ID);
    setLocation(m_srollBarData->m_inputBox.m_location);
    customStyle += ConcatNewLine("         %1.m_inputBox.m_location = %2;").arg(t_scrollBarInfoName).arg(t_locationName);

    for(BaseInfo t_baseInfo : m_srollBarData->m_inputBox.m_inputBoxInfos){
        setBaseInfo(t_baseInfo);
        customStyle += ConcatNewLine("         %1.m_inputBox.m_inputBoxInfos.append(%2);").arg(t_scrollBarInfoName).arg(t_baseInfoName);
    }

    customStyle += ConcatNewLine("         %1.m_ID = QString::fromLocal8Bit(\"%2\");").arg(t_scrollBarInfoName).arg(m_srollBarData->m_ID);
    setLocation(m_srollBarData->m_location);
    customStyle += ConcatNewLine("         %1.m_location = %2;").arg(t_scrollBarInfoName).arg(t_locationName);

    QString code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_privateFunName));
    code += ConcatNewLine("{");
    code += ConcatNewLine(customStyle);
    code += ConcatNewLine("         ui->%1->initSingleSlider(%2);").arg(m_srollBarData->m_ID).arg(t_scrollBarInfoName);
    code += ConcatNewLine("}");

    generate->addFunImplement(code);
}

void CustomSingleSliderPlate::setScrollBarParameter(SingleSlidingBlockData *srollBarData)
{
    m_srollBarData = srollBarData;
}

}
