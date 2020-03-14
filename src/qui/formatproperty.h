﻿/*!
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

    RDomWidget * formart();

    QStringList getOriginalResources(){return m_originalResources;}
    QStringList getResources(){return m_resources;}
    SelectorTypeMap getHtmlParsedResult(){return m_selectorType;}

private:
    QString getTypeName(Html::NodeType type);

    typedef QMap<QString, QString> StyleMap;
    StyleMap extractCssRule(Html::DomNode * node);
    void createDomWidget(RDomWidget *parentWidget, Html::DomNode * node, QRect parentRect);

    QRect calculateGeomerty(StyleMap & cssMap,Html::DomNode *node,QRect & parentRect);
    int removePxUnit(QString valueWithUnit);

    inline void createTextProp(RDomWidget * domWidget, QString text);
    inline void createReadonlyProp(RDomWidget * domWidget,bool readonly);
    inline void createEnableProp(RDomWidget * domWidget, bool disable);
    inline void createCheckedProp(RDomWidget * domWidget,bool checked);
    inline void createLayoutDirectionProp(RDomWidget * domWidget,bool leftToRight);
    void createToolTipProp(RDomWidget * domWidget,QString toolTip);

    void createTreeNode(MItem *parentItem, Html::TreeItemData *textData);

private:
    DomHtmlPtr m_dataSrc;
    CSS::CssMap m_globalCss;
    CSS::CssMap m_pageCss;

    QStringList m_originalResources;        /*!< 原始图片资源地址 */
    QStringList m_resources;                /*!< 处理后改成相对路径图片资源地址 */

    SelectorTypeMap m_selectorType;

};

} //namespace RQt

#endif // FORMATPROPERTY_H