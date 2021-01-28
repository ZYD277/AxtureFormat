#include "customscrollbar.h"

#include "../cppgenerate.h"

namespace CXX{

CustomScrollBar::CustomScrollBar() :AbstractCodeTemplate(CUSTOM_BIDIRECTIONAL_SLIDER)
{

}

void CustomScrollBar::prepareOutput(CppGenerate *generate)
{
    //弹出框部分

    QString t_locationName = QString("t_location_%1").arg(m_sameTypeIndex);
    QString t_baseInfoName = QString("t_baseInfo_%1").arg(m_sameTypeIndex);
    QString t_textInfoName = QString("t_textInfo_%1").arg(m_sameTypeIndex);


    QString t_rectangularsInfoName = QString("t_rectangularsInfo_%1").arg(m_sameTypeIndex);
    QString t_singleScrollBarName = QString("t_singleScrollBar_%1").arg(m_sameTypeIndex);

    QString t_scrollBarStyleName = QString("t_scrollBarStyle_%1").arg(m_sameTypeIndex);


    QString t_baseInfo = QString("         BaseInfo %1;").arg(t_baseInfoName);
    QString t_location = QString("         Location %1;").arg(t_locationName);
    QString t_textInfo = QString("         TextInfo %1;").arg(t_textInfoName);

    QString t_rectangularsInfo = QString("         QList<BaseInfo> %1;").arg(t_rectangularsInfoName);
    QString t_singleScrollBar = QString("         DScrollBar %1;").arg(t_singleScrollBarName);

    QString t_scrollBarStyle = QString("         BidirectionalSlider %1;").arg(t_scrollBarStyleName);


    QString customStyle;
    customStyle += ConcatNewLine(t_baseInfo);
    customStyle += ConcatNewLine(t_textInfo);
    customStyle += ConcatNewLine(t_location);
    customStyle += ConcatNewLine(t_rectangularsInfo);
    customStyle += ConcatNewLine(t_singleScrollBar);
    customStyle += ConcatNewLine(t_scrollBarStyle);

    auto setBaseInfo = [&](BaseInfo t_baseInfo){

        //获取单个控件位置信息
        customStyle += ConcatNewLine(QString("         %1.m_width = %2;").arg(t_locationName).arg(t_baseInfo.m_location.m_width));
        customStyle += ConcatNewLine(QString("         %1.m_height = %2;").arg(t_locationName).arg(t_baseInfo.m_location.m_height));
        customStyle += ConcatNewLine(QString("         %1.m_top = %2;").arg(t_locationName).arg(t_baseInfo.m_location.m_top));
        customStyle += ConcatNewLine(QString("         %1.m_left = %2;").arg(t_locationName).arg(t_baseInfo.m_location.m_left));

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

    //获取滚动条除了滑块以外的矩形信息
    for(BaseInfo baseInfo : m_scrollBar->m_rectangularsInfo){
        setBaseInfo(baseInfo);
        customStyle += ConcatNewLine(QString("         %1.append(%2);").arg(t_rectangularsInfoName).arg(t_baseInfoName));
    }

    //获取滚动条滑块信息
    {
        auto setScrollBarInfo = [&](DScrollBar scrollBar){
            customStyle += ConcatNewLine(QString("         %1.m_width = %2;").arg(t_locationName).arg(scrollBar.m_location.m_width));
            customStyle += ConcatNewLine(QString("         %1.m_height = %2;").arg(t_locationName).arg(scrollBar.m_location.m_height));
            customStyle += ConcatNewLine(QString("         %1.m_top = %2;").arg(t_locationName).arg(scrollBar.m_location.m_top));
            customStyle += ConcatNewLine(QString("         %1.m_left = %2;").arg(t_locationName).arg(scrollBar.m_location.m_left));

            customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_singleScrollBarName).arg(t_locationName));

            for(BaseInfo baseInfo : scrollBar.m_srcImgs){
                setBaseInfo(baseInfo);
                customStyle += ConcatNewLine(QString("         %1.m_srcImgs.append(%2);").arg(t_singleScrollBarName).arg(t_baseInfoName));
            }

        };

        //左边滚动条
        setScrollBarInfo(m_scrollBar->m_leftScrollBar);
        customStyle += ConcatNewLine(QString("         %1.m_leftScrollBar = %2;").arg(t_scrollBarStyleName).arg(t_singleScrollBarName));

        //右边滚动条
        setScrollBarInfo(m_scrollBar->m_rightScrollBar);
        customStyle += ConcatNewLine(QString("         %1.m_rightScrollBar = %2;").arg(t_scrollBarStyleName).arg(t_singleScrollBarName));

    }

    //获取整个滑动条位置信息
    customStyle += ConcatNewLine(QString("         %1.m_width = %2;").arg(t_locationName).arg(m_scrollBar->m_location.m_width));
    customStyle += ConcatNewLine(QString("         %1.m_height = %2;").arg(t_locationName).arg(m_scrollBar->m_location.m_height));
    customStyle += ConcatNewLine(QString("         %1.m_top = %2;").arg(t_locationName).arg(m_scrollBar->m_location.m_top));
    customStyle += ConcatNewLine(QString("         %1.m_left = %2;").arg(t_locationName).arg(m_scrollBar->m_location.m_left));

    customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_scrollBarStyleName).arg(t_locationName));

    customStyle += ConcatNewLine(QString("         %1.m_rectangularsInfo = %2;").arg(t_scrollBarStyleName).arg(t_rectangularsInfoName));


    QString code;
    code += ConcatNewLine("{");
    code += ConcatNewLine(customStyle);
    code += ConcatNewLine("         ui->%1->setSelfStyleSheet(%2);").arg(m_scrollBar->m_ID).arg(t_scrollBarStyleName);
    code += ConcatNewLine("    }");


    generate->addConstructInitCode(code);
}

void CustomScrollBar::setScrollBarParameter(BidirectionalSlider *srollBarData)
{
    m_scrollBar = srollBarData;
}

} //namespace CXX
