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

#include "exportui.h"
#include "qtstruct.h"
#include "../html/htmlstruct.h"
#include "../css/cssstruct.h"
#include "qui/qtstruct.h"

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

    QStringList getOriginalResources(){return m_originalResources;}
    QStringList getResources(){return m_resources;}
    SelectorTypeMap getHtmlParsedResult(){return m_selectorType;}
    QString getTypeName(Html::NodeType type);

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
    void createComBoxImageProp(RDomWidget *domWidget, QString imageSrc, QString arrowImage, QString unArrowImage);
    void createSpinboxImageProp(RDomWidget * domWidget,Html::SpinboxData * data);

    inline void createTextProp(RDomWidget * domWidget, QString text);
    inline void createReadonlyProp(RDomWidget * domWidget,bool readonly);
    inline void createVisibleProp(RDomWidget * domWidget,bool visible);
    inline void createEnableProp(RDomWidget * domWidget, bool disable);
    inline void createTristateProp(RDomWidget * domWidget, bool tristate);
    void createCheckedProp(RDomWidget * domWidget,bool checked);
    inline void createLayoutDirectionProp(RDomWidget * domWidget,bool leftToRight);
    inline void createOrientationProp(RDomWidget *domWidget, bool leftToRight);
    inline void createToolTipProp(RDomWidget * domWidget,QString toolTip);
    inline void createCurrentIndexProp(RDomWidget * domWidget, int currentIndex);

    void createTreeNode(MItem *parentItem, Html::TreeItemData *textData);

    QString switchImageURL(QString imageSrc);

    bool judgeLayoutDirection(QString selectorId);
    QString getCssStyle(QString selectorName,QString propName);

    CSS::CssRule findRuleByName(CSS::Rules & rules,QString ruleName);
    void replaceRuleByName(CSS::Rules &rules, QString ruleName, CSS::CssRule newRule);

    void createConnections(Html::DomNode *node);

private:
    DomHtmlPtr m_dataSrc;
    CSS::CssMap m_globalCss;
    CSS::CssMap m_pageCss;

    QStringList m_originalResources;        /*!< 原始图片资源地址 */
    QStringList m_resources;                /*!< 处理后改成相对路径图片资源地址 */

    SelectorTypeMap m_selectorType;

    MConnections * m_conns;
};

} //namespace RQt

#endif // FORMATPROPERTY_H
