#include "customfoldingcontrol.h"

#include "../cppgenerate.h"

namespace CXX{

CustomFoldingControl::CustomFoldingControl() : AbstractCodeTemplate(CUSTOM_FOLDING_CONTROL)
{

}

void CustomFoldingControl::prepareOutput(CppGenerate *generate)
{

    QString t_locationName = QString("t_location_%1").arg(m_sameTypeIndex);
    QString t_baseInfoName = QString("t_baseInfo_%1").arg(m_sameTypeIndex);
    QString t_textInfoName = QString("t_textInfo_%1").arg(m_sameTypeIndex);

    QString t_foldingInfoName = QString("t_foldingInfo_%1").arg(m_sameTypeIndex);
    QString t_unFoldInfoName = QString("t_unFoldInfo_%1").arg(m_sameTypeIndex);
    QString t_informationName = QString("t_information_%1").arg(m_sameTypeIndex);

    QString t_FoldingControlsName = QString("t_FoldingControls_%1").arg(m_sameTypeIndex);


    QString t_baseInfo = QString("         BaseInfo %1;").arg(t_baseInfoName);
    QString t_location = QString("         Location %1;").arg(t_locationName);
    QString t_textInfo = QString("         TextInfo %1;").arg(t_textInfoName);

    QString t_foldingInfo = QString("         FoldingInfo %1;").arg(t_foldingInfoName);
    QString t_unFoldInfo = QString("         UnFoldInfo %1;").arg(t_unFoldInfoName);
    QString t_information = QString("         Information %1;").arg(t_informationName);
    QString t_FoldingControls = QString("         FoldingControls %1;").arg(t_FoldingControlsName);



    QString customStyle;
    customStyle += ConcatNewLine(t_baseInfo);
    customStyle += ConcatNewLine(t_textInfo);
    customStyle += ConcatNewLine(t_location);

    customStyle += ConcatNewLine(t_foldingInfo);
    customStyle += ConcatNewLine(t_unFoldInfo);
    customStyle += ConcatNewLine(t_information);
    customStyle += ConcatNewLine(t_FoldingControls);

    auto setLocation = [&](Location t_location){
        if(t_location.m_width != 0)
            customStyle += ConcatNewLine(QString("         %1.m_width = %2;").arg(t_locationName).arg(t_location.m_width));
        if(t_location.m_height != 0)
            customStyle += ConcatNewLine(QString("         %1.m_height = %2;").arg(t_locationName).arg(t_location.m_height));
        if(t_location.m_top != 0)
            customStyle += ConcatNewLine(QString("         %1.m_top = %2;").arg(t_locationName).arg(t_location.m_top));
        if(t_location.m_left != 0)
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

    //获取控件位置信息
    setLocation(m_foldingControl->m_location);
    customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_FoldingControlsName).arg(t_locationName));

    //获取折叠信息
    for(Information t_information : m_foldingControl->m_informations){
        setLocation(t_information.m_location);
        customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_informationName).arg(t_locationName));
        customStyle += ConcatNewLine(QString("         %1.m_ID = QString::fromLocal8Bit(\"%2\");").arg(t_informationName).arg(t_information.m_ID));

        setLocation(t_information.m_foldingInfo.m_location);
        customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_foldingInfoName).arg(t_locationName));
        customStyle += ConcatNewLine(QString("         %1.m_ID = QString(\"%2\");").arg(t_foldingInfoName).arg(t_information.m_foldingInfo.m_ID));

        for(BaseInfo t_baseInfo : t_information.m_foldingInfo.m_information){
            setBaseInfo(t_baseInfo);
            customStyle += ConcatNewLine(QString("         %1.m_information.append(%2);").arg(t_foldingInfoName).arg(t_baseInfoName));
        }

        customStyle += ConcatNewLine(QString("         %1.m_foldingInfo = %2;").arg(t_informationName).arg(t_foldingInfoName));

        customStyle += ConcatNewLine(QString("         %1.m_information.clear();").arg(t_foldingInfoName));

        setLocation(t_information.m_unFoldInfo.m_location);
        customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_unFoldInfoName).arg(t_locationName));

        setLocation(t_information.m_unFoldInfo.m_parentLocation);
        customStyle += ConcatNewLine(QString("         %1.m_parentLocation = %2;").arg(t_unFoldInfoName).arg(t_locationName));

        customStyle += ConcatNewLine(QString("         %1.m_ID = QString(\"%2\");").arg(t_unFoldInfoName).arg(t_information.m_unFoldInfo.m_ID));

        for(BaseInfo t_baseInfo : t_information.m_unFoldInfo.m_information){
            setBaseInfo(t_baseInfo);
            customStyle += ConcatNewLine(QString("         %1.m_information.append(%2);").arg(t_unFoldInfoName).arg(t_baseInfoName));
        }
        customStyle += ConcatNewLine(QString("         %1.m_unFoldInfo = %2;").arg(t_informationName).arg(t_unFoldInfoName));

        customStyle += ConcatNewLine(QString("         %1.m_information.clear();").arg(t_unFoldInfoName));

        customStyle += ConcatNewLine(QString("         %1.m_informations.append(%2);").arg(t_FoldingControlsName).arg(t_informationName));

    }

	QString code;
    code += ConcatNewLine("{");
    code += ConcatNewLine(customStyle);
    code += ConcatNewLine("         ui->%1->setSelfStyleSheet(%2);").arg(m_foldingControl->m_ID).arg(t_FoldingControlsName);
    code += ConcatNewLine("    }");


    generate->addConstructInitCode(code);
}

void CustomFoldingControl::setFoldingControlParameter(FoldingControls * foldingControl)
{
    m_foldingControl = foldingControl;
}

} //namespace CXX
