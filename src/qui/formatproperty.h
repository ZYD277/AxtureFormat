/*!
 *  @brief     格式化统一节点属性
 *  @details   使用BaseData作为基础属性，其内部变量众多，不便于统一接口处理。此类用于将各种类型的属性统一接口，
 *             为格式化输出做准备
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.04
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef FORMATPROPERTY_H
#define FORMATPROPERTY_H

#include <QSize>

#include "exportui.h"
#include "qtstruct.h"
#include "../html/htmlstruct.h"
#include "../css/cssstruct.h"
#include "qui/qtstruct.h"
#include "../template/cppstruct.h"

namespace RQt{

class RDomWidget;
class MItem;

class FormatProperty
{
public:
    FormatProperty();

    void setDataSource(DomHtmlPtr ptr);
    void setCssMap(CSS::CssMap globalCss, CSS::CssMap pageCss);

    CSS::CssMap getGlobalCss(){return m_globalCss;}
    CSS::CssMap getPageCss(){return m_pageCss;}

    RDomWidget * formart();
    MConnections * connections(){return m_conns;}

    McontrolImproves *customControls(){return m_controlImproves;}

    QStringList getCustomClassList(){return m_customClassList;}

    QSize getWindowMinimumSize(){return m_maxWindowSize;}

    QStringList getOriginalResources(){return m_originalResources;}
    QStringList getResources(){return m_resources;}
    SelectorTypeMap getHtmlParsedResult(){return m_selectorType;}
    QString getTypeName(Html::NodeType type);
	QString gradientSwitchToQss(QString value);//渐变色背景转换为qss格式。


    CXX::CppCodeDatas getCodeDatas(){return m_codeDatas;}

private:
    typedef QMap<QString, QString> StyleMap;
    StyleMap extractCssRule(Html::DomNode * node);
    void createDomWidget(RDomWidget *parentWidget, Html::DomNode * node, QRect parentRect);
    void processNodeStyle(Html::DomNode * node);

    QRect calculateGeomerty(StyleMap & cssMap,Html::DomNode *node,QRect & parentRect);
    int removePxUnit(QString valueWithUnit);

    //创建UI控件属性节点
    inline void createImageProp(RDomWidget * domWidget, QString imageSrc);
    void createButtonImageProp(RDomWidget *domWidget, Html::ButtonData *baseData);
    void createRadioBtnImageProp(RDomWidget *domWidget, Html::BaseData *baseData, QString widgetName);
    void createTabWidgetImageProp(RDomWidget *domWidget, Html::TabWidgetData *tabData);
	void createTabWidgetStyleSheet(RDomWidget *domWidget, Html::TabWidgetData *tabData);
    void createComBoxImageProp(RDomWidget *domWidget, QString imageSrc, QString arrowImage, QString unArrowImage);
    void createSpinboxImageProp(RDomWidget * domWidget,Html::SpinboxData * data);
    void createProgressStyleProp(RDomWidget * domWidget,Html::SliderData * data);
    void createSrollBarStyleProp(RDomWidget * domWidget, Html::ScrollBarData * data, bool horizonal);
    void createTreeImageProp(RDomWidget * domWidget, Html::TreeData * data, QString textColorId);
    void createTableCodeData(Html::DomNode *node, Html::TableData *data, QString hSectionSize, QString vSectionSize, int columnCount, CXX::TableStyleCodeData * tableData);
    //自定义控件创建ui属性结点
    void createCustomButtonImageProp(RDomWidget *domWidget, Html::ButtonData *baseData);

    void createLineEditImageProp(RDomWidget *domWidget, Html::TextFieldData *baseData);


    inline void createTextProp(RDomWidget * domWidget, QString text);
    inline void createReadonlyProp(RDomWidget * domWidget,bool readonly);
    inline void createVisibleProp(RDomWidget * domWidget,bool visible);
    inline void createEnableProp(RDomWidget * domWidget, bool disable);
    inline void createTristateProp(RDomWidget * domWidget, bool tristate);
    void createCheckedProp(RDomWidget * domWidget,bool checked);
    inline void createLayoutDirectionProp(RDomWidget * domWidget,bool leftToRight);
    inline void createOrientationProp(RDomWidget *domWidget, bool horizonal);
    inline void createToolTipProp(RDomWidget * domWidget,QString toolTip);
    inline void createCurrentIndexProp(RDomWidget * domWidget, int currentIndex);

    QString switchImageURL(QString imageSrc);

    bool judgeLayoutDirection(QString selectorId);
    QString getCssStyle(QString selectorName,QString propName);

    CSS::CssRule findRuleByName(CSS::Rules & rules,QString ruleName);
    CSS::Rules findRulesByName(CSS::Rules & rules,QString ruleName);

    void replaceRuleByName(CSS::Rules &rules, QString ruleName, CSS::CssRule newRule);

    void createCodeDatas(Html::DomNode *node);
    void addCustomControlBgImg(QString srcImg);
    void getDrawStyle(CXX::DropDownButtonData *twSwitchData);

    void createConnections(Html::DomNode *node);
    void createControlImprove(Html::DomNode *node);

    QString switchCssRgbaToQt(QString cssRgba);
    QString switchCssGradientToQt(CSS::Rules linearGradients);

    void getMaxWindowSize(QRect rect);

private:
    DomHtmlPtr m_dataSrc;
    CSS::CssMap m_globalCss;
    CSS::CssMap m_pageCss;

    QStringList m_originalResources;        /*!< 原始图片资源地址，会存在引用其它工程目录下的图片 */
    QStringList m_resources;                /*!< 处理后改成相对路径图片资源地址 */

    CXX::CppCodeDatas m_codeDatas;          /*!< 自定义代码生成时代码集合 */

    SelectorTypeMap m_selectorType;

    MConnections * m_conns;

    McontrolImproves *m_controlImproves;

    QStringList m_customClassList;          /*!< 筛选用于提升的自定义类避免重复添加 */

    QSize m_maxWindowSize;                  /*!< axure窗口最大尺寸 */

};

} //namespace RQt

#endif // FORMATPROPERTY_H
