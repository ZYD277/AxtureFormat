#include "myfloatingwindow.h"

#include "../cppgenerate.h"

namespace CXX{

MyFloatingWindow::MyFloatingWindow() : AbstractCodeTemplate(CUSTOM_FLOATING_WINDOW)
{

}


void MyFloatingWindow::prepareOutput(CppGenerate *generate)
{

    QString t_locationName = QString("t_location_%1").arg(m_sameTypeIndex);
    QString t_baseInfoName = QString("t_baseInfo_%1").arg(m_sameTypeIndex);
    QString t_textInfoName = QString("t_textInfo_%1").arg(m_sameTypeIndex);

    QString t_floatButtonName = QString("t_floatButton_%1").arg(m_sameTypeIndex);
    QString t_mainWidgetName = QString("t_mainWidget_%1").arg(m_sameTypeIndex);
    QString t_srcImgsName = QString("t_srcImgs_%1").arg(m_sameTypeIndex);
    QString t_switchButtonName = QString("t_switchButton_%1").arg(m_sameTypeIndex);

    QString t_floatingWindowName = QString("t_floatingWindow_%1").arg(m_sameTypeIndex);


    QString t_baseInfo = QString("         BaseInfo %1;").arg(t_baseInfoName);
    QString t_location = QString("         Location %1;").arg(t_locationName);
    QString t_textInfo = QString("         TextInfo %1;").arg(t_textInfoName);

    QString t_floatButton = QString("         FloatButton %1;").arg(t_floatButtonName);
    QString t_mainWidget = QString("         MainWidget %1;").arg(t_mainWidgetName);
    QString t_srcImgs = QString("         QList<BaseInfo> %1;").arg(t_srcImgsName);
    QString t_switchButton = QString("         SwitchButton %1;").arg(t_switchButtonName);

    QString t_floatingWindow = QString("         FloatingWindow %1;").arg(t_floatingWindowName);


    QString customStyle;
    customStyle += ConcatNewLine(t_baseInfo);
    customStyle += ConcatNewLine(t_textInfo);
    customStyle += ConcatNewLine(t_location);
    customStyle += ConcatNewLine(t_floatButton);
    customStyle += ConcatNewLine(t_mainWidget);
    customStyle += ConcatNewLine(t_srcImgs);
    customStyle += ConcatNewLine(t_switchButton);
    customStyle += ConcatNewLine(t_floatingWindow);

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

    //获取悬浮按钮的信息
    for(BaseInfo baseInfo : m_floattingWindow->m_floatButton.m_srcImgs){
        setBaseInfo(baseInfo);
        customStyle += ConcatNewLine(QString("         %1.append(%2);").arg(t_srcImgsName).arg(t_baseInfoName));
    }

    customStyle += ConcatNewLine(QString("         %1.m_srcImgs = %2;").arg(t_floatButtonName).arg(t_srcImgsName));

    setLocation(m_floattingWindow->m_floatButton.m_location);
    customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_floatButtonName).arg(t_locationName));

    //获取主窗口信息
    //获取id
    customStyle += ConcatNewLine(QString("         %1.m_id = QString::fromLocal8Bit(\"%2\");").arg(t_mainWidgetName).arg(m_floattingWindow->m_mainWidget.m_id));
    //背景img
    customStyle += ConcatNewLine(QString("         %1.m_srcImg = QString::fromLocal8Bit(\"%2\");").arg(t_mainWidgetName).arg(m_floattingWindow->m_mainWidget.m_srcImg));
    //位置
    setLocation(m_floattingWindow->m_mainWidget.m_location);
    customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_mainWidgetName).arg(t_locationName));
    //开关按钮
    for(SwitchButton switchButton : m_floattingWindow->m_mainWidget.m_switchButtons){
        setBaseInfo(switchButton.m_closeState);
        customStyle += ConcatNewLine(QString("         %1.m_closeState = %2;").arg(t_switchButtonName).arg(t_baseInfoName));

        setBaseInfo(switchButton.m_openState);
        customStyle += ConcatNewLine(QString("         %1.m_openState = %2;").arg(t_switchButtonName).arg(t_baseInfoName));

        setLocation(switchButton.m_location);
        customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_switchButtonName).arg(t_locationName));


        customStyle += ConcatNewLine(QString("         %1.m_switchButtons.append(%2);").arg(t_mainWidgetName).arg(t_switchButtonName));

    }

    //获取整个悬浮窗信息
    customStyle += ConcatNewLine(QString("         %1.m_floatButton = %2;").arg(t_floatingWindowName).arg(t_floatButtonName));
    customStyle += ConcatNewLine(QString("         %1.m_mainWidget = %2;").arg(t_floatingWindowName).arg(t_mainWidgetName));

    setLocation(m_floattingWindow->m_location);
    customStyle += ConcatNewLine(QString("         %1.m_location = %2;").arg(t_floatingWindowName).arg(t_locationName));

    QString code;
    code += ConcatNewLine("{");
    code += ConcatNewLine(customStyle);
    code += ConcatNewLine("         ui->%1->setSelfStyleSheet(%2);").arg(m_floattingWindow->m_ID).arg(t_floatingWindowName);
    code += ConcatNewLine("    }");


    generate->addConstructInitCode(code);
}

void MyFloatingWindow::setFloatingWindowParameter(FloatingWindow * floatingWindowData)
{
    m_floattingWindow = floatingWindowData;
}

} //namespace CXX
